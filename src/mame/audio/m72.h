// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/
#ifndef MAME_AUDIO_M72_H
#define MAME_AUDIO_M72_H

#pragma once

#include "sound/dac.h"
#include "screen.h"

class m72_audio_device : public device_t
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~m72_audio_device() {}

	// configuration
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_dac_tag(T &&tag) { m_dac.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_samples_tag(T &&tag) { m_samples.set_tag(std::forward<T>(tag)); }

	DECLARE_READ8_MEMBER(sample_r);
	DECLARE_WRITE8_MEMBER(sample_w);

	auto sample_update() { return m_sample_update.bind(); }

	/* the port goes to different address bits depending on the game */
	void set_sample_start(int start);
	DECLARE_WRITE8_MEMBER(vigilant_sample_addr_w);
	DECLARE_WRITE8_MEMBER(shisen_sample_addr_w);
	DECLARE_WRITE8_MEMBER(rtype2_sample_addr_w);
	DECLARE_WRITE8_MEMBER(poundfor_sample_addr_w);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint32_t m_sample_addr;
	optional_device<screen_device> m_screen;
	optional_region_ptr<uint8_t> m_samples;
	optional_device<dac_byte_interface> m_dac;

	devcb_write_line m_sample_update;
	emu_timer *m_sample_timer;

	TIMER_CALLBACK_MEMBER(sample_timer);
};

DECLARE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device)

#define MCFG_IREM_M72_AUDIO_DAC(_tag) \
	downcast<m72_audio_device &>(*device).set_dac_tag(_tag);

#define MCFG_IREM_M72_AUDIO_SAMPLE(_tag) \
	downcast<m72_audio_device &>(*device).set_samples_tag(_tag);

#endif // MAME_AUDIO_M72_H
