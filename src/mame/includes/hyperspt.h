class hyperspt_state : public driver_device
{
public:
	hyperspt_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  scroll;
	UINT8 *  scroll2;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int		 sprites_gfx_banked;
};

/*----------- defined in video/hyperspt.c -----------*/

WRITE8_HANDLER( hyperspt_videoram_w );
WRITE8_HANDLER( hyperspt_colorram_w );
WRITE8_HANDLER( hyperspt_flipscreen_w );

PALETTE_INIT( hyperspt );
VIDEO_START( hyperspt );
VIDEO_UPDATE( hyperspt );
VIDEO_START( roadf );

