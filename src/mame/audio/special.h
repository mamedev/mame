// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * audio/specimx.h
 *
 ****************************************************************************/

#ifndef MAME_AUDIO_SPECIAL_H
#define MAME_AUDIO_SPECIAL_H

#pragma once


class specimx_sound_device : public device_t, public device_sound_interface
{
public:
	specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(set_input_ch0);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch1);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch2);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream *m_mixer_channel;
	int m_specimx_input[3];
};

DECLARE_DEVICE_TYPE(SPECIMX_SND, specimx_sound_device)

#endif // MAME_AUDIO_SPECIAL_H
