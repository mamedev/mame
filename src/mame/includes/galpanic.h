#include "includes/kaneko16.h"

class galpanic_state : public kaneko16_state
{
public:
	galpanic_state(running_machine &machine, const driver_device_config_base &config)
		: kaneko16_state(machine, config) { }

	UINT16 *m_bgvideoram;
	UINT16 *m_fgvideoram;
	size_t m_fgvideoram_size;
	bitmap_t *m_sprites_bitmap;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/galpanic.c -----------*/

PALETTE_INIT( galpanic );
WRITE16_HANDLER( galpanic_bgvideoram_w );
WRITE16_HANDLER( galpanic_paletteram_w );
VIDEO_START( galpanic );
SCREEN_UPDATE( galpanic );
SCREEN_UPDATE( comad );


