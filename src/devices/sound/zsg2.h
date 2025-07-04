// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap, superctr
/*
    ZOOM ZSG-2 custom wavetable synthesizer
*/

#ifndef MAME_SOUND_ZSG2_H
#define MAME_SOUND_ZSG2_H

#pragma once

// ======================> zsg2_device

class zsg2_device : public device_t,
					public device_sound_interface
{
public:
	zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto ext_read() { return m_ext_read_handler.bind(); }

	uint16_t read(offs_t offset, uint16_t mem_mask = ~0);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	const uint16_t STATUS_ACTIVE = 0x8000;

	// 16 registers per channel, 48 channels
	struct zchan
	{
		uint16_t v[16];
		uint16_t status;
		uint32_t cur_pos;
		uint32_t step_ptr;
		uint32_t step;
		uint32_t start_pos;
		uint32_t end_pos;
		uint32_t loop_pos;
		uint32_t page;

		uint16_t vol;
		uint16_t vol_initial;
		uint16_t vol_target;
		int16_t vol_delta;

		uint16_t output_cutoff;
		uint16_t output_cutoff_initial;
		uint16_t output_cutoff_target;
		int16_t output_cutoff_delta;

		int32_t emphasis_filter_state;
		int32_t output_filter_state;

		uint8_t output_gain[4]; // Attenuation for output channels
		int16_t samples[5]; // +1 history
	};

	uint16_t m_gain_tab[256];
	uint16_t m_reg[32];

	zchan m_chan[48];
	uint32_t m_sample_count;

	required_region_ptr<uint32_t> m_mem_base;
	uint32_t m_read_address;
	std::unique_ptr<uint32_t[]> m_mem_copy;
	uint32_t m_mem_blocks;
	std::unique_ptr<int16_t[]> m_full_samples;

	sound_stream *m_stream;

	devcb_read32 m_ext_read_handler;

	uint32_t read_memory(uint32_t offset);
	void chan_w(int ch, int reg, uint16_t data);
	uint16_t chan_r(int ch, int reg);
	void control_w(int reg, uint16_t data);
	uint16_t control_r(int reg);
	int16_t *prepare_samples(uint32_t offset);
	void filter_samples(zchan *ch);
	int16_t get_ramp(uint8_t val);
	inline uint16_t ramp(uint16_t current, uint16_t target, int16_t delta);
};

DECLARE_DEVICE_TYPE(ZSG2, zsg2_device)

#endif // MAME_SOUND_ZSG2_H
