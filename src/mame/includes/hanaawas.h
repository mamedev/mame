/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state : public driver_device
{
public:
	hanaawas_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* misc */
	int        mux;
};


/*----------- defined in video/hanaawas.c -----------*/

WRITE8_HANDLER( hanaawas_videoram_w );
WRITE8_HANDLER( hanaawas_colorram_w );
WRITE8_DEVICE_HANDLER( hanaawas_portB_w );

PALETTE_INIT( hanaawas );
VIDEO_START( hanaawas );
VIDEO_UPDATE( hanaawas );
