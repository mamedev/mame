

class funkybee_state : public driver_device
{
public:
	funkybee_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        gfx_bank;
};


/*----------- defined in video/funkybee.c -----------*/

WRITE8_HANDLER( funkybee_videoram_w );
WRITE8_HANDLER( funkybee_colorram_w );
WRITE8_HANDLER( funkybee_gfx_bank_w );
WRITE8_HANDLER( funkybee_scroll_w );
WRITE8_HANDLER( funkybee_flipscreen_w );

PALETTE_INIT( funkybee );
VIDEO_START( funkybee );
VIDEO_UPDATE( funkybee );
