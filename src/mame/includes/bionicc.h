/***************************************************************************

    Bionic Commando

***************************************************************************/

#include "video/bufsprite.h"

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT16 *  m_bgvideoram;
	UINT16 *  m_fgvideoram;
	UINT16 *  m_txvideoram;
	UINT16 *  m_paletteram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	UINT16    m_scroll[4];

	UINT16    m_inp[3];
	UINT16    m_soundcommand;

	required_device<buffered_spriteram16_device> m_spriteram;
};


/*----------- defined in video/bionicc.c -----------*/

WRITE16_HANDLER( bionicc_fgvideoram_w );
WRITE16_HANDLER( bionicc_bgvideoram_w );
WRITE16_HANDLER( bionicc_txvideoram_w );
WRITE16_HANDLER( bionicc_paletteram_w );
WRITE16_HANDLER( bionicc_scroll_w );
WRITE16_HANDLER( bionicc_gfxctrl_w );

VIDEO_START( bionicc );
SCREEN_UPDATE_IND16( bionicc );
