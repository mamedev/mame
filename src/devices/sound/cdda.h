// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
#ifndef MAME_SOUND_CDDA_H
#define MAME_SOUND_CDDA_H

#pragma once

#include "cdrom.h"


class cdda_device : public device_t, public device_sound_interface
{
public:
	cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 44'100);

	void set_cdrom(void *file);

	void start_audio(uint32_t startlba, uint32_t numblocks);
	void stop_audio();
	void pause_audio(int pause);
	int16_t get_channel_volume(int channel);

	uint32_t get_audio_lba();
	int audio_active();
	int audio_paused();
	int audio_ended();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void get_audio_data(stream_sample_t *bufL, stream_sample_t *bufR, uint32_t samples_wanted);

	cdrom_file *        m_disc;

	// internal state
	sound_stream *      m_stream;

	int8_t                m_audio_playing, m_audio_pause, m_audio_ended_normally;
	uint32_t              m_audio_lba, m_audio_length;

	std::unique_ptr<uint8_t[]>   m_audio_cache;
	uint32_t              m_audio_samples;
	uint32_t              m_audio_bptr;
	int16_t               m_audio_volume[2];
};

DECLARE_DEVICE_TYPE(CDDA, cdda_device)

#endif // MAME_SOUND_CDDA_H
