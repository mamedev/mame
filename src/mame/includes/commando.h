// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Commando

*************************************************************************/
#ifndef MAME_INCLUDES_COMMANDO_H
#define MAME_INCLUDES_COMMANDO_H

#pragma once

#include "video/bufsprite.h"
#include "emupal.h"

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_spaceinv();
	void init_commando();

	void commando(machine_config &config);

protected:
	DECLARE_WRITE8_MEMBER(commando_videoram_w);
	DECLARE_WRITE8_MEMBER(commando_colorram_w);
	DECLARE_WRITE8_MEMBER(commando_videoram2_w);
	DECLARE_WRITE8_MEMBER(commando_colorram2_w);
	DECLARE_WRITE8_MEMBER(commando_scrollx_w);
	DECLARE_WRITE8_MEMBER(commando_scrolly_w);
	DECLARE_WRITE8_MEMBER(commando_c804_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_commando(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void commando_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void sound_map(address_map &map);

private:
	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	uint8_t m_scroll_x[2];
	uint8_t m_scroll_y[2];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
};

#endif // MAME_INCLUDES_COMMANDO_H
