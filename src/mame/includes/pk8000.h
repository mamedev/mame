// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_INCLUDES_PK8000_H
#define MAME_INCLUDES_PK8000_H

#pragma once

#include "emupal.h"

class pk8000_base_state : public driver_device
{
public:
	pk8000_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(_84_porta_r);
	DECLARE_WRITE8_MEMBER(_84_porta_w);
	DECLARE_WRITE8_MEMBER(_84_portc_w);

protected:
	DECLARE_READ8_MEMBER(video_color_r);
	DECLARE_WRITE8_MEMBER(video_color_w);
	DECLARE_READ8_MEMBER(text_start_r);
	DECLARE_WRITE8_MEMBER(text_start_w);
	DECLARE_READ8_MEMBER(chargen_start_r);
	DECLARE_WRITE8_MEMBER(chargen_start_w);
	DECLARE_READ8_MEMBER(video_start_r);
	DECLARE_WRITE8_MEMBER(video_start_w);
	DECLARE_READ8_MEMBER(color_start_r);
	DECLARE_WRITE8_MEMBER(color_start_w);
	DECLARE_READ8_MEMBER(color_r);
	DECLARE_WRITE8_MEMBER(color_w);

	void pk8000_palette(palette_device &palette) const;
	uint32_t video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *videomem);

	uint8_t m_text_start;
	uint8_t m_chargen_start;
	uint8_t m_video_start;
	uint8_t m_color_start;

	uint8_t m_video_mode;
	uint8_t m_video_color;
	uint8_t m_color[32];
	uint8_t m_video_enable;
	required_device<cpu_device> m_maincpu;
};

#endif // MAME_INCLUDES_PK8000_H
