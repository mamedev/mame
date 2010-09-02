class suna16_state : public driver_device
{
public:
	suna16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 prot;
	UINT16 *paletteram;
	UINT16 *spriteram;
	UINT16 *spriteram2;
	int color_bank;
};


/*----------- defined in video/suna16.c -----------*/

WRITE16_HANDLER( suna16_flipscreen_w );
WRITE16_HANDLER( bestbest_flipscreen_w );

READ16_HANDLER ( suna16_paletteram16_r );
WRITE16_HANDLER( suna16_paletteram16_w );

VIDEO_START( suna16 );
VIDEO_UPDATE( suna16 );
VIDEO_UPDATE( bestbest );
