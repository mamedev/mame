class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	UINT8 m_control;
	required_shared_ptr<UINT8> m_workram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_tmpbitmap;
	DECLARE_WRITE8_MEMBER(tunhunt_control_w);
	DECLARE_READ8_MEMBER(tunhunt_button_r);
	DECLARE_WRITE8_MEMBER(tunhunt_videoram_w);
	DECLARE_READ8_MEMBER(dsw2_0r);
	DECLARE_READ8_MEMBER(dsw2_1r);
	DECLARE_READ8_MEMBER(dsw2_2r);
	DECLARE_READ8_MEMBER(dsw2_3r);
	DECLARE_READ8_MEMBER(dsw2_4r);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(tunhunt);
	UINT32 screen_update_tunhunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pens();
	void draw_motion_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_shell(bitmap_ind16 &bitmap, const rectangle &cliprect, int picture_code,
		int hposition,int vstart,int vstop,int vstretch,int hstretch);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
