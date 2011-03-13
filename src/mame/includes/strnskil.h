class strnskil_state : public driver_device
{
public:
	strnskil_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *xscroll;
	UINT8 scrl_ctrl;
	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/strnskil.c -----------*/

WRITE8_HANDLER( strnskil_videoram_w );
WRITE8_HANDLER( strnskil_scrl_ctrl_w );

PALETTE_INIT( strnskil );
VIDEO_START( strnskil );
SCREEN_UPDATE( strnskil );
