// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
#ifndef MAME_AUDIO_DSBZ80_H
#define MAME_AUDIO_DSBZ80_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "sound/mpeg_audio.h"

#define DSBZ80_TAG "dsbz80"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dsbz80_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dsbz80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto rxd_handler() { return m_rxd_handler.bind(); }

	required_device<cpu_device> m_ourcpu;
	required_device<i8251_device> m_uart;

	DECLARE_WRITE_LINE_MEMBER(write_txd);

	DECLARE_WRITE8_MEMBER(mpeg_trigger_w);
	DECLARE_WRITE8_MEMBER(mpeg_start_w);
	DECLARE_WRITE8_MEMBER(mpeg_end_w);
	DECLARE_WRITE8_MEMBER(mpeg_volume_w);
	DECLARE_WRITE8_MEMBER(mpeg_stereo_w);
	DECLARE_READ8_MEMBER(mpeg_pos_r);

	void dsbz80_map(address_map &map);
	void dsbz80io_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	mpeg_audio *decoder;
	int16_t audio_buf[1152*2];
	uint32_t mp_start, mp_end, mp_vol, mp_pan, mp_state, lp_start, lp_end, start, end;
	int mp_pos, audio_pos, audio_avail;

	devcb_write_line   m_rxd_handler;

	DECLARE_WRITE_LINE_MEMBER(output_txd);
};


// device type definition
DECLARE_DEVICE_TYPE(DSBZ80, dsbz80_device)

#endif // MAME_AUDIO_DSBZ80_H
