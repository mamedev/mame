/***************************************************************************

    Time Pilot

***************************************************************************/

class timeplt_state : public driver_device
{
public:
	timeplt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	UINT8    nmi_enable;

	/* devices */
	cpu_device *maincpu;
};


/*----------- defined in video/timeplt.c -----------*/

READ8_HANDLER( timeplt_scanline_r );
WRITE8_HANDLER( timeplt_videoram_w );
WRITE8_HANDLER( timeplt_colorram_w );
WRITE8_HANDLER( timeplt_flipscreen_w );

PALETTE_INIT( timeplt );
VIDEO_START( timeplt );
VIDEO_START( chkun );
VIDEO_UPDATE( timeplt );
