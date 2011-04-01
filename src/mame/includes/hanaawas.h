/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state : public driver_device
{
public:
	hanaawas_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_mux;
};


/*----------- defined in video/hanaawas.c -----------*/

WRITE8_HANDLER( hanaawas_videoram_w );
WRITE8_HANDLER( hanaawas_colorram_w );
WRITE8_DEVICE_HANDLER( hanaawas_portB_w );

PALETTE_INIT( hanaawas );
VIDEO_START( hanaawas );
SCREEN_UPDATE( hanaawas );
