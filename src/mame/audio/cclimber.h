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

	DECLARE_WRITE_LINE_MEMBER(sample_trigger);
	DECLARE_WRITE8_MEMBER(sample_trigger_w);
	DECLARE_WRITE8_MEMBER(sample_rate_w);
	DECLARE_WRITE8_MEMBER(sample_volume_w);

protected:
	// device level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void play_sample(int start,int freq,int volume);

private:
	std::unique_ptr<int16_t[]> m_sample_buf;    /* buffer to decode samples at run time */
	int m_sample_num;
	int m_sample_freq;
	int m_sample_volume;
	optional_device<samples_device> m_samples;
	optional_region_ptr<uint8_t> m_samples_region;

	DECLARE_WRITE8_MEMBER( sample_select_w );

	SAMPLES_START_CB_MEMBER( sh_start );
};


#endif // MAME_AUDIO_CCLIMBER_H
