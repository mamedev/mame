// license:BSD-3-Clause
// copyright-holders:Devin Acker
#ifndef MAME_SOUND_GEW7_H
#define MAME_SOUND_GEW7_H

#pragma once

#include "gew.h"

class gew7_pcm_device : public gew_pcm_device
{
public:
	gew7_pcm_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	void write_hi(offs_t offset, uint8_t data);
	uint8_t read_hi(offs_t offset);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void init_sample(sample_t& sample, uint32_t index);
};

DECLARE_DEVICE_TYPE(GEW7_PCM, gew7_pcm_device)

#endif // MAME_SOUND_GEW7_H
