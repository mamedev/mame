/*************************************************************************

    Commando

*************************************************************************/

#include "video/bufsprite.h"

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_videoram2;
	UINT8 *  m_colorram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	UINT8 m_scroll_x[2];
	UINT8 m_scroll_y[2];

	/* devices */
	device_t *m_audiocpu;
	required_device<buffered_spriteram8_device> m_spriteram;
};



/*----------- defined in video/commando.c -----------*/

WRITE8_HANDLER( commando_videoram_w );
WRITE8_HANDLER( commando_colorram_w );
WRITE8_HANDLER( commando_videoram2_w );
WRITE8_HANDLER( commando_colorram2_w );
WRITE8_HANDLER( commando_scrollx_w );
WRITE8_HANDLER( commando_scrolly_w );
WRITE8_HANDLER( commando_c804_w );

VIDEO_START( commando );
SCREEN_UPDATE_IND16( commando );
