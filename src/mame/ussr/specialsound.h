// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * ussr/specialsound.h
 *
 ****************************************************************************/

#ifndef MAME_USSR_SPECIALSOUND_H
#define MAME_USSR_SPECIALSOUND_H

#pragma once


class specimx_sound_device : public device_t, public device_sound_interface
{
public:
	specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_input_ch0(int state);
	void set_input_ch1(int state);
	void set_input_ch2(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_mixer_channel;
	int m_specimx_input[3];
};

DECLARE_DEVICE_TYPE(SPECIMX_SND, specimx_sound_device)

#endif // MAME_USSR_SPECIALSOUND_H
