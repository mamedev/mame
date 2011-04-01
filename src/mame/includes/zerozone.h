/*************************************************************************

    Zero Zone

*************************************************************************/

class zerozone_state : public driver_device
{
public:
	zerozone_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    m_videoram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      m_videoram_size;

	/* video-related */
	UINT16      m_tilebank;
	tilemap_t     *m_zz_tilemap;

	/* devices */
	device_t *m_audiocpu;
};

/*----------- defined in video/zerozone.c -----------*/

WRITE16_HANDLER( zerozone_tilemap_w );
WRITE16_HANDLER( zerozone_tilebank_w );

VIDEO_START( zerozone );
SCREEN_UPDATE( zerozone );
