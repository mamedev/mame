class lvcards_state : public driver_device
{
public:
	lvcards_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 payout;
	UINT8 pulse;
	UINT8 result;
	UINT8 *videoram;
	UINT8 *colorram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/lvcards.c -----------*/

WRITE8_HANDLER( lvcards_videoram_w );
WRITE8_HANDLER( lvcards_colorram_w );

PALETTE_INIT( lvcards );
PALETTE_INIT( ponttehk );
VIDEO_START( lvcards );
SCREEN_UPDATE( lvcards );
