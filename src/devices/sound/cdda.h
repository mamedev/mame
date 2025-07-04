// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
#ifndef MAME_SOUND_CDDA_H
#define MAME_SOUND_CDDA_H

#pragma once

#include "imagedev/cdromimg.h"


class cdda_device : public device_t, public device_sound_interface
{
public:
	cdda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 44100);

	template<typename T> void set_cdrom_tag(T &&tag) { m_disc.set_tag(std::forward<T>(tag)); }

	void start_audio(uint32_t startlba, uint32_t numblocks);
	void stop_audio();
	void pause_audio(int pause);
	void scan_forward();
	void scan_reverse();
	void cancel_scan();
	int16_t get_channel_sample(int channel);

	void set_audio_lba(uint32_t lba);
	uint32_t get_audio_lba();
	void set_audio_length(uint32_t sectors);
	int audio_active();
	int audio_paused();
	int audio_ended();

	auto audio_end_cb() { return m_audio_end_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void get_audio_data(sound_stream &stream);

	required_device<cdrom_image_device> m_disc;

	// internal state
	sound_stream *      m_stream;

	int8_t                m_audio_playing, m_audio_pause, m_audio_ended_normally, m_audio_scan, m_audio_scan_direction;
	uint32_t              m_audio_lba, m_audio_length;

	std::unique_ptr<uint8_t[]>   m_audio_cache;
	uint32_t              m_audio_samples;
	uint32_t              m_audio_bptr;
	int16_t               m_audio_data[2];

	uint32_t              m_sequence_counter;

	devcb_write_line m_audio_end_cb;
};

DECLARE_DEVICE_TYPE(CDDA, cdda_device)

#endif // MAME_SOUND_CDDA_H
