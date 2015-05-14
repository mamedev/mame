// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#pragma once

#ifndef __TMS3615_H__
#define __TMS3615_H__

#define TMS3615_TONES 13
#define TMS3615_FOOTAGE_8   0
#define TMS3615_FOOTAGE_16  1


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS3615_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TMS3615, _clock)
#define MCFG_TMS3615_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, TMS3615, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tms3615_device

class tms3615_device : public device_t,
						public device_sound_interface
{
public:
	tms3615_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tms3615_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	void enable_w(int enable);

private:
	sound_stream *m_channel;        /* returned by stream_create() */
	int m_samplerate;               /* output sample rate */
	int m_basefreq;                 /* chip's base frequency */
	int m_counter8[TMS3615_TONES];  /* tone frequency counter for 8' */
	int m_counter16[TMS3615_TONES]; /* tone frequency counter for 16'*/
	int m_output8;                  /* output signal bits for 8' */
	int m_output16;                 /* output signal bits for 16' */
	int m_enable;                   /* mask which tones to play */
};

extern ATTR_DEPRECATED const device_type TMS3615;


#endif /* __TMS3615_H__ */
