// license:BSD-3-Clause
// copyright-holders:Uki

/*************************************************************************

    Himeshikibu

*************************************************************************/
#ifndef MAME_INCLUDES_HIMESIKI_H
#define MAME_INCLUDES_HIMESIKI_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"

class himesiki_state : public driver_device
{
public:
	himesiki_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_ram(*this, "bg_ram"),
		m_spriteram_p103a(*this, "sprram_p103a"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void himesiki(machine_config &config);

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_bg_ram;
	required_shared_ptr<uint8_t> m_spriteram_p103a;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t    *m_bg_tilemap;
	int          m_scrollx[2];
	int          m_scrolly;

	int        m_flipscreen;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(himesiki_rombank_w);
	DECLARE_WRITE8_MEMBER(himesiki_sound_w);
	DECLARE_WRITE8_MEMBER(himesiki_bg_ram_w);
	DECLARE_WRITE8_MEMBER(himesiki_scrollx_w);
	DECLARE_WRITE8_MEMBER(himesiki_scrolly_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_himesiki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void himesiki_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void himesiki_iom0(address_map &map);
	void himesiki_iom1(address_map &map);
	void himesiki_prm0(address_map &map);
	void himesiki_prm1(address_map &map);
};

#endif // MAME_INCLUDES_HIMESIKI_H
