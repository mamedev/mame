/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/

class galaxia_state : public driver_device
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_video;
	UINT8 *m_color;
	UINT8 *m_bullet;

	UINT8 *m_fo_state;

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_temp_bitmap;

	UINT8 m_collision;
};


/*----------- defined in video/galaxia.c -----------*/

PALETTE_INIT( galaxia );
VIDEO_START( galaxia );
SCREEN_UPDATE_IND16( galaxia );
SCREEN_UPDATE_IND16( astrowar );
