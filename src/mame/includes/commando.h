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
	DECLARE_WRITE8_MEMBER(commando_videoram_w);
	DECLARE_WRITE8_MEMBER(commando_colorram_w);
	DECLARE_WRITE8_MEMBER(commando_videoram2_w);
	DECLARE_WRITE8_MEMBER(commando_colorram2_w);
	DECLARE_WRITE8_MEMBER(commando_scrollx_w);
	DECLARE_WRITE8_MEMBER(commando_scrolly_w);
	DECLARE_WRITE8_MEMBER(commando_c804_w);
};



/*----------- defined in video/commando.c -----------*/


VIDEO_START( commando );
SCREEN_UPDATE_IND16( commando );
