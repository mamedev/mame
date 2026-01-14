// license:BSD-3-Clause
// copyright-holders:Draft

#include "emu.h"
#include "sam8905.h"

// Global debug sequence counter
static int g_debug_seq = 0;

DEFINE_DEVICE_TYPE(SAM8905, sam8905_device, "sam8905", "Dream SAM8905")

sam8905_device::sam8905_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAM8905, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_waveform_read(*this, 0)
	, m_waveform_write(*this)
	, m_sample_output(*this)
{
}

void sam8905_device::device_start()
{
	m_aram = std::make_unique<uint16_t[]>(256);
	m_dram = std::make_unique<uint32_t[]>(256);

	m_stream = stream_alloc(0, 2, clock() / 1024); // Placeholder sample rate logic

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
}

void sam8905_device::device_reset()
{
	memset(m_slots, 0, sizeof(m_slots));
	m_control_reg = 0;
	m_interrupt_latch = 0;
}

// ---------------------------------------------------------
// Arithmetic & Logic Helpers
// ---------------------------------------------------------

uint32_t sam8905_device::get_constant(uint8_t mad)
{
	// From Appendix I - Constants derived from MAD
	static const uint32_t constants[16] = {
		0x00040, 0x02000, 0x04000, 0x06000, 0x08000, 0x0A000, 0x0C000, 0x0E000,
		0x10000, 0x12000, 0x14000, 0x16000, 0x18000, 0x1A000, 0x1C000, 0x1E000
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
			}
		}
	} else {
		// External memory access
		if (!m_waveform_read.isunset()) {
			// Build 20-bit address: WAVE[7:0] | PHI[11:0]
			uint32_t addr = ((wf & 0xFF) << 12) | phi;

			// Debug: trace waveform addresses for slot 2
			static int wf_trace = 0;
			if (slot_idx == 2 && wf_trace < 30) {
				logerror("EXT_WF S2: wf=0x%03X phi=0x%03X addr=0x%05X\n", wf, phi, addr);
				wf_trace++;
			}

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
	// Debug: trace slot 7 PC 5-7 at start of execute_cycle
	if (m_slave_mode && slot_idx == 7 && pc >= 5 && pc <= 7) {
		static int s7_early_trace = 0;
		if (s7_early_trace < 10) {
			logerror("Slot7 early PC%02d: inst=0x%04X wsp_bit=%d\n", pc, inst, BIT(inst, 8));
			s7_early_trace++;
		}
	}

	slot_t &slot = m_slots[slot_idx];
	uint8_t mad = (inst >> 11) & 0xF;
	uint8_t emitter_sel = (inst >> 9) & 0x3;
	bool wsp = BIT(inst, 8);


	uint32_t bus = 0;
	uint32_t dram_addr = (slot_idx << 4) | mad;

	// Emitters
	switch (emitter_sel) {
		case 0: bus = m_dram[dram_addr]; break; // RM
		case 1: bus = (slot.a + slot.b) & MASK19; break; // RADD
		case 2: bus = slot.mul_result; break; // RP
		case 3: bus = 0; break; // RSP (NOP)
	}

	// Helper lambda to update carry - called after any change to A or B
	// Carry calculation per Table 3:
	// - B positive (bit 18=0): carry = 19-bit overflow from addition
	// - B negative (bit 18=1): carry = result is positive (sign bit = 0)
	auto update_carry = [&slot]() {
		bool b_neg = BIT(slot.b, 18);
		uint32_t sum = slot.a + slot.b;
		if (!b_neg)
			slot.carry = (sum > MASK19);  // 19-bit overflow
		else
			slot.carry = !BIT(sum & MASK19, 18);  // For envelope: positive result
	};

	// Initial carry state from current A,B
	update_carry();

	// Receivers (Active Low except WSP)
	// WA (Write A)
	if (!BIT(inst, 7)) {
		bool wphi_active = !BIT(inst, 4);

		// Debug: trace WA/WA_WSP for slot 2 when active
		uint32_t p15 = m_dram[(slot_idx << 4) | 15];
		uint8_t alg = (p15 >> 8) & 0x7;
		if (slot_idx == 2 && alg == 2) {
			static int wa_trace = 0;
			if (wa_trace < 60) {
				logerror("WA S2: inst=0x%04X mad=%d bus=0x%05X wsp=%d wphi=%d -> %s\n",
					inst, mad, bus, wsp, wphi_active,
					(wsp && !wphi_active) ? "WA_WSP" : "normalWA");
				wa_trace++;
			}
		}

		// "WPHI WSP takes priority over WA WSP giving a normal WA" (Section 8-3)
		if (wsp && !wphi_active) {
			// WA WSP Truth Table - only when WPHI is NOT also active
			uint32_t wave = (bus >> 9) & 0x1FF;
			uint32_t final_wave = bus & 0x1FF;
			bool end_bit = BIT(bus, 18);
			bool wf_match = (wave == final_wave);

			// Debug: trace WA_WSP carry state for slot 2
			if (slot_idx == 2 && alg == 2) {
				static int wsp_trace = 0;
				if (wsp_trace < 30) {
					logerror("WA_WSP S2: A=0x%05X B=0x%05X wave=%03X final=%03X carry=%d\n",
						slot.a, slot.b, wave, final_wave, slot.carry);
					wsp_trace++;
				}
			}

			// Debug: trace when wave reaches finalWave for external sample slots
			if (wf_match && slot.carry && alg == 2) {
				static int match_count = 0;
				if (match_count < 20) {
					logerror("WA_WSP S%d: WAVE REACHED FINAL! wave=%03X final=%03X E=%d clrq->%d\n",
						slot_idx, wave, final_wave, end_bit, end_bit);
					match_count++;
				}
			}

			if (!slot.carry) {
				slot.a = 0; slot.clear_rqst = false; slot.int_mod = true;
			} else if (!wf_match) {
				slot.a = 0x200; slot.clear_rqst = false; slot.int_mod = true;
			} else {
				slot.a = 0; slot.int_mod = true;
				slot.clear_rqst = end_bit;
			}
			update_carry();  // A changed
		} else {
			// Normal WA: load A from bus
			// Per Section 8-3: "WA (without WSP) always sets CLEARRQST and clears INTMOD"
			slot.a = bus;
			slot.clear_rqst = true;
			slot.int_mod = false;
			update_carry();  // A changed
		}
	}

	// WB (Write B)
	if (!BIT(inst, 6)) {
		slot.b = bus;
		update_carry();  // B changed
	}

	// WM (Write Memory)
	if (!BIT(inst, 5)) {
		bool write_enable = true;
		if (wsp) {
			// WM WSP Truth Table (Section 8-4)
			if (!slot.clear_rqst) write_enable = false;
			else if (slot.carry) write_enable = false;

			// Interrupt latch: set when CLEARRQST=1 AND (INTMOD=1 OR CARRY=1)
			bool irq_condition = slot.clear_rqst && (slot.int_mod || slot.carry);
			if (irq_condition) {
				uint8_t new_latch = (slot_idx << 4) | mad;
				if (m_interrupt_latch != new_latch)
					logerror("SAM IRQ latch SET: slot %d word %d (latch %02X -> %02X) intmod=%d carry=%d\n",
						slot_idx, mad, m_interrupt_latch, new_latch, slot.int_mod, slot.carry);
				m_interrupt_latch = new_latch;
			}

			// Debug: trace when IRQ WOULD fire if intmod wasn't cleared
			if (slot_idx == 2 && mad == 2 && slot.clear_rqst && !irq_condition) {
				static int debug_count = 0;
				if (debug_count < 5) {
					logerror("WM_WSP S2W2: IRQ blocked - clrq=%d intmod=%d carry=%d (needs intmod=1 or carry=1)\n",
						slot.clear_rqst, slot.int_mod, slot.carry);
					debug_count++;
				}
			}
			// Debug: trace WM WSP for slots 0,1,2 when word 2 (amplitude)
			if (mad == 2 && slot_idx <= 2) {
				uint32_t p15 = m_dram[(slot_idx << 4) | 15];
				uint8_t alg = (p15 >> 8) & 0x7;
				if (alg > 0) { // only when slot is active
					static int trace_count = 0;
					if (trace_count < 40) {
						logerror("WM_WSP S%dW2: ALG=%d A=0x%05X B=0x%05X bus=0x%05X b_neg=%d carry=%d clrq=%d intmod=%d IRQ=%d\n",
							slot_idx, alg, slot.a, slot.b, bus, BIT(slot.b, 18), slot.carry, slot.clear_rqst, slot.int_mod,
							slot.clear_rqst && (slot.int_mod || slot.carry));
						trace_count++;
					}
				}
			}
		}
		if (write_enable) {
		// Debug: catch any write to D-RAM[0x75]
		if (m_slave_mode && dram_addr == 0x75) {
			uint32_t p15 = m_dram[(slot_idx << 4) | 15];
			logerror("SEQ%06d ALG-WRITE D[75]=0x%05X (was 0x%05X) inst=0x%04X slot=%d ALG=%d pc_start=0x%02X pc=%d\n",
				++g_debug_seq, bus, m_dram[dram_addr], inst, slot_idx, (p15 >> 8) & 7, pc_start, pc);
		}
		m_dram[dram_addr] = bus;
	}
	}

	// WPHI (Write Phase)
	// PHI register is 12 bits, extracted from bits 18:7 of the 19-bit bus
	// Lower 7 bits are fractional precision in D-RAM, upper 12 bits are the actual phase
	if (!BIT(inst, 4)) {
		slot.phi = (bus >> 7) & 0xFFF;  // Upper 12 bits for waveform address
		if (wsp) slot.wf = 0x100; // Force Internal Sinus
	}

	// WXY (Write Multiplier) - executes when bit 3 = 0
	if (!BIT(inst, 3)) {
		slot.y = (bus >> 7) & 0xFFF;
		slot.x = get_waveform(slot.wf, slot.phi, mad, slot_idx) & 0xFFF;
		// Debug: trace WXY for slot 0 ALG=1
		uint32_t p15_wxy = m_dram[(slot_idx << 4) | 15];
		uint8_t alg_wxy = (p15_wxy >> 8) & 0x7;
		if (slot_idx == 0 && alg_wxy == 1) {
			static int wxy_trace = 0;
			if (wxy_trace < 20) {
				logerror("WXY S0: wf=0x%03X phi=0x%03X x=%d y=%d\n",
					slot.wf, slot.phi, (int16_t)(slot.x << 4) >> 4, (int16_t)(slot.y << 4) >> 4);
				wxy_trace++;
			}
		}
		// Calculate multiplication result (12x12 fractional)
		int32_t product = (int32_t)((int16_t)(slot.x << 4) >> 4) * (int32_t)((int16_t)(slot.y << 4) >> 4);
		slot.mul_result = (uint32_t)(product >> 5) & MASK19;
	}

	// WSP (Write Special) - INDEPENDENT of WXY, executes when bit 8 = 1
	// Sets mix_l/mix_r from bus value for WACC accumulation
	if (wsp) {
		// Debug: trace WSP for FX chip slots 4-11
		static int wsp_trace = 0;
		if (m_slave_mode && slot_idx >= 4 && slot_idx <= 11 && wsp_trace < 50) {
			logerror("WSP slot%d pc=%d: bus=0x%05X -> mix_l=%d mix_r=%d\n",
				slot_idx, pc, bus, (bus >> 3) & 0x7, bus & 0x7);
			wsp_trace++;
		}
		slot.mix_l = (bus >> 3) & 0x7;
		slot.mix_r = bus & 0x7;
	}

	// clearB
	if (!BIT(inst, 2)) {
		slot.b = 0;
		update_carry();  // B changed

		// WWE (Write Waveform Enable) - triggered by RSP + clearB + WSP
		// RSP = emitter_sel==3, clearB = bit 2 active, WSP = bit 8 active
		// Writes waveform data to external RAM at WF+PHI address
		if (emitter_sel == 3 && wsp && !m_waveform_write.isunset()) {
			// Only write externally when WF indicates external memory (WF < 0x80)
			// WF >= 0x80 is input sample address space, WF >= 0x100 is internal waveform
			if ((slot.wf & 0x1FF) < 0x80) {
				// Build 15-bit address from WF[6:0] and PHI[11:4]
				// 32KB = 15 bits: WF[6:0] << 8 | PHI[7:0]
				uint32_t ext_addr = ((slot.wf & 0x7F) << 8) | ((slot.phi >> 4) & 0xFF);
				// Write data is from A register, converted to 12-bit
				int16_t ext_data = (slot.a >> 7) & 0xFFF;
				// Sign extend to 16-bit for callback
				if (ext_data & 0x800) ext_data |= 0xF000;
				m_waveform_write(ext_addr, ext_data);

				// Debug: trace external writes (limited)
				static int ext_write_trace = 0;
				if (ext_write_trace < 50) {
					logerror("WWE S%d: wf=0x%03X phi=0x%03X addr=0x%04X data=%d\n",
						slot_idx, slot.wf, slot.phi, ext_addr, ext_data);
					ext_write_trace++;
				}
			}
		}
	}

	// WWF (Write Waveform)
	if (!BIT(inst, 1)) slot.wf = (bus >> 9) & 0x1FF;

	// WACC (Accumulate) - with proper dB attenuation
	if (!BIT(inst, 0)) {
		// dB attenuation lookup: 000=mute, 001=-36dB, 010=-30dB, 011=-24dB,
		//                        100=-18dB, 101=-12dB, 110=-6dB, 111=0dB
		static constexpr uint16_t MIX_ATTEN[8] = { 0, 16, 32, 64, 128, 256, 512, 1024 };

		// Sign-extend 19-bit mul_result to 32-bit for proper signed math
		int32_t signed_mul = (slot.mul_result & MASK19);
		if (signed_mul & 0x40000) signed_mul |= 0xFFF80000;  // Sign extend from bit 18

		// Apply dB attenuation lookup
		int32_t l_contrib = (signed_mul * MIX_ATTEN[slot.mix_l]) >> 10;
		int32_t r_contrib = (signed_mul * MIX_ATTEN[slot.mix_r]) >> 10;
		slot.l_acc += l_contrib;
		slot.r_acc += r_contrib;

		// Debug: trace WACC in slave mode for slots with proper algorithms
		// Only log when slot 7 has proper ALG=4 (p15=0x3C480), meaning FX is fully initialized
		static int wacc_trace = 0;
		static bool fx_init_done = false;
		if (m_slave_mode && !fx_init_done) {
			uint32_t slot7_p15 = m_dram[(7 << 4) | 15];
			if (slot7_p15 == 0x3C480) fx_init_done = true;
		}
		if (m_slave_mode && fx_init_done && slot_idx >= 4 && slot_idx <= 11 && wacc_trace < 100) {
			logerror("WACC S%d: mul=%d mix_l=%d mix_r=%d l_c=%d r_c=%d\n",
				slot_idx, signed_mul, slot.mix_l, slot.mix_r, l_contrib, r_contrib);
			wacc_trace++;
		}
	}
}

// Process a single audio frame (16 slots) - used for master-slave synchronization
void sam8905_device::process_frame(int32_t &out_l, int32_t &out_r)
{
	// FX chip: wait for initialization to complete before processing
	// The firmware programs A-RAM before setting final slot configurations
	// Use slot 7's p15=0x3C480 as initialization complete marker
	static bool fx_init_complete = false;
	if (m_slave_mode && !fx_init_complete) {
		uint32_t slot7_p15 = m_dram[(7 << 4) | 15];
		if (slot7_p15 == 0x3C480) {
			fx_init_complete = true;
			logerror("FX init complete - starting audio processing\n");
		} else {
			out_l = 0;
			out_r = 0;
			return;  // Skip processing until initialized
		}
	}

	// SSR bit (bit 3): 0 = 44.1kHz, 1 = 22.05kHz
	bool ssr_mode = BIT(m_control_reg, 3);

	out_l = 0;
	out_r = 0;

	for (int s = 0; s < 16; ++s) {
		m_slots[s].l_acc = 0;
		m_slots[s].r_acc = 0;

		// Fetch algorithm block from address 15 of D-RAM block
		// Word 15 format: | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
		uint32_t param15 = m_dram[(s << 4) | 15];

		// Debug: trace param15 changes in slave mode
		static uint32_t last_p15[16] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
		if (m_slave_mode && param15 != last_p15[s]) {
			logerror("FX slot %d: p15=0x%05X %s ALG=%d\n", s, param15,
				BIT(param15, 11) ? "IDLE" : "ACTIVE", (param15 >> 8) & 0x7);
			// Dump ALG=0 A-RAM when slot 7 first gets ALG=4 (which becomes ALG=0 in 22kHz mode)
			if (s == 7 && param15 == 0x3C480) {
				logerror("ALG=0 A-RAM (22kHz mode, base=0x00) for slot 7:\n");
				for (int i = 0; i < 64; i++) {
					uint16_t inst = m_aram[i];
					bool wsp_flag = (inst >> 8) & 1;
					logerror("  PC%02d: 0x%04X%s\n", i, inst, wsp_flag ? " [WSP]" : "");
				}
			}
			last_p15[s] = param15;
		}

		if (BIT(param15, 11)) {
			// I (Idle) bit - slot produces no sound
			continue;
		}

		uint16_t pc_start;
		int inst_count;

		if (ssr_mode) {
			// 22.05kHz mode: 4 algorithms × 64 instructions
			uint8_t alg = (param15 >> 8) & 0x3;
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

		// Debug: count and trace slot 7 with ALG=4
		static int s7_trace = 0;
		if (m_slave_mode && s == 7 && ((param15 >> 8) & 0x7) == 4) {
			int addr = (7 << 4) | 5;  // 0x75
			uint32_t word5 = m_dram[addr];
			if (s7_trace < 5) {
				logerror("SEQ%06d READ this=%p addr=%02X val=0x%05X mix_l=%d mix_r=%d\n",
					++g_debug_seq, (void*)this, addr, word5, m_slots[s].mix_l, m_slots[s].mix_r);
				s7_trace++;
			}
		}

		// Debug: trace slot execution in slave mode
		static int slot_exec_trace = 0;
		if (m_slave_mode && (m_slots[s].l_acc != 0 || m_slots[s].r_acc != 0) && slot_exec_trace < 50) {
			logerror("FX slot %d: p15=0x%05X ALG=%d mix_l=%d mix_r=%d l_acc=%d r_acc=%d\n",
				s, param15, (param15 >> 8) & 0x7, m_slots[s].mix_l, m_slots[s].mix_r,
				(int32_t)m_slots[s].l_acc, (int32_t)m_slots[s].r_acc);
			slot_exec_trace++;
		}

		out_l += (int32_t)m_slots[s].l_acc;
		out_r += (int32_t)m_slots[s].r_acc;
	}

	// Store for slave mode stream output
	m_last_out_l = out_l;
	m_last_out_r = out_r;

	// Debug: trace non-zero output in slave mode
	static int fx_out_trace = 0;
	if ((out_l != 0 || out_r != 0) && fx_out_trace < 50) {
		logerror("process_frame output: L=%d R=%d\n", out_l, out_r);
		fx_out_trace++;
	}
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
		int32_t out_l = 0, out_r = 0;

		for (int s = 0; s < 16; ++s) {
			m_slots[s].l_acc = 0;
			m_slots[s].r_acc = 0;

			// Fetch algorithm block from address 15 of D-RAM block
			// Word 15 format: | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
			uint32_t param15 = m_dram[(s << 4) | 15];
			if (BIT(param15, 11)) {
				// I (Idle) bit - slot produces no sound
				static uint32_t idle_trace[16] = {0};
				if (param15 != idle_trace[s]) {
					logerror("Slot %d IDLE: param15=0x%05X\n", s, param15);
					idle_trace[s] = param15;
				}
				continue;
			}

			uint16_t pc_start;
			int inst_count;

			if (ssr_mode) {
				// 22.05kHz mode: 4 algorithms × 64 instructions
				uint8_t alg = (param15 >> 8) & 0x3;  // Only 2 bits
				pc_start = alg << 6;  // 64 instructions per block
				inst_count = 64;
			} else {
				// 44.1kHz mode: 8 algorithms × 32 instructions
				uint8_t alg = (param15 >> 8) & 0x7;  // 3 bits
				pc_start = alg << 5;  // 32 instructions per block
				inst_count = 32;
				// Debug: trace all active slots
				static uint32_t last_p15[16] = {0};
				if (param15 != last_p15[s]) {
					logerror("Slot %d active: ALG=%d param15=0x%05X\n", s, alg, param15);
					// Dump ALG microcode when slot becomes active
					if ((s <= 2 && alg == 1) || (s == 2 && alg == 2)) {
						for (int i = 0; i < 30; i++) {
							logerror("ALG%d[%02d]: 0x%04X\n", alg, i, m_aram[pc_start + i]);
						}
					}
					last_p15[s] = param15;
				}
			}

			// Skip reserved instructions (last 2)
			for (int pc = 0; pc < inst_count - 2; ++pc) {
				execute_cycle(s, m_aram[pc_start + pc], pc_start, pc);
			}

			out_l += (int32_t)m_slots[s].l_acc;
			out_r += (int32_t)m_slots[s].r_acc;

			// Trace slot output when non-zero (once per slot activation)
			static int32_t last_out[16] = {0};
			int32_t cur_out = m_slots[s].l_acc + m_slots[s].r_acc;
			if (cur_out != 0 && last_out[s] == 0) {
				uint8_t alg = (param15 >> 8) & 0x7;
				logerror("Slot %d OUTPUT: alg=%d l=%d r=%d w6=%05X w7=%05X\n",
					s, alg, m_slots[s].l_acc, m_slots[s].r_acc,
					m_dram[(s << 4) | 6], m_dram[(s << 4) | 7]);
			}
			last_out[s] = cur_out;
		}

		stream.put_int_clamp(0, samp, out_l, 32768);
		stream.put_int_clamp(1, samp, out_r, 32768);

		// Fire sample output callback for inter-chip audio (FX processing)
		if (!m_sample_output.isunset()) {
			// Pack L/R as 32-bit value: upper 16 = L, lower 16 = R
			uint32_t packed = ((out_l & 0xFFFF) << 16) | (out_r & 0xFFFF);
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
					logerror("A-RAM[%02X] = 0x%04X\n", m_address_reg, m_data_latch & 0x7FFF);
				} else {
					m_dram[m_address_reg] = m_data_latch & MASK19;
					// Debug: log D-RAM writes for slot 7 (all words initially, then filter)
					int slot = (m_address_reg >> 4) & 0xF;
					int word = m_address_reg & 0xF;
					if (m_slave_mode && slot == 7) {
						logerror("FX D-RAM[%02X] = 0x%05X (slot %d, word %d)\n",
							m_address_reg, m_data_latch & MASK19, slot, word);
					}
					if ((slot <= 2) && (word == 2 || word == 3 || word == 15)) {
						logerror("CPU D-RAM[%02X] = 0x%05X (slot %d, word %d)\n",
							m_address_reg, m_data_latch & MASK19, slot, word);
					}
				}
			} else { // WR=0: Read Request - latch RAM data
				if (BIT(data, 1)) {
					// A-RAM selected (15-bit)
					m_data_latch = m_aram[m_address_reg] & 0x7FFF;
				} else {
					// D-RAM selected (19-bit)
					m_data_latch = m_dram[m_address_reg] & MASK19;
					// Log D-RAM reads for slot 2
					int slot = (m_address_reg >> 4) & 0xF;
					int word = m_address_reg & 0xF;
					if (slot == 2) {
						logerror("CPU D-RAM READ[%02X] = 0x%05X (slot %d, word %d)\n",
							m_address_reg, m_data_latch, slot, word);
					}
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
	if (offset == 0) {
		logerror("SAM read IRQ latch: %02X\n", result);
	}
	return result;
}
