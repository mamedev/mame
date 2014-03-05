class quizdna_state : public driver_device
{
public:
	quizdna_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 *m_bg_ram;
	UINT8 *m_fg_ram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_bg_xscroll[2];
	int m_flipscreen;
	int m_video_enable;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(quizdna_rombank_w);
	DECLARE_WRITE8_MEMBER(gekiretu_rombank_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_ram_w);
	DECLARE_WRITE8_MEMBER(quizdna_fg_ram_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_yscroll_w);
	DECLARE_WRITE8_MEMBER(quizdna_bg_xscroll_w);
	DECLARE_WRITE8_MEMBER(quizdna_screen_ctrl_w);
	DECLARE_WRITE8_MEMBER(paletteram_xBGR_RRRR_GGGG_BBBB_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	UINT32 screen_update_quizdna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
