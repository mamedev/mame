class speedbal_state : public driver_device
{
public:
	speedbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spriteram(*this, "spriteram"),
		m_background_videoram(*this, "bg_videoram"),
		m_foreground_videoram(*this, "fg_videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_background_videoram;
	required_shared_ptr<UINT8> m_foreground_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	bool m_leds_start;
	UINT32 m_leds_shiftreg;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_DRIVER_INIT(speedbal);
	DECLARE_DRIVER_INIT(musicbal);

	DECLARE_WRITE8_MEMBER(speedbal_coincounter_w);
	DECLARE_WRITE8_MEMBER(speedbal_foreground_videoram_w);
	DECLARE_WRITE8_MEMBER(speedbal_background_videoram_w);

	DECLARE_WRITE8_MEMBER(speedbal_maincpu_50_w);
	DECLARE_WRITE8_MEMBER(leds_output_block);
	DECLARE_WRITE8_MEMBER(leds_start_block);
	DECLARE_WRITE8_MEMBER(leds_shift_bit);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	virtual void video_start();
	UINT32 screen_update_speedbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
