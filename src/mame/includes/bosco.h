// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class bosco_state : public galaga_state
{
public:
	bosco_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
			m_bosco_radarattr(*this, "bosco_radarattr"),
			m_bosco_starcontrol(*this, "starcontrol"),
			m_bosco_starblink(*this, "bosco_starblink") { }

	required_shared_ptr<UINT8> m_bosco_radarattr;

	required_shared_ptr<UINT8> m_bosco_starcontrol;
	required_shared_ptr<UINT8> m_bosco_starblink;

	UINT8 *m_bosco_radarx;
	UINT8 *m_bosco_radary;

	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	UINT32 m_spriteram_size;
	DECLARE_WRITE8_MEMBER(bosco_flip_screen_w);
	TILEMAP_MAPPER_MEMBER(fg_tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	DECLARE_VIDEO_START(bosco);
	DECLARE_PALETTE_INIT(bosco);
	UINT32 screen_update_bosco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_bosco(screen_device &screen, bool state);

	inline void get_tile_info_bosco(tile_data &tileinfo,int tile_index,int ram_offs);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect, int flip);
	DECLARE_WRITE8_MEMBER( bosco_videoram_w );
	DECLARE_WRITE8_MEMBER( bosco_scrollx_w );
	DECLARE_WRITE8_MEMBER( bosco_scrolly_w );
	DECLARE_WRITE8_MEMBER( bosco_starclr_w );
};
