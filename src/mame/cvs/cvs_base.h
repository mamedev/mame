// license:BSD-3-Clause
// copyright-holders: Mike Coates, Couriersud

/***************************************************************************

    Century CVS System

****************************************************************************/

#ifndef MAME_CVS_CVS_BASE_H
#define MAME_CVS_CVS_BASE_H

#pragma once

#include "cpu/s2650/s2650.h"
#include "machine/gen_latch.h"
#include "machine/s2636.h"
#include "sound/dac.h"
#include "sound/tms5110.h"

#include "emupal.h"
#include "screen.h"


class cvs_base_state : public driver_device
{
protected:
	static inline constexpr uint8_t CVS_MAX_STARS = 250;
	static inline constexpr int8_t CVS_S2636_Y_OFFSET = -5;
	static inline constexpr int8_t CVS_S2636_X_OFFSET = -26;

	struct cvs_star
	{
		int x = 0, y = 0, code = 0;
	};

	// memory pointers
	required_shared_ptr<uint8_t> m_bullet_ram;

	// video-related
	cvs_star m_stars[CVS_MAX_STARS];
	bitmap_ind16 m_collision_background;
	uint8_t m_collision_register = 0U;
	uint16_t m_total_stars = 0U;
	int32_t m_stars_scroll = 0U;

	// devices
	required_device<s2650_device> m_maincpu;
	optional_device_array<s2636_device, 3> m_s2636;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory
	memory_share_creator<uint8_t> m_video_ram;
	memory_share_creator<uint8_t> m_color_ram;

	memory_view m_ram_view;

	cvs_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bullet_ram(*this, "bullet_ram")
		, m_maincpu(*this, "maincpu")
		, m_s2636(*this, "s2636%u", 0U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_video_ram(*this, "video_ram", 0x400, ENDIANNESS_BIG)
		, m_color_ram(*this, "color_ram", 0x400, ENDIANNESS_BIG)
		, m_ram_view(*this, "video_color_ram_view")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void write_s2650_flag(int state);
	uint8_t collision_r();
	uint8_t collision_clear();
	void scroll_start();
	void init_stars() ATTR_COLD;
	void update_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, const pen_t star_pen, bool update_always);
};

#endif // MAME_CVS_CVS_BASE_H
