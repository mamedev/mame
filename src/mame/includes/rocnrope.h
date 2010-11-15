class rocnrope_state : public driver_device
{
public:
	rocnrope_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
};

/*----------- defined in video/rocnrope.c -----------*/

WRITE8_HANDLER( rocnrope_videoram_w );
WRITE8_HANDLER( rocnrope_colorram_w );
WRITE8_HANDLER( rocnrope_flipscreen_w );

PALETTE_INIT( rocnrope );
VIDEO_START( rocnrope );
VIDEO_UPDATE( rocnrope );
