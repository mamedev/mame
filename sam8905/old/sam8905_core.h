// SAM8905 Standalone Core for Testing
// Extracted from MAME sam8905.cpp for standalone A-RAM/D-RAM testing
#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <cmath>

class SAM8905Core {
public:
	SAM8905Core();
	~SAM8905Core() = default;

	// Configuration
	void set_ssr_mode(bool ssr) { m_ssr_mode = ssr; }
	bool get_ssr_mode() const { return m_ssr_mode; }

	// Memory loading
	void load_aram(const uint16_t* data, size_t count, size_t offset = 0);
	void load_aram_file(const char* filename, int algo_slot);
	void load_dram(const uint32_t* data, size_t count, size_t offset = 0);
	void load_dram_file(const char* filename);

	// Direct memory access
	void write_aram(size_t addr, uint16_t value) { if (addr < 256) m_aram[addr] = value & 0x7FFF; }
	void write_dram(size_t addr, uint32_t value) { if (addr < 256) m_dram[addr] = value & MASK19; }
	uint16_t read_aram(size_t addr) const { return addr < 256 ? m_aram[addr] : 0; }
	uint32_t read_dram(size_t addr) const { return addr < 256 ? m_dram[addr] : 0; }

	// Audio generation
	void reset();
	void generate_samples(int16_t* buffer, size_t num_samples);

	// Debug
	void dump_slot(int slot_idx) const;
	void dump_aram(int algo_slot) const;

private:
	struct slot_t {
		uint32_t a = 0, b = 0;
		uint32_t x = 0, y = 0;
		uint32_t phi = 0, wf = 0;
		uint32_t l_acc = 0, r_acc = 0;
		uint8_t mix_l = 0, mix_r = 0;
		bool clear_rqst = false, int_mod = false;
		uint32_t mul_result = 0;
	};

	// State
	uint16_t m_aram[256] = {};  // 256 x 15-bit micro-instructions
	uint32_t m_dram[256] = {};  // 256 x 19-bit parameters
	slot_t m_slots[16] = {};
	bool m_ssr_mode = true;     // true = 22.05kHz (Keyfox10), false = 44.1kHz

	// Helpers
	void execute_cycle(int slot_idx, uint16_t inst);
	int32_t get_waveform(uint32_t wf, uint32_t phi, uint8_t mad);
	uint32_t get_constant(uint8_t mad);

	// dB attenuation lookup (MIX code 0-7)
	// 000: mute, 001: -36dB, 010: -30dB, 011: -24dB
	// 100: -18dB, 101: -12dB, 110: -6dB, 111: 0dB
	static constexpr uint16_t MIX_ATTEN[8] = {
		0,     // 000: mute
		16,    // 001: -36dB (1/64)
		32,    // 010: -30dB (1/32)
		64,    // 011: -24dB (1/16)
		128,   // 100: -18dB (1/8)
		256,   // 101: -12dB (1/4)
		512,   // 110: -6dB  (1/2)
		1024   // 111: 0dB   (1/1)
	};

	static constexpr uint32_t MASK19 = 0x7FFFF;

	// Bit extraction helper
	static constexpr bool BIT(uint32_t val, int bit) { return (val >> bit) & 1; }
};
