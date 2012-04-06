/*************************************************************************

    Ghosts'n Goblins

*************************************************************************/

#include "video/bufsprite.h"

class gng_state : public driver_device
{
public:
	gng_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT8 *    m_bgvideoram;
	UINT8 *    m_fgvideoram;
//  UINT8 *    m_paletteram;  // currently this uses generic palette handling
//  UINT8 *    m_paletteram2; // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	UINT8      m_scrollx[2];
	UINT8      m_scrolly[2];

	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_WRITE8_MEMBER(gng_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gng_coin_counter_w);
	DECLARE_READ8_MEMBER(diamond_hack_r);
	DECLARE_WRITE8_MEMBER(gng_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(gng_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(gng_bgscrollx_w);
	DECLARE_WRITE8_MEMBER(gng_bgscrolly_w);
	DECLARE_WRITE8_MEMBER(gng_flipscreen_w);
};


/*----------- defined in video/gng.c -----------*/


VIDEO_START( gng );
SCREEN_UPDATE_IND16( gng );
