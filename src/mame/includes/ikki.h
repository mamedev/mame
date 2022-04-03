// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Ikki

*************************************************************************/
#ifndef MAME_INCLUDES_IKKI_H
#define MAME_INCLUDES_IKKI_H

#pragma once

#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class ikki_state : public driver_device
{
public:
	ikki_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_videoram(*this, "videoram"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void ikki(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	bitmap_ind16 m_sprite_bitmap{};
	uint8_t      m_flipscreen = 0U;
	int        m_punch_through_pen = 0;
	uint8_t      m_irq_source = 0U;

	uint8_t ikki_e000_r();
	void ikki_coin_counters(uint8_t data);
	void ikki_scrn_ctrl_w(uint8_t data);
	void ikki_palette(palette_device &palette);
	uint32_t screen_update_ikki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ikki_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ikki_cpu1(address_map &map);
	void ikki_cpu2(address_map &map);
};

#endif // MAME_INCLUDES_IKKI_H
