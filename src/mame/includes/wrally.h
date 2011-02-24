class wrally_state : public driver_device
{
public:
	wrally_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *shareram;
	tilemap_t *pant[2];
	UINT16 *vregs;
	UINT16 *videoram;
	UINT16 *spriteram;
};


/*----------- defined in machine/wrally.c -----------*/

WRITE16_HANDLER( wrally_vram_w );
WRITE16_HANDLER( wrally_flipscreen_w );
WRITE16_HANDLER( OKIM6295_bankswitch_w );
WRITE16_HANDLER( wrally_coin_counter_w );
WRITE16_HANDLER( wrally_coin_lockout_w );

/*----------- defined in video/wrally.c -----------*/

VIDEO_START( wrally );
SCREEN_UPDATE( wrally );

