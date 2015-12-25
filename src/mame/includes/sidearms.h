// license:???
// copyright-holders:Paul Leaman, Curt Coder
#include "video/bufsprite.h"

class sidearms_state : public driver_device
{
public:
	sidearms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram") ,
		m_bg_scrollx(*this, "bg_scrollx"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_ports(*this, ports) { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	required_shared_ptr<UINT8> m_bg_scrollx;
	required_shared_ptr<UINT8> m_bg_scrolly;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	DECLARE_IOPORT_ARRAY(ports);
	optional_ioport_array<5> m_ports;

	int m_gameid;

	UINT8 *m_tilerom;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	int m_bgon;
	int m_objon;
	int m_staron;
	int m_charon;
	int m_flipon;

	UINT32 m_hflop_74a_n;
	UINT32 m_hcount_191;
	UINT32 m_vcount_191;
	UINT32 m_latch_374;

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(c804_w);
	DECLARE_WRITE8_MEMBER(gfxctrl_w);
	DECLARE_WRITE8_MEMBER(star_scrollx_w);
	DECLARE_WRITE8_MEMBER(star_scrolly_w);

	DECLARE_READ8_MEMBER(turtship_ports_r);

	DECLARE_WRITE8_MEMBER(whizz_bankswitch_w);

	DECLARE_DRIVER_INIT(dyger);
	DECLARE_DRIVER_INIT(sidearms);
	DECLARE_DRIVER_INIT(whizz);
	DECLARE_DRIVER_INIT(turtship);
	virtual void machine_start() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_sidearms_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_philko_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset, int end_offset );
	void draw_starfield( bitmap_ind16 &bitmap );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
