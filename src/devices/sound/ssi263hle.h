// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Silicon Systems SSI-263A Phoneme Speech Synthesizer

    Temporary implementation using the Votrax SC-01A

    NOTE: This is completely wrong, and exists only to have
    working audio in Thayer's Quest, which would not otherwise
    be playable due to relying on speech output for important
    gameplay cues.

**********************************************************************/

#ifndef MAME_SOUND_SSI263HLE_H
#define MAME_SOUND_SSI263HLE_H

#include "sound/votrax.h"


#pragma once

class ssi263hle_device : public device_t, public device_mixer_interface
{
public:
	ssi263hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void map(address_map &map);

	auto ar_callback() { return m_ar_cb.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<votrax_sc01_device> m_votrax;

	TIMER_CALLBACK_MEMBER(phoneme_tick);

	void duration_phoneme_w(u8 data);
	void inflection_w(u8 data);
	void rate_inflection_w(u8 data);
	void control_articulation_amplitude_w(u8 data);
	void filter_frequency_w(u8 data);
	u8 status_r();

	void votrax_request(int state);

	devcb_write_line m_ar_cb;

	emu_timer *m_phoneme_timer = nullptr;

	u8 m_duration = 0;
	u8 m_phoneme = 0;
	u16 m_inflection = 0;
	u8 m_rate = 0;
	u8 m_articulation = 0;
	bool m_control = false;
	u8 m_amplitude = 0;
	u8 m_filter = 0;
	u8 m_mode = 0;
	u8 m_data_request = 1;

	u8 m_votrax_fifo[1024];
	u32 m_votrax_fifo_wr = 0;
	u32 m_votrax_fifo_rd = 0;
	u32 m_votrax_fifo_cnt = 0;

	static const char PHONEME_NAMES[0x40][5];
	static const u8 PHONEMES_TO_SC01[0x40];
};

DECLARE_DEVICE_TYPE(SSI263HLE, ssi263hle_device)

#endif
