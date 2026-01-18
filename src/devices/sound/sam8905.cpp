// license:BSD-3-Clause
// copyright-holders:Draft

#include "emu.h"
#include "sam8905.h"

#define DAC_SHIFT 9

DEFINE_DEVICE_TYPE(SAM8905, sam8905_device, "sam8905", "Dream SAM8905")

sam8905_device::sam8905_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t buffer_size)
	: device_t(mconfig, SAM8905, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_buffer_size(buffer_size)
	, m_waveform_read(*this, 0)
	, m_waveform_write(*this)
	, m_sample_output(*this)
{
}

void sam8905_device::device_start()
{
	m_aram = std::make_unique<uint16_t[]>(256);
	m_dram = std::make_unique<uint32_t[]>(256);

	m_stream = stream_alloc(0, 2, clock() / m_buffer_size); // Placeholder sample rate logic

	// Initialize master-slave mode
	m_slave_mode = false;
	// m_last_out_l = 0;
	// m_last_out_r = 0;

	// Initialize slave mode ring buffer (1024 samples should be plenty)
	m_slave_ring_size = clock() / m_buffer_size; //1024;
	m_slave_ring_l = std::make_unique<int32_t[]>(m_slave_ring_size);
	m_slave_ring_r = std::make_unique<int32_t[]>(m_slave_ring_size);
	m_slave_ring_write_pos = 0;
	m_slave_ring_read_pos = 0;
	std::fill_n(m_slave_ring_l.get(), m_slave_ring_size, 0);
	std::fill_n(m_slave_ring_r.get(), m_slave_ring_size, 0);

	save_pointer(NAME(m_aram), 256);
	save_pointer(NAME(m_dram), 256);
	save_item(NAME(m_control_reg));
	save_item(NAME(m_address_reg));
	save_item(NAME(m_interrupt_latch));
	save_item(NAME(m_slave_mode));
	// save_item(NAME(m_last_out_l));
	// save_item(NAME(m_last_out_r));
	save_pointer(NAME(m_slave_ring_l), m_slave_ring_size);
	save_pointer(NAME(m_slave_ring_r), m_slave_ring_size);
	save_item(NAME(m_slave_ring_write_pos));
	save_item(NAME(m_slave_ring_read_pos));
	// Chip-level registers
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_phi));
	save_item(NAME(m_wf));
	save_item(NAME(m_mul_result));
	save_item(NAME(m_carry));
	save_item(NAME(m_clear_rqst));
	save_item(NAME(m_int_mod));
	save_item(NAME(m_l_acc));
	save_item(NAME(m_r_acc));
	save_item(NAME(m_mix_l));
	save_item(NAME(m_mix_r));
}

void sam8905_device::device_reset()
{
	m_bus = 0;
	m_a = m_b = 0;
	m_x = m_y = 0;
	m_phi = m_wf = 0;
	m_mul_result = 0;
	m_carry = false;
	m_clear_rqst = false;
	m_int_mod = false;
	m_l_acc = m_r_acc = 0;
	m_mix_l = m_mix_r = 0;
	m_control_reg = 0;
	m_interrupt_latch = 0;
}

// ---------------------------------------------------------
// Arithmetic & Logic Helpers
// ---------------------------------------------------------

uint32_t sam8905_device::get_constant(uint8_t mad)
{
	// From Appendix I - Constants derived from MAD
	// X is fractional Q0.11 format: value = constant / 2048
	// MAD=0: 0.0004883, MAD=1: 0.06299, ... MAD=15: 0.93799
	static const uint32_t constants[16] = {
		0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
		0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
	};
	return constants[mad & 0xf];
}

int32_t sam8905_device::get_waveform(uint32_t wf, uint32_t phi, uint8_t mad, int slot_idx)
{
	phi &= 0xFFF; // 12-bit phase
	bool internal = (wf & 0x100);  // WF[8] = INT/EXT

	if (internal) {
		// WF[8]=1: Internal waveform
		// WF bit layout: [8]=INT/EXT, [7]=R, [6]=I, [5:4]=SEL, [3]=Z, [2:0]=unused
		bool z_bit = (wf & 0x08);      // WF[3] = Z (zero select)
		int sel = (wf >> 4) & 3;       // WF[5:4] = SEL (ramp type)
#if 0
        // CORRECT ACCORDING TO MANUAL
		bool invert = (wf & 0x40);     // WF[6] = I (0=direct, 1=two's complement)
		bool ramp_mode = (wf & 0x80);  // WF[7] = R (0=sinus, 1=ramp/constant)
#else
        // WORKING FX
		bool invert = (wf & 0x80);     // WF[7] = I (0=direct, 1=two's complement)
		bool ramp_mode = (wf & 0x40);  // WF[6] = R (0=sinus, 1=ramp/constant)
#endif

        if (z_bit) {
            return 0;
        }

        // if (wf & 3) {
        //     //printf("unused 0x%x\n", wf);
		//     invert = (wf & 0x80);     // WF[7] = I (0=direct, 1=two's complement)
		//     ramp_mode = (wf & 0x40);  // WF[6] = R (0=sinus, 1=ramp/constant)
        // }

		int32_t result;
		if (!ramp_mode) {
			// R=0: Sinus wave
			// X = .71875 sin ( (PI/2048) * PHI + PI/4096 )
			double angle = (M_PI / 2048.0) * (double)phi + (M_PI / 4096.0);
			result = (int32_t)(0.71875 * sin(angle) * 2048.0);
		} else {
			// Ramps based on SEL bits
			int sel = (wf >> 4) & 3;
			switch(sel) {
				case 0: // 2x PHI ramp
					result = (phi < 1024) ? phi * 2 :
					         (phi < 3072) ? (phi * 2) - 4096 : (phi * 2) - 8192;
					break;
				case 1: // Constant from micro-instruction
					result = get_constant(mad);
					break;
				case 2: // PHI ramp
					result = (phi < 2048) ? phi : phi - 4096;
					break;
				case 3: // PHI/2 ramp
					result = (phi < 2048) ? phi / 2 : (phi / 2) - 2048;
					break;
				default:
					result = 0;
			}
		}

        // Option 1: Mask to 12-bit (wraps)
        if (invert) {
            result = (-result) & 0xFFF;
            if (result & 0x800) result |= ~0xFFF; // sign extend
        }

        // Option 2: Saturate
        // if (invert) {
        //     result = -result;
        //     if (result > 2047) result = 2047;
        //     if (result < -2048) result = -2048;
        // }

        // EXPERIMENTING
		// 
		// Apply I bit: one's complement inversion with INVERTED semantics
		// Manual says "I (0=direct, 1=two's complement)" but testing shows:
		// I=0 means apply one's complement (XOR 0xFFF), I=1 means direct
		// This is consistent with old hardware where ~0 = -1 (not 0)
		// if (invert) {
		// 	// One's complement: bit inversion (XOR with 0xFFF)
		// 	result = result ^ 0xFFF;
		// 	// Sign extend from 12-bit
		// 	if (result & 0x800) result |= ~0xFFF;
		// }
		result &= 0xFFF;

        return result;
	} else {
		// External memory access
		if (!m_waveform_read.isunset()) {
			// Build 20-bit address: WAVE[7:0] | PHI[11:0]
			uint32_t addr = ((wf & 0xFF) << 12) | phi;
			// Read 12-bit sample from external ROM
			int16_t sample = m_waveform_read(addr);
			// Sign-extend to 32-bit (sample is already in 12-bit signed format)
			return sample; //(int32_t)(int16_t)(sample << 4) >> 4;
		}
		return 0;
	}
	return 0;
}

// ---------------------------------------------------------
// Core Interpreter
// ---------------------------------------------------------

void sam8905_device::execute_cycle(int slot_idx, uint16_t inst, int pc_start, int pc)
{
	uint8_t mad = (inst >> 11) & 0xF;
	uint8_t emitter_sel = (inst >> 9) & 0x3;
	bool wsp = BIT(inst, 8);

    // JUST in case RSP is used with writes
	uint32_t bus = m_bus;
	uint32_t dram_addr = (slot_idx << 4) | mad;

	// Emitters
	switch (emitter_sel) {
		case 0: bus = m_dram[dram_addr]; break; // RM
		case 1: bus = (m_a + m_b) & MASK19; break; // RADD
		case 2: bus = m_mul_result; break; // RP
		case 3: break; //bus = 0; break; // RSP (NOP)
	}
    m_bus = bus;

	// Helper lambda to update carry - called after any change to A or B
	// Carry calculation per Table 3:
	// - B positive (bit 18=0): carry = 19-bit overflow from addition
	// - B negative (bit 18=1): carry = result is positive (sign bit = 0)
	auto update_carry = [this]() {
		bool b_neg = BIT(m_b, 18);
		uint32_t sum = m_a + m_b;
		if (!b_neg)
			m_carry = (sum > MASK19);  // 19-bit overflow
		else
			m_carry = !BIT(sum & MASK19, 18);  // For envelope: positive result
	};

	// Initial carry state from current A,B
	update_carry();

	// Receivers (Active Low except WSP)
	// WA (Write A)
	if (!BIT(inst, 7)) {
		bool wphi_active = !BIT(inst, 4);

		// "WPHI WSP takes priority over WA WSP giving a normal WA" (Section 8-3)
		if (wsp && !wphi_active) {
			// WA WSP Truth Table - only when WPHI is NOT also active
			uint32_t wave = (bus >> 9) & 0x1FF;
			uint32_t final_wave = bus & 0x1FF;
			bool end_bit = BIT(bus, 18);
			bool wf_match = (wave == final_wave);

			if (!m_carry) {
				m_a = 0;
                m_clear_rqst = false;
                m_int_mod = true;
			} else if (!wf_match) {
				m_a = 0x200;
                m_clear_rqst = false;
                m_int_mod = true;
			} else {
				m_a = 0;
                m_int_mod = true;
				m_clear_rqst = end_bit;
			}
			//update_carry();  // A changed
		} else {
			// Normal WA: load A from bus
			// Per Section 8-3: "WA (without WSP) always sets CLEARRQST and clears INTMOD"
			m_a = bus;
			m_clear_rqst = true;
			m_int_mod = false;
			//update_carry();  // A changed
		}
	}

	// WB (Write B)
	if (!BIT(inst, 6)) {
		m_b = bus;
		//update_carry();  // B changed
	}

	// WM (Write Memory)
	if (!BIT(inst, 5)) {
		bool write_enable = true;
		if (wsp) {
			// WM WSP Truth Table (Section 8-4)
			if (!m_clear_rqst) {
                write_enable = false;
            }
			else if (m_carry) {
                write_enable = false;
            }
			// Interrupt latch: set when CLEARRQST=1 AND (INTMOD=1 OR CARRY=1)
			bool irq_condition = m_clear_rqst && (m_int_mod || m_carry);
			if (irq_condition) {
				m_interrupt_latch = (slot_idx << 4) | mad; // NB: Use MAD to identify interrupt reason?
			}
		}
		if (write_enable) {
			m_dram[dram_addr] = bus;
		}
	}

	// WPHI (Write Phase)
	// PHI register is 12 bits, extracted from bits 18:7 of the 19-bit bus
	// Lower 7 bits are fractional precision in D-RAM, upper 12 bits are the actual phase
	if (!BIT(inst, 4)) {
		m_phi = (bus >> 7) & 0xFFF;  // Upper 12 bits for waveform address
		if (wsp) {
			m_wf = 0x100; // Force Internal Sinus
		} else if (!(m_wf & 0x100)) {
			// External waveform: WPHI updates address, triggers ROM read, updates X
			//m_x = get_waveform(m_wf, m_phi, mad, slot_idx) & 0xFFF;
		}
	}

	// WXY (Write X and Y registers) - executes when bit 3 = 0
	if (!BIT(inst, 3)) {
		m_y = (bus >> 7) & 0xFFF;
		m_x = get_waveform(m_wf, m_phi, mad, slot_idx) & 0xFFF;
		// WSP inside WXY - sets mix_l/mix_r from bus value
		if (wsp) {
			m_mix_l = (bus >> 3) & 0x7;
			m_mix_r = bus & 0x7;
		}
	}

	// clearB
	if (!BIT(inst, 2)) {
		m_b = 0;
		//update_carry();  // B changed

		// WWE (Write Waveform Enable) - triggered by RSP + clearB + WSP
		// RSP = emitter_sel==3, clearB = bit 2 active, WSP = bit 8 active
		// Per SAM8905 datasheet Section 9: Data to write is loaded into Y register
		// via WXY instruction before WWE fires. WWE outputs Y to WD pins.
		if (emitter_sel == 3 && wsp && !m_waveform_write.isunset()) {
			// Only write externally when WF indicates external memory (WF < 0x80)
			// WF >= 0x80 is input sample address space, WF >= 0x100 is internal waveform
			if ((m_wf & 0x1FF) < 0x80) {
				// Build 15-bit SRAM address: lower 15 bits of (WF << 12 | PHI)
				// SRAM WA0-WA14 connects to SAM address bus bits 0-14
				// 32KB = 15 bits: WF[2:0] << 12 | PHI[11:0]
				uint32_t ext_addr = ((m_wf & 0x7) << 12) | (m_phi & 0xFFF);
				// Write data is from Y register (12-bit signed)
				int16_t ext_data = m_y & 0xFFF;
				// Sign extend to 16-bit for callback
				//if (ext_data & 0x800) ext_data |= 0xF000;
				m_waveform_write(ext_addr, ext_data);
			}
		}
	}

	// WWF (Write Waveform)
	if (!BIT(inst, 1)) {
		m_wf = (bus >> 9) & 0x1FF;
		// External waveform: WWF updates address, triggers ROM read, updates X
		// if (!(m_wf & 0x100)) {
		// 	m_x = get_waveform(m_wf, m_phi, mad, slot_idx) & 0xFFF;
		// }
	}

	// WACC (Accumulate) - with proper dB attenuation
	if (!BIT(inst, 0)) {
		// dB attenuation lookup: 000=mute, 001=-36dB, 010=-30dB, 011=-24dB,
		//                        100=-18dB, 101=-12dB, 110=-6dB, 111=0dB
		static constexpr uint16_t MIX_ATTEN[8] = { 0, 16, 32, 64, 128, 256, 512, 1024 };

		// Sign-extend 19-bit mul_result to 32-bit for proper signed math
		int32_t signed_mul = (m_mul_result & MASK19);
		if (signed_mul & 0x40000) signed_mul |= 0xFFF80000;  // Sign extend from bit 18

		// Apply dB attenuation lookup
		int32_t l_contrib = (signed_mul * MIX_ATTEN[m_mix_l]) >> 10;
		int32_t r_contrib = (signed_mul * MIX_ATTEN[m_mix_r]) >> 10;
		// m_l_acc += l_contrib;
		// m_r_acc += r_contrib;

        m_l_acc = std::clamp((int32_t)m_l_acc + l_contrib, -(1 << 24), (1 << 24) - 1);
        m_r_acc = std::clamp((int32_t)m_r_acc + r_contrib, -(1 << 24), (1 << 24) - 1);
	}

	// Multiplier is combinational - continuously computes X * Y
	// Update mul_result at end of cycle based on current X and Y values
	{
		int32_t x_signed = (int32_t)((int16_t)(m_x << 4) >> 4);
		int32_t y_signed = (int32_t)((int16_t)(m_y << 4) >> 4);
		// Q0.11 * Q0.11 = Q0.22 (24-bit), round to 19-bit Q0.18: shift right by 4
		int32_t product = x_signed * y_signed;
		m_mul_result = (uint32_t)((product + 8) >> 4) & MASK19;
	}
}

// Process a single audio frame (16 slots) - used for master-slave synchronization
void sam8905_device::process_frame(int32_t &out_l, int32_t &out_r)
{
	// SSR bit (bit 3): 0 = 44.1kHz, 1 = 22.05kHz
	bool ssr_mode = BIT(m_control_reg, 3);

	// Clear chip-level accumulators at start of frame
	m_l_acc = 0;
	m_r_acc = 0;

	for (int s = 0; s < 16; ++s) {
		// Fetch algorithm block from address 15 of D-RAM block
		// Word 15 format: | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
		uint32_t param15 = m_dram[(s << 4) | 15];

		if (BIT(param15, 11)) {
			// I (Idle) bit - slot produces no sound
			continue;
		}

		uint16_t pc_start;
		int inst_count;

		if (ssr_mode) {
			// 22.05kHz mode: 4 algorithms × 64 instructions
			// Per programmer's guide: A-RAM address uses AL2,AL1 (bits 10-9) not AL1,AL0 (bits 9-8)
			uint8_t alg = (param15 >> 9) & 0x3;
			pc_start = alg << 6;
			inst_count = 64;
		} else {
			// 44.1kHz mode: 8 algorithms × 32 instructions
			uint8_t alg = (param15 >> 8) & 0x7;
			pc_start = alg << 5;
			inst_count = 32;
		}

		// Skip reserved instructions (last 2)
		for (int pc = 0; pc < inst_count - 2; ++pc) {
			execute_cycle(s, m_aram[pc_start + pc], pc_start, pc);
		}
	}

	// Output accumulated values
	// Push to slave mode ring buffer
	m_slave_ring_l[m_slave_ring_write_pos] = (int32_t)m_l_acc;
	m_slave_ring_r[m_slave_ring_write_pos] = (int32_t)m_r_acc;
	m_slave_ring_write_pos = (m_slave_ring_write_pos + 1) % m_slave_ring_size;
}

void sam8905_device::sound_stream_update(sound_stream &stream)
{
    //lib/ Master mode: process frames independently
	// SSR bit (bit 3): 0 = 44.1kHz, 1 = 22.05kHz
	bool ssr_mode = BIT(m_control_reg, 3);

    // In slave mode, read from ring buffer filled by process_frame calls
	if (m_slave_mode) {
		for (int samp = 0; samp < stream.samples(); ++samp) {
			// Read from ring buffer if data available, otherwise use last value
            int32_t last_out_l = m_slave_ring_l[m_slave_ring_read_pos];
            int32_t last_out_r = m_slave_ring_r[m_slave_ring_read_pos];

			if (m_slave_ring_read_pos != m_slave_ring_write_pos) {
				m_slave_ring_read_pos = (m_slave_ring_read_pos + 1) % m_slave_ring_size;
			}
			// stream.put_int_clamp(0, samp, last_out_l, 32768);
			// stream.put_int_clamp(1, samp, last_out_r, 32768);
			stream.put_int_clamp(0, samp, last_out_l >> DAC_SHIFT, 16384);
			stream.put_int_clamp(1, samp, last_out_r >> DAC_SHIFT, 16384);
		}

		// Debug: check for ring buffer divergence
		size_t distance = (m_slave_ring_write_pos >= m_slave_ring_read_pos)
			? (m_slave_ring_write_pos - m_slave_ring_read_pos)
			: (m_slave_ring_size - m_slave_ring_read_pos + m_slave_ring_write_pos);
		if (distance > 1024) {
			static int diverge_warn = 0;
			//if (diverge_warn < 2000) {
				printf("SAM8905 slave ring divergence: write=%zu read=%zu distance=%zu, MODE: %u\n",
					m_slave_ring_write_pos, m_slave_ring_read_pos, distance, ssr_mode);
				diverge_warn++;
			//}
		}
        //m_slave_ring_write_pos = m_slave_ring_read_pos;
		return;
	}

	for (int samp = 0; samp < stream.samples(); ++samp) {
		// Clear chip-level accumulators at start of frame
		m_l_acc = 0;
		m_r_acc = 0;

		for (int s = 0; s < 16; ++s) {
			// Fetch algorithm block from address 15 of D-RAM block
			// Word 15 format: | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
			uint32_t param15 = m_dram[(s << 4) | 15];
			if (BIT(param15, 11)) {
				// I (Idle) bit - slot produces no sound
				continue;
			}

			uint16_t pc_start;
			int inst_count;

			if (ssr_mode) {
				// 22.05kHz mode: 4 algorithms × 64 instructions
				uint8_t alg = (param15 >> 9) & 0x3;
				pc_start = alg << 6;
				inst_count = 64;
			} else {
				// 44.1kHz mode: 8 algorithms × 32 instructions
				uint8_t alg = (param15 >> 8) & 0x7;
				pc_start = alg << 5;
				inst_count = 32;
			}

			// Skip reserved instructions (last 2)
			for (int pc = 0; pc < inst_count - 2; ++pc) {
				execute_cycle(s, m_aram[pc_start + pc], pc_start, pc);
			}
		}

		// Accumulators are 24-bit, output upper 16 bits (per SAM8905 datasheet)
		// stream.put_int_clamp(0, samp, (int32_t)m_l_acc >> 8, 32768);
		// stream.put_int_clamp(1, samp, (int32_t)m_r_acc >> 8, 32768);
		stream.put_int_clamp(0, samp, (int32_t)m_l_acc >> DAC_SHIFT, 16384);
		stream.put_int_clamp(1, samp, (int32_t)m_r_acc >> DAC_SHIFT, 16384);

		// Fire sample output callback for inter-chip audio (FX processing)
		if (!m_sample_output.isunset()) {
			// Output upper 16 bits of 24-bit accumulator, clamped to 15-bit signed range
			// 15-bit constraint required for FX SRAM delay lines: SRAM stores only 8 bits
			// (WD10:WD3), with WD11 sign-extended from WD10. Values outside 11-bit range
			// after >> 4 scaling would corrupt on SRAM roundtrip.
			// int32_t l_out = std::clamp((int32_t)m_l_acc >> 9, -16384, 16383);
			// int32_t r_out = std::clamp((int32_t)m_r_acc >> 9, -16384, 16383);
			int32_t l_out = std::clamp((int32_t)m_l_acc >> DAC_SHIFT, -16384, 16383);
			int32_t r_out = std::clamp((int32_t)m_r_acc >> DAC_SHIFT, -16384, 16383);
			// Pack L/R as 32-bit value: upper 16 = L, lower 16 = R
			uint32_t packed = ((l_out & 0xFFFF) << 16) | (r_out & 0xFFFF);
			m_sample_output(packed);
		}
	}
}

// ---------------------------------------------------------
// CPU Bus Interface
// ---------------------------------------------------------

void sam8905_device::write(offs_t offset, uint8_t data)
{
	m_stream->update();
	switch (offset & 7) {
		case 0: m_address_reg = data; break;
		case 1: m_data_latch = (m_data_latch & 0x7FF00) | data; break;
		case 2: m_data_latch = (m_data_latch & 0x700FF) | (data << 8); break;
		case 3: m_data_latch = (m_data_latch & 0x0FFFF) | ((data & 0x7) << 16); break;
		case 4: // Control Reg
			m_control_reg = data;
			if (BIT(data, 0)) { // WR=1: Write Request
				if (BIT(data, 1)) {
					m_aram[m_address_reg] = m_data_latch & 0x7FFF;
				} else {
					m_dram[m_address_reg] = m_data_latch & MASK19;
				}
			} else { // WR=0: Read Request - latch RAM data
				if (BIT(data, 1)) {
					m_data_latch = m_aram[m_address_reg] & 0x7FFF;
				} else {
					m_data_latch = m_dram[m_address_reg] & MASK19;
				}
			}
			break;
	}
}

uint8_t sam8905_device::read(offs_t offset)
{
	uint8_t result = 0;
	switch (offset & 7) {
		case 0: result = m_interrupt_latch; break;  // Interrupt source: (slot << 4) | word
		case 1: result = m_data_latch & 0xFF; break;         // LSB
		case 2: result = (m_data_latch >> 8) & 0xFF; break;  // NSB
		case 3: result = (m_data_latch >> 16) & 0x07; break; // MSB (3 bits for D-RAM)
		case 4: result = m_control_reg; break;
	}
	return result;
}
