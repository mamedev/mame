class xxmissio_state : public driver_device
{
public:
	xxmissio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 m_status;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_xscroll;
	UINT8 m_yscroll;
	UINT8 m_flipscreen;
	DECLARE_WRITE8_MEMBER(xxmissio_bank_sel_w);
	DECLARE_WRITE8_MEMBER(xxmissio_status_m_w);
	DECLARE_WRITE8_MEMBER(xxmissio_status_s_w);
	DECLARE_WRITE8_MEMBER(xxmissio_flipscreen_w);
	DECLARE_WRITE8_MEMBER(xxmissio_bgram_w);
	DECLARE_READ8_MEMBER(xxmissio_bgram_r);
	DECLARE_CUSTOM_INPUT_MEMBER(xxmissio_status_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_xxmissio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(xxmissio_interrupt_m);
	INTERRUPT_GEN_MEMBER(xxmissio_interrupt_s);
	DECLARE_WRITE8_MEMBER(xxmissio_scroll_x_w);
	DECLARE_WRITE8_MEMBER(xxmissio_scroll_y_w);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
