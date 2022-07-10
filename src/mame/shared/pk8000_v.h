// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_INCLUDES_PK8000_V_H
#define MAME_INCLUDES_PK8000_V_H

#pragma once

#include "emupal.h"

class pk8000_base_state : public driver_device
{
public:
	pk8000_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

protected:
	uint8_t _84_porta_r();
	void _84_porta_w(uint8_t data);
	void _84_portc_w(uint8_t data);

	uint8_t video_color_r();
	void video_color_w(uint8_t data);
	uint8_t text_start_r();
	void text_start_w(uint8_t data);
	uint8_t chargen_start_r();
	void chargen_start_w(uint8_t data);
	uint8_t video_start_r();
	void video_start_w(uint8_t data);
	uint8_t color_start_r();
	void color_start_w(uint8_t data);
	uint8_t color_r(offs_t offset);
	void color_w(offs_t offset, uint8_t data);

	void pk8000_palette(palette_device &palette) const;
	uint32_t video_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *videomem);

	uint8_t m_text_start = 0;
	uint8_t m_chargen_start = 0;
	uint8_t m_video_start = 0;
	uint8_t m_color_start = 0;

	uint8_t m_video_mode = 0;
	uint8_t m_video_color = 0;
	uint8_t m_color[32];
	uint8_t m_video_enable = 0;
	required_device<cpu_device> m_maincpu;
};

#endif // MAME_INCLUDES_PK8000_V_H
