class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vb_attribram(*this, "vb_attribram"),
		m_vb_videoram(*this, "vb_videoram"),
		m_vb_scrolly_lo(*this, "vb_scrolly_lo"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"){ }

	required_shared_ptr<UINT8> m_vb_attribram;
	required_shared_ptr<UINT8> m_vb_videoram;
	required_shared_ptr<UINT8> m_vb_scrolly_lo;
	int m_vb_scrollx_hi;
	int m_vb_scrolly_hi;
	int m_vb_scrollx_lo;
	int m_gfxset;
	int m_vb_scrollx[256];
	int m_vb_bgprombank;
	int m_vb_spprombank;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(vball_irq_ack_w);
	DECLARE_WRITE8_MEMBER(vb_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpu_sound_command_w);
	DECLARE_WRITE8_MEMBER(vb_scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(vb_scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(vb_videoram_w);
	DECLARE_READ8_MEMBER(vb_attrib_r);
	DECLARE_WRITE8_MEMBER(vb_attrib_w);
	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	UINT32 screen_update_vb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(vball_scanline);
	void vb_bgprombank_w( int bank );
	void vb_spprombank_w( int bank );
	void vb_mark_all_dirty(  );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scanline_to_vcount(int scanline);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
