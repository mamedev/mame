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
	cclimber_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sample_trigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sample_rate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sample_volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sample_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	SAMPLES_START_CB_MEMBER( sh_start );

protected:
	// device level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	void play_sample(int start,int freq,int volume);

private:
	std::unique_ptr<int16_t[]> m_sample_buf;    /* buffer to decode samples at run time */
	int m_sample_num;
	int m_sample_freq;
	int m_sample_volume;
	optional_device<samples_device> m_samples;
	optional_region_ptr<uint8_t> m_samples_region;
};


#endif
