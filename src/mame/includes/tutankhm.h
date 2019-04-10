// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#ifndef MAME_INCLUDES_TUTANKHM_H
#define MAME_INCLUDES_TUTANKHM_H

#pragma once

#include "audio/timeplt.h"
#include "emupal.h"

class tutankhm_state : public driver_device
{
public:
	tutankhm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_timeplt_audio(*this, "timeplt_audio")
	{
	}

	void tutankhm(machine_config &config);

protected:
	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(tutankhm_bankselect_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(sound_on_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_tutankhm(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void main_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_scroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	uint8_t     m_flip_x;
	uint8_t     m_flip_y;

	/* misc */
	uint8_t    m_irq_toggle;
	uint8_t    m_irq_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_device<timeplt_audio_device> m_timeplt_audio;
};

#endif // MAME_INCLUDES_TUTANKHM_H
