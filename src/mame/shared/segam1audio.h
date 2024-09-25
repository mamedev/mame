// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SHARED_SEGAM1AUDIO_H
#define MAME_SHARED_SEGAM1AUDIO_H

#include "cpu/m68000/m68000.h"
#include "machine/i8251.h"
#include "sound/multipcm.h"
#include "sound/ymopn.h"

#pragma once

#define M1AUDIO_TAG "m1audio"
#define M1AUDIO_CPU_REGION "m1audio:sndcpu"
#define M1AUDIO_MPCM1_REGION "m1audio:pcm1"
#define M1AUDIO_MPCM2_REGION "m1audio:pcm2"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class segam1audio_device : public device_t
{
public:
	// construction/destruction
	segam1audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto rxd_handler() { return m_rxd_handler.bind(); }

	void m1_snd_mpcm_bnk1_w(uint16_t data);
	void m1_snd_mpcm_bnk2_w(uint16_t data);

	void write_txd(int state);

	void mpcm1_map(address_map &map) ATTR_COLD;
	void mpcm2_map(address_map &map) ATTR_COLD;
	void segam1audio_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<multipcm_device> m_multipcm_1;
	required_device<multipcm_device> m_multipcm_2;
	required_device<ym3438_device> m_ym;
	required_device<i8251_device> m_uart;

	required_memory_region m_multipcm1_region;
	required_memory_region m_multipcm2_region;

	required_memory_bank m_mpcmbank1;
	required_memory_bank m_mpcmbank2;

	devcb_write_line   m_rxd_handler;

	void output_txd(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(SEGAM1AUDIO, segam1audio_device)

#endif  // MAME_SHARED_SEGAM1AUDIO_H
