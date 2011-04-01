/*************************************************************************

    Cheeky Mouse

*************************************************************************/


class cheekyms_state : public driver_device
{
public:
	cheekyms_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        m_videoram;
	UINT8 *        m_spriteram;
	UINT8 *        m_port_80;

	/* video-related */
	tilemap_t        *m_cm_tilemap;
	bitmap_t       *m_bitmap_buffer;

	/* devices */
	device_t *m_maincpu;
	device_t *m_dac;
};


/*----------- defined in video/cheekyms.c -----------*/

PALETTE_INIT( cheekyms );
VIDEO_START( cheekyms );
SCREEN_UPDATE( cheekyms );
WRITE8_HANDLER( cheekyms_port_40_w );
WRITE8_HANDLER( cheekyms_port_80_w );
