// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> beep_device

class beep_device : public device_t,
					public device_sound_interface
{
public:
	beep_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~beep_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	void set_state(int on);
	void set_frequency(int frequency);
	void set_volume(int volume);

private:
	sound_stream *m_stream;   /* stream number */
	int m_enable;             /* enable beep */
	int m_frequency;          /* set frequency - this can be changed using the appropiate function */
	int m_incr;               /* initial wave state */
	INT16 m_signal;           /* current signal */
};

extern const device_type BEEP;


#endif /* __BEEP_H__ */
