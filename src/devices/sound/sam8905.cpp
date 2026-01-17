// license:BSD-3-Clause
// copyright-holders:Draft

#include "emu.h"
#include "sam8905.h"

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
	m_last_out_l = 0;
	m_last_out_r = 0;

	save_pointer(NAME(m_aram), 256);
	save_pointer(NAME(m_dram), 256);
	save_item(NAME(m_control_reg));
	save_item(NAME(m_address_reg));
	save_item(NAME(m_interrupt_latch));
	save_item(NAME(m_slave_mode));
	save_item(NAME(m_last_out_l));
	save_item(NAME(m_last_out_r));
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
	bool internal = (wf & 0x100);

	if (internal) {
		// Bit 8=1: Internal. Check fields R, I, SEL, Z
		bool z_bit = (wf & 0x1);
		if (z_bit) return 0;

		bool ramp_mode = (wf & 0x40); // R bit
		if (!ramp_mode) {
			// Sinus: X = .71875 sin ( (PI/2048) * PHI + PI/4096 )
			double angle = (M_PI / 2048.0) * (double)phi + (M_PI / 4096.0);
			return (int32_t)(0.71875 * sin(angle) * 2048.0);
		} else {
			// Ramps based on SEL bits
			int sel = (wf >> 2) & 3;
			switch(sel) {
				case 0: return (phi < 1024) ? phi * 2 : (phi < 3072) ? (phi * 2) - 4096 : (phi * 2) - 8192; // 2x PHI
				case 1: return get_constant(mad); // Constant from micro-instruction
				case 2: return (phi < 2048) ? phi : phi - 4096; // PHI ramp
				case 3: return (phi < 2048) ? phi / 2 : (phi / 2) - 2048; // PHI/2 ramp
				default: return 0;
			}
		}
	} else {
		// External memory access
		if (!m_waveform_read.isunset()) {
			// Build 20-bit address: WAVE[7:0] | PHI[11:0]
			uint32_t addr = ((wf & 0xFF) << 12) | phi;
			// Read 12-bit sample from external ROM
			int16_t sample = m_waveform_read(addr);
			// Sign-extend to 32-bit (sample is already in 12-bit signed format)
			return (int32_t)(int16_t)(sample << 4) >> 4;
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

	uint32_t bus = 0;
	uint32_t dram_addr = (slot_idx << 4) | mad;

	// Emitters
	switch (emitter_sel) {
		case 0: bus = m_dram[dram_addr]; break; // RM
		case 1: bus = (m_a + m_b) & MASK19; break; // RADD
		case 2: bus = m_mul_result; break; // RP
		case 3: bus = 0; break; // RSP (NOP)
	}

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
				m_a = 0; m_clear_rqst = false; m_int_mod = true;
			} else if (!wf_match) {
				m_a = 0x200; m_clear_rqst = false; m_int_mod = true;
			} else {
				m_a = 0; m_int_mod = true;
				m_clear_rqst = end_bit;
			}
			update_carry();  // A changed
		} else {
			// Normal WA: load A from bus
			// Per Section 8-3: "WA (without WSP) always sets CLEARRQST and clears INTMOD"
			m_a = bus;
			m_clear_rqst = true;
			m_int_mod = false;
			update_carry();  // A changed
		}
	}

	// WB (Write B)
	if (!BIT(inst, 6)) {
		m_b = bus;
		update_carry();  // B changed
	}

	// WM (Write Memory)
	if (!BIT(inst, 5)) {
		bool write_enable = true;
		if (wsp) {
			// WM WSP Truth Table (Section 8-4)
			if (!m_clear_rqst) write_enable = false;
			else if (m_carry) write_enable = false;

			// Interrupt latch: set when CLEARRQST=1 AND (INTMOD=1 OR CARRY=1)
			bool irq_condition = m_clear_rqst && (m_int_mod || m_carry);
			if (irq_condition) {
				m_interrupt_latch = (slot_idx << 4) | mad;
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
		if (wsp) m_wf = 0x100; // Force Internal Sinus
	}

	// WXY (Write Multiplier) - executes when bit 3 = 0
	if (!BIT(inst, 3)) {
		m_y = (bus >> 7) & 0xFFF;
		m_x = get_waveform(m_wf, m_phi, mad, slot_idx) & 0xFFF;
		// Calculate multiplication result (12x12 fractional Q0.11 inputs)
		// Sign-extend 12-bit to 16-bit for signed multiplication
		int32_t x_signed = (int32_t)((int16_t)(m_x << 4) >> 4);
		int32_t y_signed = (int32_t)((int16_t)(m_y << 4) >> 4);
		// Q0.11 * Q0.11 = Q0.22 (24-bit), round to 19-bit Q0.18: shift right by 4
		int32_t product = x_signed * y_signed;
		m_mul_result = (uint32_t)((product + 8) >> 4) & MASK19;
		// WSP inside WXY - sets mix_l/mix_r from bus value
		if (wsp) {
			m_mix_l = (bus >> 3) & 0x7;
			m_mix_r = bus & 0x7;
		}
	}

	// clearB
	if (!BIT(inst, 2)) {
		m_b = 0;
		update_carry();  // B changed

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
				if (ext_data & 0x800) ext_data |= 0xF000;
				m_waveform_write(ext_addr, ext_data);
			}
		}
	}

	// WWF (Write Waveform)
	if (!BIT(inst, 1)) m_wf = (bus >> 9) & 0x1FF;

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
		m_l_acc += l_contrib;
		m_r_acc += r_contrib;
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
	// Accumulators are 24-bit, output upper 16 bits (per SAM8905 datasheet)
	out_l = (int32_t)m_l_acc >> 8;
	out_r = (int32_t)m_r_acc >> 8;

	// Store for slave mode stream output
	m_last_out_l = out_l;
	m_last_out_r = out_r;
}

void sam8905_device::sound_stream_update(sound_stream &stream)
{
	// In slave mode, just output the last computed values (set by process_frame)
	if (m_slave_mode) {
		for (int samp = 0; samp < stream.samples(); ++samp) {
			stream.put_int_clamp(0, samp, m_last_out_l, 32768);
			stream.put_int_clamp(1, samp, m_last_out_r, 32768);
		}
		return;
	}

	// Master mode: process frames independently
	// SSR bit (bit 3): 0 = 44.1kHz, 1 = 22.05kHz
	bool ssr_mode = BIT(m_control_reg, 3);

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
		stream.put_int_clamp(0, samp, (int32_t)m_l_acc >> 8, 32768);
		stream.put_int_clamp(1, samp, (int32_t)m_r_acc >> 8, 32768);

		// Fire sample output callback for inter-chip audio (FX processing)
		if (!m_sample_output.isunset()) {
			// Output upper 16 bits of 24-bit accumulator, clamped to 16-bit range
			int32_t l_out = std::clamp((int32_t)m_l_acc >> 8, -32768, 32767);
			int32_t r_out = std::clamp((int32_t)m_r_acc >> 8, -32768, 32767);
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
