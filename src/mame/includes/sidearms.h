#include "video/bufsprite.h"

class sidearms_state : public driver_device
{
public:
	sidearms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") ,
		m_bg_scrollx(*this, "bg_scrollx"),
		m_bg_scrolly(*this, "bg_scrolly"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	int m_gameid;

	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<UINT8> m_bg_scrollx;
	required_shared_ptr<UINT8> m_bg_scrolly;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
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

	DECLARE_WRITE8_MEMBER(sidearms_bankswitch_w);
	DECLARE_READ8_MEMBER(turtship_ports_r);
	DECLARE_WRITE8_MEMBER(whizz_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sidearms_videoram_w);
	DECLARE_WRITE8_MEMBER(sidearms_colorram_w);
	DECLARE_WRITE8_MEMBER(sidearms_c804_w);
	DECLARE_WRITE8_MEMBER(sidearms_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(sidearms_star_scrollx_w);
	DECLARE_WRITE8_MEMBER(sidearms_star_scrolly_w);
	DECLARE_DRIVER_INIT(dyger);
	DECLARE_DRIVER_INIT(sidearms);
	DECLARE_DRIVER_INIT(whizz);
	DECLARE_DRIVER_INIT(turtship);
	TILE_GET_INFO_MEMBER(get_sidearms_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_philko_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(sidearms_tilemap_scan);
};

/*----------- defined in video/sidearms.c -----------*/


VIDEO_START( sidearms );
SCREEN_UPDATE_IND16( sidearms );
