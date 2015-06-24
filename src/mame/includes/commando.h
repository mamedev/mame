// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Commando

*************************************************************************/

#include "video/bufsprite.h"

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8 m_scroll_x[2];
	UINT8 m_scroll_y[2];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE8_MEMBER(commando_videoram_w);
	DECLARE_WRITE8_MEMBER(commando_colorram_w);
	DECLARE_WRITE8_MEMBER(commando_videoram2_w);
	DECLARE_WRITE8_MEMBER(commando_colorram2_w);
	DECLARE_WRITE8_MEMBER(commando_scrollx_w);
	DECLARE_WRITE8_MEMBER(commando_scrolly_w);
	DECLARE_WRITE8_MEMBER(commando_c804_w);
	DECLARE_DRIVER_INIT(spaceinv);
	DECLARE_DRIVER_INIT(commando);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_commando(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(commando_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;
};
