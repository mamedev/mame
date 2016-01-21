// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
#pragma once

#ifndef __DSBZ80_H__
#define __DSBZ80_H__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/mpeg_audio.h"

#define DSBZ80_TAG "dsbz80"

#define MCFG_DSBZ80_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DSBZ80, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsbz80_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dsbz80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE8_MEMBER(latch_w);

	required_device<cpu_device> m_ourcpu;

	DECLARE_WRITE8_MEMBER(mpeg_trigger_w);
	DECLARE_WRITE8_MEMBER(mpeg_start_w);
	DECLARE_WRITE8_MEMBER(mpeg_end_w);
	DECLARE_WRITE8_MEMBER(mpeg_volume_w);
	DECLARE_WRITE8_MEMBER(mpeg_stereo_w);
	DECLARE_READ8_MEMBER(mpeg_pos_r);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_READ8_MEMBER(status_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	mpeg_audio *decoder;
	INT16 audio_buf[1152*2];
	UINT8 m_dsb_latch;
	UINT32 mp_start, mp_end, mp_vol, mp_pan, mp_state, lp_start, lp_end, start, end;
	int mp_pos, audio_pos, audio_avail;
	int status;
};


// device type definition
extern const device_type DSBZ80;

#endif  /* __DSBZ80_H__ */
