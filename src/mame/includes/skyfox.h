// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Skyfox

*************************************************************************/
#ifndef MAME_INCLUDES_SKYFOX_H
#define MAME_INCLUDES_SKYFOX_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "screen.h"

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram")
	{ }

	void skyfox(machine_config &config);

	void init_skyfox();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	/* devices/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bgram;

	int m_bg_ctrl = 0;

	void output_w(offs_t offset, uint8_t data);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void skyfox_palette(palette_device &palette) const;
	uint32_t screen_update_skyfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void skyfox_map(address_map &map);
	void skyfox_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SKYFOX_H
