// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Amiga audio hardware

***************************************************************************/

#pragma once

#ifndef __SOUND_AMIGA_H__
#define __SOUND_AMIGA_H__

#include "emu.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> amiga_sound_device

class amiga_sound_device : public device_t, public device_sound_interface
{
public:
	amiga_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~amiga_sound_device() {}

	void update();
	void data_w(int which, UINT16 data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	static const int CLOCK_DIVIDER = 16;

	struct audio_channel
	{
		emu_timer *irq_timer;
		UINT32 curlocation;
		UINT16 curlength;
		UINT16 curticks;
		UINT8 index;
		bool dma_enabled;
		bool manualmode;
		INT8 latched;
	};

	void dma_reload(audio_channel *chan);

	// internal state
	audio_channel m_channel[4];
	sound_stream *m_stream;

	TIMER_CALLBACK_MEMBER( signal_irq );
};

extern const device_type AMIGA;

#endif // __SOUND_AMIGA_H__
