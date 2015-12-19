// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Macross Plus

*************************************************************************/

class macrossp_state : public driver_device
{
public:
	macrossp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scra_videoram(*this, "scra_videoram"),
		m_scra_linezoom(*this, "scra_linezoom"),
		m_scra_videoregs(*this, "scra_videoregs"),

		m_scrb_videoram(*this, "scrb_videoram"),
		m_scrb_linezoom(*this, "scrb_linezoom"),
		m_scrb_videoregs(*this, "scrb_videoregs"),

		m_scrc_videoram(*this, "scrc_videoram"),
		m_scrc_linezoom(*this, "scrc_linezoom"),
		m_scrc_videoregs(*this, "scrc_videoregs"),

		m_text_videoram(*this, "text_videoram"),
		m_text_linezoom(*this, "text_linezoom"),
		m_text_videoregs(*this, "text_videoregs"),

		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	/* memory pointers */
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_scra_videoram;
	required_shared_ptr<UINT32> m_scra_linezoom;
	required_shared_ptr<UINT32> m_scra_videoregs;
	required_shared_ptr<UINT32> m_scrb_videoram;
	required_shared_ptr<UINT32> m_scrb_linezoom;
	required_shared_ptr<UINT32> m_scrb_videoregs;
	required_shared_ptr<UINT32> m_scrc_videoram;
	required_shared_ptr<UINT32> m_scrc_linezoom;
	required_shared_ptr<UINT32> m_scrc_videoregs;
	required_shared_ptr<UINT32> m_text_videoram;
	required_shared_ptr<UINT32> m_text_linezoom;
	required_shared_ptr<UINT32> m_text_videoregs;
	required_shared_ptr<UINT32> m_mainram;
	std::unique_ptr<UINT32[]>         m_spriteram_old;
	std::unique_ptr<UINT32[]>         m_spriteram_old2;

	/* video-related */
	tilemap_t  *m_scra_tilemap;
	tilemap_t  *m_scrb_tilemap;
	tilemap_t  *m_scrc_tilemap;
	tilemap_t  *m_text_tilemap;

	/* misc */
	int              m_sndpending;
	int              m_snd_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ32_MEMBER(macrossp_soundstatus_r);
	DECLARE_WRITE32_MEMBER(macrossp_soundcmd_w);
	DECLARE_READ16_MEMBER(macrossp_soundcmd_r);
	DECLARE_WRITE16_MEMBER(palette_fade_w);
	DECLARE_WRITE32_MEMBER(macrossp_speedup_w);
	DECLARE_WRITE32_MEMBER(quizmoon_speedup_w);
	DECLARE_WRITE32_MEMBER(macrossp_scra_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_scrb_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_scrc_videoram_w);
	DECLARE_WRITE32_MEMBER(macrossp_text_videoram_w);
	DECLARE_DRIVER_INIT(quizmoon);
	DECLARE_DRIVER_INIT(macrossp);
	TILE_GET_INFO_MEMBER(get_macrossp_scra_tile_info);
	TILE_GET_INFO_MEMBER(get_macrossp_scrb_tile_info);
	TILE_GET_INFO_MEMBER(get_macrossp_scrc_tile_info);
	TILE_GET_INFO_MEMBER(get_macrossp_text_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_macrossp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_macrossp(screen_device &screen, bool state);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, int linem, int pri);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
