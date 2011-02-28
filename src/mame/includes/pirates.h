class pirates_state : public driver_device
{
public:
	pirates_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *tx_tileram;
	UINT16 *spriteram;
	UINT16 *fg_tileram;
	UINT16 *bg_tileram;
	UINT16 *scroll;
	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/pirates.c -----------*/

WRITE16_HANDLER( pirates_tx_tileram_w );
WRITE16_HANDLER( pirates_fg_tileram_w );
WRITE16_HANDLER( pirates_bg_tileram_w );

VIDEO_START( pirates );
SCREEN_UPDATE( pirates );
