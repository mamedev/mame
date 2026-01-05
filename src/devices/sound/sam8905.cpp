// license:BSD-3-Clause
// copyright-holders:Draft

#include "emu.h"
#include "sam8905.h"

DEFINE_DEVICE_TYPE(SAM8905, sam8905_device, "sam8905", "Dream SAM8905")

sam8905_device::sam8905_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAM8905, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_waveform_read(*this, 0)
{
}

void sam8905_device::device_start()
{
	m_aram = std::make_unique<uint16_t[]>(256);
	m_dram = std::make_unique<uint32_t[]>(256);

	m_stream = stream_alloc(0, 2, clock() / 1024); // Placeholder sample rate logic

	save_pointer(NAME(m_aram), 256);
	save_pointer(NAME(m_dram), 256);
	save_item(NAME(m_control_reg));
	save_item(NAME(m_address_reg));
}

void sam8905_device::device_reset()
{
	memset(m_slots, 0, sizeof(m_slots));
	m_control_reg = 0;
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

int32_t sam8905_device::get_waveform(uint32_t wf, uint32_t phi, uint8_t mad)
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

void sam8905_device::execute_cycle(int slot_idx, uint16_t inst)
{
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

	// Carry calculation (Section 8-1)
	bool b_neg = BIT(slot.b, 18);
	bool carry;
	if (!b_neg)
		carry = ((uint64_t)slot.a + slot.b) > MASK19;
	else
		carry = !BIT((slot.a + slot.b) & MASK19, 18);

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

			if (!carry) {
				slot.a = 0; slot.clear_rqst = false; slot.int_mod = true;
			} else if (!wf_match) {
				slot.a = 0x200; slot.clear_rqst = false; slot.int_mod = true;
			} else {
				slot.a = 0; slot.int_mod = true;
				slot.clear_rqst = end_bit;
			}
		} else {
			// Normal WA: load A from bus
			slot.a = bus;
			slot.clear_rqst = true;
			slot.int_mod = false;
		}
	}

	// WB (Write B)
	if (!BIT(inst, 6)) slot.b = bus;

	// WM (Write Memory)
	if (!BIT(inst, 5)) {
		bool write_enable = true;
		if (wsp) {
			// WM WSP Truth Table (Section 8-4)
			if (!slot.clear_rqst) write_enable = false;
			else if (carry) write_enable = false;
			// Interrupt logic would trigger here based on slot.int_mod
		}
		if (write_enable) m_dram[dram_addr] = bus;
	}

	// WPHI (Write Phase)
	if (!BIT(inst, 4)) {
		slot.phi = (bus >> 7) & 0xFFF;
		if (wsp) slot.wf = 0x100; // Force Internal Sinus
	}

	// WXY (Write Multiplier)
	if (!BIT(inst, 3)) {
		slot.y = (bus >> 7) & 0xFFF;
		slot.x = get_waveform(slot.wf, slot.phi, mad) & 0xFFF;
		if (wsp) {
			slot.mix_l = (bus >> 3) & 0x7;
			slot.mix_r = bus & 0x7;
		}
		// Calculate multiplication result (12x12 fractional)
		int32_t product = (int32_t)((int16_t)(slot.x << 4) >> 4) * (int32_t)((int16_t)(slot.y << 4) >> 4);
		slot.mul_result = (uint32_t)(product >> 5) & MASK19;
	}

	// clearB
	if (!BIT(inst, 2)) slot.b = 0;

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
	}
}

void sam8905_device::sound_stream_update(sound_stream &stream)
{
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
			if (BIT(param15, 11)) continue; // I (Idle) bit - slot produces no sound

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
			}

			// Skip reserved instructions (last 2)
			for (int pc = 0; pc < inst_count - 2; ++pc) {
				execute_cycle(s, m_aram[pc_start + pc]);
			}

			out_l += (int32_t)m_slots[s].l_acc;
			out_r += (int32_t)m_slots[s].r_acc;
		}

		stream.put_int_clamp(0, samp, out_l, 32768);
		stream.put_int_clamp(1, samp, out_r, 32768);
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
			if (BIT(data, 0)) { // Write Request
				if (BIT(data, 1)) m_aram[m_address_reg] = m_data_latch & 0x7FFF;
				else m_dram[m_address_reg] = m_data_latch & MASK19;
			}
			break;
	}
}

uint8_t sam8905_device::read(offs_t offset)
{
	// Similar logic to write for reading D-RAM/A-RAM via data latch
	return 0;
}
