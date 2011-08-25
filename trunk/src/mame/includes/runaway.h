class runaway_state : public driver_device
{
public:
	runaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	emu_timer *m_interrupt_timer;
	UINT8* m_video_ram;
	UINT8* m_sprite_ram;
	tilemap_t *m_bg_tilemap;
	int m_tile_bank;
};


/*----------- defined in video/runaway.c -----------*/

VIDEO_START( runaway );
VIDEO_START( qwak );
SCREEN_UPDATE( runaway );
SCREEN_UPDATE( qwak );

WRITE8_HANDLER( runaway_paletteram_w );
WRITE8_HANDLER( runaway_video_ram_w );
WRITE8_HANDLER( runaway_tile_bank_w );
