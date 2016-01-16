// license:BSD-3-Clause
// copyright-holders:David Haywood
class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_bg0_videoram(*this, "bg0_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_fg0_videoram;
	required_shared_ptr<UINT16> m_bg0_videoram;

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;

	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE16_MEMBER(irqack_w);
	DECLARE_WRITE16_MEMBER(fg0_videoram_w);
	DECLARE_WRITE16_MEMBER(bg0_videoram_w);

	DECLARE_CUSTOM_INPUT_MEMBER(vblank_r);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	TILE_GET_INFO_MEMBER(get_fg0_tile_info);
	TILEMAP_MAPPER_MEMBER(bg0_scan);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
