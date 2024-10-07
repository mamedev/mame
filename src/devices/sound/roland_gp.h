// license:BSD-3-Clause
// copyright-holders:giulioz
#ifndef MAME_SOUND_ROLAND_GP_H
#define MAME_SOUND_ROLAND_GP_H

#pragma once

#include "dirom.h"

class tc6116_device : public device_t, public device_sound_interface, public device_rom_interface<23>
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	tc6116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_int_callback.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	static constexpr unsigned NUM_CHANNELS = 28;

	struct pcm_channel
	{
		pcm_channel() { }

		// registers
		uint8_t pitch_coarse;
		uint8_t pitch_fine;
		uint8_t pan_l;
		uint8_t pan_r;
		uint8_t rev_send;
		uint8_t chorus_send;
		uint8_t volume1;
		uint8_t volume1_speed;
		uint8_t volume2;
		uint8_t volume2_speed;
		uint8_t cutoff;
		uint8_t cutoff_speed;

		bool irq_enable; // 0
		bool filter_mode; // 1 (0:lpf, 1:hpf)
		uint8_t resonance_flags; // 8-15

		uint8_t sub_phase_addr; // 0-4
		bool key; // 5
		bool alt_loop; // 6
		bool reverse_play; // 7
		uint8_t hiaddr; // 8-11
		uint8_t nibble; // 2-15

		// work variables
		uint16_t sub_phase_state; // 0-13
		bool irq_disable; // 14
		bool alt_loop_state; // 15

		uint16_t volume1_tv;
		uint16_t volume2_tv;
		uint16_t cutoff_tv;
	};

	devcb_write_line m_int_callback;

	uint32_t m_clock;                   // clock
	uint32_t m_rate;                    // sample rate (usually 32000 Hz)
	sound_stream *m_stream;             // stream handle
	pcm_channel m_chns[NUM_CHANNELS];   // channel memory
	[[maybe_unused]] uint8_t m_sel_chn; // selected channel
};

DECLARE_DEVICE_TYPE(TC6116, tc6116_device)

#endif // MAME_SOUND_ROLAND_GP_H
