// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi
#ifndef MAME_TAITO_TNZS_VIDEO_H
#define MAME_TAITO_TNZS_VIDEO_H

#pragma once

#include "video/x1_001.h"

#include "emupal.h"
#include "screen.h"


class tnzs_video_state_base : public driver_device
{
protected:
	tnzs_video_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_spritegen(*this, "spritegen")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{
	}

	void prompalette(palette_device &palette) const;
	uint32_t screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_tnzs(int state);

	required_device<cpu_device> m_maincpu;
	required_device<x1_001_device> m_spritegen;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

#endif // MAME_TAITO_TNZS_VIDEO_H
