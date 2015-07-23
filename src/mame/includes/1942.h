// license:???
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

    1942

***************************************************************************/

class _1942_state : public driver_device
{
public:
	_1942_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_protopal(*this, "protopal"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_bg_videoram;
	optional_shared_ptr<UINT8> m_protopal;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_palette_bank;
	UINT8 m_scroll[2];
	void create_palette();
	DECLARE_PALETTE_INIT(1942);
	DECLARE_PALETTE_INIT(1942p);
	DECLARE_WRITE8_MEMBER(c1942p_palette_w);

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_WRITE8_MEMBER(c1942_bankswitch_w);
	DECLARE_WRITE8_MEMBER(c1942_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(c1942_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(c1942_palette_bank_w);
	DECLARE_WRITE8_MEMBER(c1942_scroll_w);
	DECLARE_WRITE8_MEMBER(c1942_c804_w);
	DECLARE_WRITE8_MEMBER(c1942p_f600_w);
	DECLARE_WRITE8_MEMBER(c1942p_soundlatch_w);
	DECLARE_DRIVER_INIT(1942);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	void video_start_c1942p();
	UINT32 screen_update_1942(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_1942p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(c1942_scanline);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_sprites_p( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
