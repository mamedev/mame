// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
#pragma once

#ifndef __CDDA_H__
#define __CDDA_H__

#include "cdrom.h"

class cdda_device : public device_t,
									public device_sound_interface
{
public:
	cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void set_cdrom(void *file);

	void start_audio(UINT32 startlba, UINT32 numblocks);
	void stop_audio();
	void pause_audio(int pause);
	void set_volume(int volume);
	void set_channel_volume(int channel, int volume);
	INT16 get_channel_volume(int channel);

	UINT32 get_audio_lba();
	int audio_active();
	int audio_paused();
	int audio_ended();

	cdrom_file *        m_disc;

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	void get_audio_data(stream_sample_t *bufL, stream_sample_t *bufR, UINT32 samples_wanted);

	// internal state
	sound_stream *      m_stream;

	INT8                m_audio_playing, m_audio_pause, m_audio_ended_normally;
	UINT32              m_audio_lba, m_audio_length;

	std::unique_ptr<UINT8[]>   m_audio_cache;
	UINT32              m_audio_samples;
	UINT32              m_audio_bptr;
	INT16               m_audio_volume[2];
};

extern const device_type CDDA;


#endif /* __CDDA_H__ */
