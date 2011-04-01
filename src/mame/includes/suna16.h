class suna16_state : public driver_device
{
public:
	suna16_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 m_prot;
	UINT16 *m_paletteram;
	UINT16 *m_spriteram;
	UINT16 *m_spriteram2;
	int m_color_bank;
};


/*----------- defined in video/suna16.c -----------*/

WRITE16_HANDLER( suna16_flipscreen_w );
WRITE16_HANDLER( bestbest_flipscreen_w );

READ16_HANDLER ( suna16_paletteram16_r );
WRITE16_HANDLER( suna16_paletteram16_w );

VIDEO_START( suna16 );
SCREEN_UPDATE( suna16 );
SCREEN_UPDATE( bestbest );
