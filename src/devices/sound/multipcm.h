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

	void write(offs_t offset, uint8_t data);
	uint8_t read();

private:
	// internal state
	uint32_t m_cur_slot;
	uint32_t m_address;

	static const int32_t VALUE_TO_CHANNEL[32];

	void init_sample(sample_t &sample, uint32_t index);

	void write_slot(slot_t &slot, int32_t reg, uint8_t data);
};

DECLARE_DEVICE_TYPE(MULTIPCM, multipcm_device)

#endif // MAME_SOUND_MULTIPCM_H
