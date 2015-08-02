// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#define PIXMAP_COLOR_BASE  (16 + 32)
#define BITMAPRAM_SIZE      0x6000


class dogfgt_state : public driver_device
{
public:
	dogfgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram"),
		m_subcpu(*this, "sub") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_sharedram;

	/* video-related */
	bitmap_ind16 m_pixbitmap;
	tilemap_t   *m_bg_tilemap;
	UINT8     *m_bitmapram;
	int       m_bm_plane;
	int       m_pixcolor;
	int       m_scroll[4];
	int       m_lastflip;
	int       m_lastpixcolor;

	/* sound-related */
	int       m_soundlatch;
	int       m_last_snd_ctrl;

	/* devices */
	required_device<cpu_device> m_subcpu;
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_WRITE8_MEMBER(subirqtrigger_w);
	DECLARE_WRITE8_MEMBER(sub_irqack_w);
	DECLARE_WRITE8_MEMBER(dogfgt_soundlatch_w);
	DECLARE_WRITE8_MEMBER(dogfgt_soundcontrol_w);
	DECLARE_WRITE8_MEMBER(dogfgt_plane_select_w);
	DECLARE_READ8_MEMBER(dogfgt_bitmapram_r);
	DECLARE_WRITE8_MEMBER(internal_bitmapram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_bitmapram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(dogfgt_scroll_w);
	DECLARE_WRITE8_MEMBER(dogfgt_1800_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(dogfgt);
	UINT32 screen_update_dogfgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
