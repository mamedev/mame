class funworld_state : public driver_device
{
public:
	funworld_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8* m_videoram;
	UINT8* m_colorram;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/funworld.c -----------*/

WRITE8_HANDLER( funworld_videoram_w );
WRITE8_HANDLER( funworld_colorram_w );
PALETTE_INIT( funworld );
VIDEO_START( funworld );
VIDEO_START( magicrd2 );
SCREEN_UPDATE( funworld );
