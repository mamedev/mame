class psychic5_state : public driver_device
{
public:
	psychic5_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 bank_latch;
	UINT8 ps5_vram_page;
	UINT8 bg_clip_mode;
	UINT8 title_screen;
	UINT8 bg_status;
	UINT8 *ps5_pagedram[2];
	UINT8 *bg_videoram;
	UINT8 *ps5_dummy_bg_ram;
	UINT8 *ps5_io_ram;
	UINT8 *ps5_palette_ram;
	UINT8 *fg_videoram;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	int bg_palette_ram_base;
	int bg_palette_base;
	UINT16 palette_intensity;
	UINT8 bombsa_unknown;
	int sx1;
	int sy1;
	int sy2;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/psychic5.c -----------*/

WRITE8_HANDLER( psychic5_paged_ram_w );
WRITE8_HANDLER( psychic5_vram_page_select_w );
WRITE8_HANDLER( psychic5_title_screen_w );

READ8_HANDLER( psychic5_paged_ram_r );
READ8_HANDLER( psychic5_vram_page_select_r );

VIDEO_START( psychic5 );
VIDEO_RESET( psychic5 );
SCREEN_UPDATE( psychic5 );

WRITE8_HANDLER( bombsa_paged_ram_w );
WRITE8_HANDLER( bombsa_unknown_w );

VIDEO_START( bombsa );
VIDEO_RESET( bombsa );
SCREEN_UPDATE( bombsa );
