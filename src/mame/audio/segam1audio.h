// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_AUDIO_SEGAM1AUDIO_H
#define MAME_AUDIO_SEGAM1AUDIO_H

#include "cpu/m68000/m68000.h"
#include "machine/i8251.h"
#include "sound/2612intf.h"
#include "sound/multipcm.h"

#pragma once

#define M1AUDIO_CPU_REGION "m1sndcpu"
#define M1AUDIO_MPCM1_REGION "m1pcm1"
#define M1AUDIO_MPCM2_REGION "m1pcm2"

#define MCFG_SEGAM1AUDIO_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEGAM1AUDIO, 0)

#define MCFG_SEGAM1AUDIO_RXD_HANDLER(_devcb) \
	devcb = &segam1audio_device::set_rxd_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class segam1audio_device : public device_t
{
public:
	// construction/destruction
	segam1audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// static configuration
	template <class Object> static devcb_base &set_rxd_handler(device_t &device, Object &&cb) { return downcast<segam1audio_device &>(device).m_rxd_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE16_MEMBER(m1_snd_mpcm_bnk1_w);
	DECLARE_WRITE16_MEMBER(m1_snd_mpcm_bnk2_w);

	DECLARE_WRITE_LINE_MEMBER(write_txd);
	DECLARE_WRITE_LINE_MEMBER(output_txd);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<multipcm_device> m_multipcm_1;
	required_device<multipcm_device> m_multipcm_2;
	required_device<ym3438_device> m_ym;
	required_device<i8251_device> m_uart;

	devcb_write_line   m_rxd_handler;
};


// device type definition
DECLARE_DEVICE_TYPE(SEGAM1AUDIO, segam1audio_device)

#endif  // MAME_AUDIO_SEGAM1AUDIO_H
