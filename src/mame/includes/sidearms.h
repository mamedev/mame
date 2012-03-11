#include "video/bufsprite.h"

class sidearms_state : public driver_device
{
public:
	sidearms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	int m_gameid;

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_bg_scrollx;
	UINT8 *m_bg_scrolly;
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

	required_device<buffered_spriteram8_device> m_spriteram;
};

/*----------- defined in video/sidearms.c -----------*/

WRITE8_HANDLER( sidearms_videoram_w );
WRITE8_HANDLER( sidearms_colorram_w );
WRITE8_HANDLER( sidearms_star_scrollx_w );
WRITE8_HANDLER( sidearms_star_scrolly_w );
WRITE8_HANDLER( sidearms_c804_w );
WRITE8_HANDLER( sidearms_gfxctrl_w );

VIDEO_START( sidearms );
SCREEN_UPDATE_IND16( sidearms );
