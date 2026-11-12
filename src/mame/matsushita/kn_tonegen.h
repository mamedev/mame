// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Tone generators of the KN2400, KN6000 and KN7000 families.

#ifndef MAME_MATSUSHITA_KN_TONEGEN_H
#define MAME_MATSUSHITA_KN_TONEGEN_H

#pragma once

#include <memory>
#include <utility>
#include <vector>

// One tone generator LSI: a register window on the CPU bus (map) and two wave
// ROM buses, Y and X (address spaces 0 and 1), each with a 24-bit word
// address. The internal ROMs fill the bottom of each bus; the top address bits
// select the expansion boards' ROMs.
class kn_tonegen_base_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	void map(address_map &map) ATTR_COLD;

protected:
	kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int inputs);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

	// The firmware latches a register address, then writes its data or reads
	// the chip's answer
	virtual void tg_write(uint16_t addr, uint16_t data);
	virtual uint16_t tg_read(uint16_t addr);

	uint16_t reg(uint16_t addr) const { return m_regs[addr]; }
	uint16_t wave_word(uint32_t word);

	sound_stream *m_stream;

private:
	const int m_inputs;
	address_space_config m_wave_config[2];
	required_memory_region_array<2> m_wave_rom;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::cache m_wave[2];

	std::unique_ptr<uint16_t []> m_regs;
	uint16_t m_addr;
	uint16_t m_wave_bank;
	uint16_t m_wave_addr;

	void wave_bus_map(address_map &map) ATTR_COLD;

	void addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t data_r();
	void data_w(uint16_t data);
	void wave_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wave_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t wave_data_r();
};

// KN6000/KN6500: IC213, 64 voices
class kn6000_tonegen_device : public kn_tonegen_base_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	kn6000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// KN2400/KN2600: one tone generator
class kn2400_tonegen_device : public kn_tonegen_base_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	kn2400_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// KN7000: IC201 (main) or IC205 (sub), 64 voices. The sub's output is routed
// into the main's inputs, which add it to the main's own voices.
class kn7000_tonegen_device : public kn_tonegen_base_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	kn7000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// kn_tonegen_base_device implementation
	virtual void tg_write(uint16_t addr, uint16_t data) override;
	virtual uint16_t tg_read(uint16_t addr) override;

private:
	static constexpr int32_t VOICES = 64;
	static constexpr int32_t PAGES = 16;
	static constexpr uint32_t STREAM_WORDS = PAGES << 20;

	enum : uint8_t { EG_ATTACK, EG_DECAY1, EG_DECAY2, EG_RELEASE };

	struct sample_info
	{
		uint32_t start = 0;                 // first wave word; reversed, one past the last
		uint32_t loop_start = 0;            // in samples
		uint32_t loop_end = 0;
		uint8_t exponent = 0;
		bool is8 = false;                   // two 8-bit samples per word
		bool reverse = false;               // plays from start - 1 down
		bool valid = false;
		uint32_t loop_seg = 0;              // 8-bit: the gain segment of loop_start
		std::vector<std::pair<uint32_t, float>> gain;  // 8-bit: segment end, gain
	};

	std::vector<sample_info> m_samples;
	uint32_t m_page_first[PAGES];
	uint32_t m_page_count[PAGES];
	double m_attack_time[256];
	double m_decay_coef[256];

	bool m_keyed[VOICES];
	uint8_t m_stage[VOICES];
	uint8_t m_release_rate[VOICES];
	uint32_t m_select[VOICES];
	uint32_t m_pitch[VOICES];
	int32_t m_sample[VOICES];
	double m_env[VOICES];
	double m_pos[VOICES];
	double m_step[VOICES];
	uint32_t m_seg[VOICES];             // 8-bit: the gain segment of the sample at m_pos

	void parse_samples() ATTR_COLD;
	double fetch(const sample_info &s, uint32_t i, uint32_t seg);
	double step(int v) const;
	void key_on(int v);
	void release(int v, uint8_t rate);
	uint16_t level(int v) const;
};

DECLARE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device)
DECLARE_DEVICE_TYPE(KN2400_TONEGEN, kn2400_tonegen_device)
DECLARE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device)

#endif // MAME_MATSUSHITA_KN_TONEGEN_H
