// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * specimx_snd.h
 *
 ****************************************************************************/

#ifndef SPECIAL_SND_H_
#define SPECIAL_SND_H_

#include "emu.h"

class specimx_sound_device : public device_t,
								public device_sound_interface
{
public:
	specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~specimx_sound_device() { }

	DECLARE_WRITE_LINE_MEMBER(set_input_ch0);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch1);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch2);

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	sound_stream *m_mixer_channel;
	int m_specimx_input[3];
};

extern const device_type SPECIMX_SND;

#endif /* SPECIAL_SND_H_ */
