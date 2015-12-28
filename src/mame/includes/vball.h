// license:BSD-3-Clause
// copyright-holders:Paul Hampson
class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_attribram(*this, "attribram"),
		m_videoram(*this, "videoram"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_attribram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrolly_lo;
	required_shared_ptr<UINT8> m_spriteram;

	int m_scrollx_hi;
	int m_scrolly_hi;
	int m_scrollx_lo;
	int m_gfxset;
	int m_scrollx[256];
	int m_bgprombank;
	int m_spprombank;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpu_sound_command_w);
	DECLARE_WRITE8_MEMBER(scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(attrib_w);

	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(vball_scanline);
	void bgprombank_w(int bank);
	void spprombank_w(int bank);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scanline_to_vcount(int scanline);
};
