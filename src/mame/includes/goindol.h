// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
/*************************************************************************

    Goindol

*************************************************************************/
#ifndef MAME_INCLUDES_GOINDOL_H
#define MAME_INCLUDES_GOINDOL_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class goindol_state : public driver_device
{
public:
	goindol_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_fg_scrolly(*this, "fg_scrolly"),
		m_fg_scrollx(*this, "fg_scrollx"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_fg_videoram(*this, "fg_videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void goindol(machine_config &config);
	void init_goindol();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_fg_scrolly;
	required_shared_ptr<uint8_t> m_fg_scrollx;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_fg_videoram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	uint16_t      m_char_bank;

	/* misc */
	int         m_prot_toggle;
	void goindol_bankswitch_w(uint8_t data);
	uint8_t prot_f422_r();
	void prot_fc44_w(uint8_t data);
	void prot_fd99_w(uint8_t data);
	void prot_fc66_w(uint8_t data);
	void prot_fcb0_w(uint8_t data);
	void goindol_fg_videoram_w(offs_t offset, uint8_t data);
	void goindol_bg_videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_goindol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, uint8_t *sprite_ram );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void goindol_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_GOINDOL_H
