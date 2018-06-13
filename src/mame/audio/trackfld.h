// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_AUDIO_TRACKFLD_H
#define MAME_AUDIO_TRACKFLD_H

#pragma once

#include "sound/vlm5030.h"
#include "cpu/m6800/m6800.h"

class trackfld_audio_device : public device_t, public device_sound_interface
{
public:
	template <typename T, typename U>
	trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&vlm_tag)
		: trackfld_audio_device(mconfig, tag, owner, clock)
	{
		m_audiocpu.set_tag(std::forward<T>(cpu_tag));
		m_vlm.set_tag(std::forward<U>(vlm_tag));
	}

	trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(trackfld_sh_timer_r);
	DECLARE_READ8_MEMBER(trackfld_speech_r);
	DECLARE_WRITE8_MEMBER(trackfld_sound_w);
	DECLARE_READ8_MEMBER(hyperspt_sh_timer_r);
	DECLARE_WRITE8_MEMBER(hyperspt_sound_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	optional_device<cpu_device> m_audiocpu;
	optional_device<vlm5030_device> m_vlm;

	// internal state
	int      m_last_addr;
	int      m_last_irq;
};

DECLARE_DEVICE_TYPE(TRACKFLD_AUDIO, trackfld_audio_device)

#endif // MAME_AUDIO_TRACKFLD_H
