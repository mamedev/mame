// SAM8905 Standalone Core Implementation
// For testing A-RAM/D-RAM captures without MAME dependencies

#include "sam8905_core.h"
#include <cstdio>
#include <fstream>

constexpr uint16_t SAM8905Core::MIX_ATTEN[8];

SAM8905Core::SAM8905Core()
{
	reset();
}

void SAM8905Core::reset()
{
	memset(m_aram, 0, sizeof(m_aram));
	memset(m_dram, 0, sizeof(m_dram));
	for (int i = 0; i < 16; ++i) {
		m_slots[i] = slot_t{};
	}
}

void SAM8905Core::load_aram(const uint16_t* data, size_t count, size_t offset)
{
	for (size_t i = 0; i < count && (offset + i) < 256; ++i) {
		m_aram[offset + i] = data[i] & 0x7FFF;
	}
}

void SAM8905Core::load_aram_file(const char* filename, int algo_slot)
{
	std::ifstream f(filename, std::ios::binary);
	if (!f) {
		fprintf(stderr, "Failed to open A-RAM file: %s\n", filename);
		return;
	}

	// Determine offset based on mode and slot
	size_t offset, inst_count;
	if (m_ssr_mode) {
		// 22.05kHz: 4 algorithms x 64 instructions
		offset = (algo_slot & 0x3) << 6;  // 64 words per algo
		inst_count = 64;
	} else {
		// 44.1kHz: 8 algorithms x 32 instructions
		offset = (algo_slot & 0x7) << 5;  // 32 words per algo
		inst_count = 32;
	}

	// Read 15-bit words (stored as 2 bytes each, big-endian in ROM)
	for (size_t i = 0; i < inst_count; ++i) {
		uint8_t hi, lo;
		if (!f.read(reinterpret_cast<char*>(&hi), 1)) break;
		if (!f.read(reinterpret_cast<char*>(&lo), 1)) break;
		m_aram[offset + i] = ((hi << 8) | lo) & 0x7FFF;
	}

	printf("Loaded A-RAM: %s -> slot %d (offset %zu, %zu words)\n",
		   filename, algo_slot, offset, inst_count);
}

void SAM8905Core::load_dram(const uint32_t* data, size_t count, size_t offset)
{
	for (size_t i = 0; i < count && (offset + i) < 256; ++i) {
		m_dram[offset + i] = data[i] & MASK19;
	}
}

void SAM8905Core::load_dram_file(const char* filename)
{
	std::ifstream f(filename, std::ios::binary);
	if (!f) {
		fprintf(stderr, "Failed to open D-RAM file: %s\n", filename);
		return;
	}

	// Read 19-bit words (stored as 3 bytes each)
	size_t i = 0;
	while (i < 256) {
		uint8_t b0, b1, b2;
		if (!f.read(reinterpret_cast<char*>(&b0), 1)) break;
		if (!f.read(reinterpret_cast<char*>(&b1), 1)) break;
		if (!f.read(reinterpret_cast<char*>(&b2), 1)) break;
		m_dram[i++] = ((b2 << 16) | (b1 << 8) | b0) & MASK19;
	}

	printf("Loaded D-RAM: %s (%zu words)\n", filename, i);
}

// ---------------------------------------------------------
// Arithmetic & Logic Helpers
// ---------------------------------------------------------

uint32_t SAM8905Core::get_constant(uint8_t mad)
{
	// From Appendix I - Constants derived from MAD
	static const uint32_t constants[16] = {
		0x00040, 0x02000, 0x04000, 0x06000, 0x08000, 0x0A000, 0x0C000, 0x0E000,
		0x10000, 0x12000, 0x14000, 0x16000, 0x18000, 0x1A000, 0x1C000, 0x1E000
	};
	return constants[mad & 0xf];
}

int32_t SAM8905Core::get_waveform(uint32_t wf, uint32_t phi, uint8_t mad)
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
				case 0: return (phi < 1024) ? phi * 2 : (phi < 3072) ? (phi * 2) - 4096 : (phi * 2) - 8192;
				case 1: return get_constant(mad);
				case 2: return (phi < 2048) ? phi : phi - 4096;
				case 3: return (phi < 2048) ? phi / 2 : (phi / 2) - 2048;
			}
		}
	} else {
		// External memory access - not implemented in standalone test
		return 0;
	}
	return 0;
}

// ---------------------------------------------------------
// Core Interpreter
// ---------------------------------------------------------

void SAM8905Core::execute_cycle(int slot_idx, uint16_t inst)
{
	slot_t &slot = m_slots[slot_idx];
	uint8_t mad = (inst >> 11) & 0xF;
	uint8_t emitter_sel = (inst >> 9) & 0x3;
	bool wsp = BIT(inst, 8);

	uint32_t bus = 0;
	uint32_t dram_addr = (slot_idx << 4) | mad;

	// Emitters
	switch (emitter_sel) {
		case 0: bus = m_dram[dram_addr]; break;           // RM
		case 1: bus = (slot.a + slot.b) & MASK19; break;  // RADD
		case 2: bus = slot.mul_result; break;             // RP
		case 3: bus = 0; break;                           // RSP (NOP)
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
		}
		if (write_enable) {
			m_dram[dram_addr] = bus;
		}
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

// ---------------------------------------------------------
// Audio Generation
// ---------------------------------------------------------

void SAM8905Core::generate_samples(int16_t* buffer, size_t num_samples)
{
	for (size_t samp = 0; samp < num_samples; ++samp) {
		int32_t out_l = 0, out_r = 0;

		for (int s = 0; s < 16; ++s) {
			m_slots[s].l_acc = 0;
			m_slots[s].r_acc = 0;

			// Fetch control from D-RAM word 15
			// Format: | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
			uint32_t param15 = m_dram[(s << 4) | 15];
			if (BIT(param15, 11)) continue; // I (Idle) bit - slot produces no sound

			uint16_t pc_start;
			int inst_count;

			if (m_ssr_mode) {
				// 22.05kHz mode: 4 algorithms x 64 instructions
				// Per Table 6: A-RAM address uses AL2,AL1 (bits 10-9) not AL1,AL0 (bits 9-8)
				uint8_t alg = (param15 >> 9) & 0x3;
				pc_start = alg << 6;  // 64 instructions per block
				inst_count = 64;
			} else {
				// 44.1kHz mode: 8 algorithms x 32 instructions
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

		// Clamp to 16-bit range
		if (out_l > 32767) out_l = 32767;
		if (out_l < -32768) out_l = -32768;
		if (out_r > 32767) out_r = 32767;
		if (out_r < -32768) out_r = -32768;

		buffer[samp * 2] = (int16_t)out_l;
		buffer[samp * 2 + 1] = (int16_t)out_r;
	}
}

// ---------------------------------------------------------
// Debug
// ---------------------------------------------------------

void SAM8905Core::dump_slot(int slot_idx) const
{
	const slot_t& s = m_slots[slot_idx];
	printf("Slot %d:\n", slot_idx);
	printf("  A=%05X B=%05X phi=%03X wf=%03X\n", s.a, s.b, s.phi, s.wf);
	printf("  X=%03X Y=%03X mul=%05X\n", s.x, s.y, s.mul_result);
	printf("  mix_l=%d mix_r=%d l_acc=%08X r_acc=%08X\n", s.mix_l, s.mix_r, s.l_acc, s.r_acc);

	printf("  D-RAM[%d]:", slot_idx);
	for (int w = 0; w < 16; ++w) {
		printf(" %05X", m_dram[(slot_idx << 4) | w]);
	}
	printf("\n");
}

void SAM8905Core::dump_aram(int algo_slot) const
{
	size_t offset, inst_count;
	if (m_ssr_mode) {
		offset = (algo_slot & 0x3) << 6;
		inst_count = 64;
	} else {
		offset = (algo_slot & 0x7) << 5;
		inst_count = 32;
	}

	printf("A-RAM Algo %d (offset %zu, %zu inst):\n", algo_slot, offset, inst_count);
	for (size_t i = 0; i < inst_count; i += 8) {
		printf("  %02zu:", i);
		for (size_t j = 0; j < 8 && (i + j) < inst_count; ++j) {
			printf(" %04X", m_aram[offset + i + j]);
		}
		printf("\n");
	}
}
