// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Konami Finalizer

***************************************************************************/

class finalizr_state : public driver_device
{
public:
	finalizr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_colorram2(*this, "colorram2"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_spriteram_2(*this, "spriteram_2")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_spriteram_2;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_spriterambank;
	int m_charbank;

	/* misc */
	int m_T1_line;
	UINT8 m_nmi_enable;
	UINT8 m_irq_enable;

	DECLARE_WRITE8_MEMBER(finalizr_coin_w);
	DECLARE_WRITE8_MEMBER(finalizr_flipscreen_w);
	DECLARE_WRITE8_MEMBER(finalizr_i8039_irq_w);
	DECLARE_WRITE8_MEMBER(i8039_irqen_w);
	DECLARE_READ8_MEMBER(i8039_T1_r);
	DECLARE_WRITE8_MEMBER(i8039_T0_w);
	DECLARE_WRITE8_MEMBER(finalizr_videoctrl_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(finalizr);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_finalizr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(finalizr_scanline);
};
