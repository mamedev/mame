// license:BSD-3-Clause
// copyright-holders:David Haywood
class pokechmp_state : public driver_device
{
public:
	pokechmp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(pokechmp_bank_w);
	DECLARE_WRITE8_MEMBER(pokechmp_sound_bank_w);
	DECLARE_WRITE8_MEMBER(pokechmp_sound_w);
	DECLARE_WRITE8_MEMBER(pokechmp_videoram_w);
	DECLARE_WRITE8_MEMBER(pokechmp_flipscreen_w);
	DECLARE_DRIVER_INIT(pokechmp);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_pokechmp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
