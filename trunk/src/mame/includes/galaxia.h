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
	bitmap_ind16 m_temp_bitmap;
	DECLARE_WRITE8_MEMBER(galaxia_video_w);
	DECLARE_WRITE8_MEMBER(galaxia_scroll_w);
	DECLARE_WRITE8_MEMBER(galaxia_ctrlport_w);
	DECLARE_WRITE8_MEMBER(galaxia_dataport_w);
	DECLARE_READ8_MEMBER(galaxia_collision_r);
	DECLARE_READ8_MEMBER(galaxia_collision_clear);
};


/*----------- defined in video/galaxia.c -----------*/

PALETTE_INIT( galaxia );
PALETTE_INIT( astrowar );
VIDEO_START( galaxia );
VIDEO_START( astrowar );

SCREEN_UPDATE_IND16( galaxia );
SCREEN_UPDATE_IND16( astrowar );
