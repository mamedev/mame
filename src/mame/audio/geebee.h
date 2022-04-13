// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
#ifndef MAME_AUDIO_GEEBEE_H
#define MAME_AUDIO_GEEBEE_H

#pragma once

class geebee_sound_device : public device_t, public device_sound_interface
{
public:
	geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum
	{
		TIMER_VOLUME_DECAY
	};

	void sound_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	// internal state
	std::unique_ptr<uint16_t[]> m_decay;
	sound_stream *m_channel;
	int m_sound_latch;
	int m_sound_signal;
	int m_volume;
	emu_timer *m_volume_timer;
	int m_noise;
	int m_vcount;
};

DECLARE_DEVICE_TYPE(GEEBEE_SOUND, geebee_sound_device)

#endif // MAME_AUDIO_GEEBEE_H
