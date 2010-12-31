class marineb_state : public driver_device
{
public:
	marineb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *   videoram;
	UINT8 *   colorram;
	UINT8 *   spriteram;

	/* video-related */
	tilemap_t   *bg_tilemap, *fg_tilemap;
	UINT8     palette_bank;
	UINT8     column_scroll;
	UINT8     flipscreen_x, flipscreen_y;
	UINT8     marineb_active_low_flipscreen;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};


/*----------- defined in video/marineb.c -----------*/

WRITE8_HANDLER( marineb_videoram_w );
WRITE8_HANDLER( marineb_colorram_w );
WRITE8_HANDLER( marineb_column_scroll_w );
WRITE8_HANDLER( marineb_palette_bank_0_w );
WRITE8_HANDLER( marineb_palette_bank_1_w );
WRITE8_HANDLER( marineb_flipscreen_x_w );
WRITE8_HANDLER( marineb_flipscreen_y_w );

PALETTE_INIT( marineb );
VIDEO_START( marineb );
VIDEO_UPDATE( marineb );
VIDEO_UPDATE( changes );
VIDEO_UPDATE( springer );
VIDEO_UPDATE( hoccer );
VIDEO_UPDATE( hopprobo );
