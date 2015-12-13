// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    cclimber.h

    Functions to emulate the cclimber audio boards

***************************************************************************/

#pragma once

#ifndef __CCLIMBER_AUDIO__
#define __CCLIMBER_AUDIO__

#include "emu.h"
#include "sound/samples.h"
//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type CCLIMBER_AUDIO;

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CCLIMBER_AUDIO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CCLIMBER_AUDIO, 0)


// ======================> cclimber_audio_device

class cclimber_audio_device : public device_t
{
public:
	// construction/destruction
	cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( sample_trigger_w );
	DECLARE_WRITE8_MEMBER( sample_rate_w );
	DECLARE_WRITE8_MEMBER( sample_volume_w );
	DECLARE_WRITE8_MEMBER( sample_select_w );

	SAMPLES_START_CB_MEMBER( sh_start );

protected:
	// device level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void play_sample(int start,int freq,int volume);

private:
	INT16 *m_sample_buf;    /* buffer to decode samples at run time */
	int m_sample_num;
	int m_sample_freq;
	int m_sample_volume;
	required_device<samples_device> m_samples;
};


#endif
