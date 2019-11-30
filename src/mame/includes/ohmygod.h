// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Oh My God!

*************************************************************************/
#ifndef MAME_INCLUDES_OHMYGOD_H
#define MAME_INCLUDES_OHMYGOD_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class ohmygod_state : public driver_device
{
public:
	ohmygod_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void ohmygod(machine_config &config);

	void init_ohmygod();
	void init_naname();

private:
	DECLARE_WRITE16_MEMBER(ohmygod_ctrl_w);
	DECLARE_WRITE16_MEMBER(ohmygod_videoram_w);
	DECLARE_WRITE16_MEMBER(ohmygod_spritebank_w);
	DECLARE_WRITE16_MEMBER(ohmygod_scrollx_w);
	DECLARE_WRITE16_MEMBER(ohmygod_scrolly_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_ohmygod(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ohmygod_map(address_map &map);
	void oki_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int m_spritebank;
	uint16_t m_scrollx;
	uint16_t m_scrolly;

	/* misc */
	int m_adpcm_bank_shift;
	int m_sndbank;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_OHMYGOD_H
