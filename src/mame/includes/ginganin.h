// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Ginga NinkyouDen

*************************************************************************/

#include "machine/gen_latch.h"

class ginganin_state : public driver_device
{
public:
	ginganin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_txtram(*this, "txtram"),
		m_spriteram(*this, "spriteram"),
		m_vregs(*this, "vregs"),
		m_fgram(*this, "fgram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_txtram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_fgram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_tx_tilemap;
	int         m_layers_ctrl;
	int         m_flipscreen;
#ifdef MAME_DEBUG
	int         m_posx;
	int         m_posy;
#endif

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE16_MEMBER(ginganin_fgram16_w);
	DECLARE_WRITE16_MEMBER(ginganin_txtram16_w);
	DECLARE_WRITE16_MEMBER(ginganin_vregs16_w);
	DECLARE_WRITE8_MEMBER(ptm_irq);
	DECLARE_DRIVER_INIT(ginganin);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_ginganin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
};
