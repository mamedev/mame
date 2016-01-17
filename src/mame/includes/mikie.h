// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/*************************************************************************

    Mikie

*************************************************************************/

class mikie_state : public driver_device
{
public:
	mikie_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_palettebank;

	/* misc */
	int        m_last_irq;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8      m_irq_mask;
	DECLARE_READ8_MEMBER(mikie_sh_timer_r);
	DECLARE_WRITE8_MEMBER(mikie_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(mikie_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(mikie_videoram_w);
	DECLARE_WRITE8_MEMBER(mikie_colorram_w);
	DECLARE_WRITE8_MEMBER(mikie_palettebank_w);
	DECLARE_WRITE8_MEMBER(mikie_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(mikie);
	UINT32 screen_update_mikie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
