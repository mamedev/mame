class shangkid_state : public driver_device
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_videoreg(*this, "videoreg"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bbx(*this, "bbx"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;
	UINT8 m_bbx_sound_enable;
	UINT8 m_sound_latch;
	optional_shared_ptr<UINT8> m_videoreg;
	int m_gfx_type;
	tilemap_t *m_background;
	DECLARE_WRITE8_MEMBER(shangkid_maincpu_bank_w);
	DECLARE_WRITE8_MEMBER(shangkid_bbx_enable_w);
	DECLARE_WRITE8_MEMBER(shangkid_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(shangkid_sound_enable_w);
	DECLARE_READ8_MEMBER(shangkid_soundlatch_r);
	DECLARE_WRITE8_MEMBER(shangkid_videoram_w);
	DECLARE_WRITE8_MEMBER(chinhero_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(shangkid_ay8910_porta_w);
	DECLARE_WRITE8_MEMBER(ay8910_portb_w);
	DECLARE_DRIVER_INIT(shangkid);
	DECLARE_DRIVER_INIT(chinhero);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_RESET(chinhero);
	DECLARE_VIDEO_START(shangkid);
	DECLARE_PALETTE_INIT(dynamski);
	DECLARE_MACHINE_RESET(shangkid);
	UINT32 screen_update_shangkid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dynamski(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite(const UINT8 *source, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void shangkid_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dynamski_draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void dynamski_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_bbx;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
