// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
#ifndef MAME_SOUND_MULTIPCM_H
#define MAME_SOUND_MULTIPCM_H

#pragma once

#include "gew.h"

class multipcm_device : public gew_pcm_device
{
public:
	multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// effect DSP interface (YMW258-F): adds sound output 2 for the send
	multipcm_device &enable_dsp_send() { m_dsp_send_enable = true; return *this; }
	auto dsp_cd_callback() { return m_dsp_cd_cb.bind(); }

	void write(offs_t offset, uint8_t data);
	uint8_t read();

private:
	devcb_write8 m_dsp_cd_cb;

	// internal state
	uint32_t m_cur_slot;
	uint32_t m_address;

	static const int32_t VALUE_TO_CHANNEL[32];

	void init_sample(sample_t &sample, uint32_t index);

	void write_slot(slot_t &slot, int32_t reg, uint8_t data);
};

DECLARE_DEVICE_TYPE(MULTIPCM, multipcm_device)

#endif // MAME_SOUND_MULTIPCM_H
