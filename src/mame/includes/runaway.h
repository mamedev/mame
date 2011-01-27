class runaway_state : public driver_device
{
public:
	runaway_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	emu_timer *interrupt_timer;
	UINT8* video_ram;
	UINT8* sprite_ram;
	tilemap_t *bg_tilemap;
	int tile_bank;
};


/*----------- defined in video/runaway.c -----------*/

VIDEO_START( runaway );
VIDEO_START( qwak );
VIDEO_UPDATE( runaway );
VIDEO_UPDATE( qwak );

WRITE8_HANDLER( runaway_paletteram_w );
WRITE8_HANDLER( runaway_video_ram_w );
WRITE8_HANDLER( runaway_tile_bank_w );
