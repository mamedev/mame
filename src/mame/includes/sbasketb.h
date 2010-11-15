class sbasketb_state : public driver_device
{
public:
	sbasketb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  scroll;
	UINT8 *  spriteram;
	UINT8 *  palettebank;
	UINT8 *  spriteram_select;

	/* video-related */
	tilemap_t  *bg_tilemap;
};

/*----------- defined in video/sbasketb.c -----------*/

WRITE8_HANDLER( sbasketb_videoram_w );
WRITE8_HANDLER( sbasketb_colorram_w );
WRITE8_HANDLER( sbasketb_flipscreen_w );

PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
VIDEO_UPDATE( sbasketb );
