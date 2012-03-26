/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/

#include "includes/cvs.h"

class galaxia_state : public cvs_state
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_stars_bitmap;
	bitmap_ind16 m_temp_bitmap;
};


/*----------- defined in video/galaxia.c -----------*/

PALETTE_INIT( galaxia );
PALETTE_INIT( astrowar );
VIDEO_START( galaxia );
VIDEO_START( astrowar );

SCREEN_UPDATE_IND16( galaxia );
SCREEN_UPDATE_IND16( astrowar );
