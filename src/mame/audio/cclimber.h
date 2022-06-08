// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cclimber.h

    Functions to emulate the cclimber audio boards

***************************************************************************/
#ifndef MAME_AUDIO_CCLIMBER_H
#define MAME_AUDIO_CCLIMBER_H

#pragma once

#include "sound/samples.h"

DECLARE_DEVICE_TYPE(CCLIMBER_AUDIO, cclimber_audio_device)

// ======================> cclimber_audio_device

class cclimber_audio_device : public device_t
{
public:
	// construction/destruction
	cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto &set_sample_clockdiv(uint8_t div) { m_sample_clockdiv = div; return *this; } // determines base sound pitch (default 2)

	void sample_trigger(int state);
	void sample_trigger_w(uint8_t data);
	void sample_rate_w(uint8_t data);
	void sample_volume_w(uint8_t data);

protected:
	// device level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void play_sample(int start, int freq, int volume);

private:
	std::unique_ptr<int16_t[]> m_sample_buf;    // buffer to decode samples at run time
	uint8_t m_sample_num;
	uint32_t m_sample_freq;
	uint8_t m_sample_volume;
	uint8_t m_sample_clockdiv;
	required_device<samples_device> m_samples;
	required_region_ptr<uint8_t> m_samples_region;

	void sample_select_w(uint8_t data);

	SAMPLES_START_CB_MEMBER( sh_start );
};


#endif // MAME_AUDIO_CCLIMBER_H
