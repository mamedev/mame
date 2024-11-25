// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_KONAMI_TRACKFLD_A_H
#define MAME_KONAMI_TRACKFLD_A_H

#pragma once

#include "sound/vlm5030.h"
#include "cpu/m6800/m6800.h"

class trackfld_audio_device : public device_t
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

	void sh_irqtrigger_w(int state);
	uint8_t trackfld_sh_timer_r();
	uint8_t trackfld_speech_r();
	void trackfld_sound_w(offs_t offset, uint8_t data);
	uint8_t hyperspt_sh_timer_r();
	void hyperspt_sound_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	optional_device<cpu_device> m_audiocpu;
	optional_device<vlm5030_device> m_vlm;

	// internal state
	int      m_last_addr;
	int      m_last_irq;
};

DECLARE_DEVICE_TYPE(TRACKFLD_AUDIO, trackfld_audio_device)

#endif // MAME_KONAMI_TRACKFLD_A_H
