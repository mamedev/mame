#include "includes/kaneko16.h"

class galpanic_state : public kaneko16_state
{
public:
	galpanic_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag) { }

	UINT16 *m_bgvideoram;
	UINT16 *m_fgvideoram;
	size_t m_fgvideoram_size;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_sprites_bitmap;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE16_MEMBER(galpanic_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galpanica_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galpanica_misc_w);
	DECLARE_WRITE16_MEMBER(galpanic_coin_w);
	DECLARE_WRITE16_MEMBER(galpanic_bgvideoram_mirror_w);
	DECLARE_READ16_MEMBER(comad_timer_r);
	DECLARE_READ16_MEMBER(zipzap_random_read);
};


/*----------- defined in video/galpanic.c -----------*/

PALETTE_INIT( galpanic );
WRITE16_HANDLER( galpanic_bgvideoram_w );
WRITE16_HANDLER( galpanic_paletteram_w );
VIDEO_START( galpanic );
SCREEN_UPDATE_IND16( galpanic );
SCREEN_UPDATE_IND16( comad );


