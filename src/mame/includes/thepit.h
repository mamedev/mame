class thepit_state : public driver_device
{
public:
	thepit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int question_address;
	int question_rom;
	int remap_address[16];
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *attributesram;
	UINT8 *spriteram;
	size_t spriteram_size;
	UINT8 graphics_bank;
	UINT8 flip_screen_x;
	UINT8 flip_screen_y;
	tilemap_t *solid_tilemap;
	tilemap_t *tilemap;
	UINT8 *dummy_tile;
};


/*----------- defined in video/thepit.c -----------*/

PALETTE_INIT( thepit );
PALETTE_INIT( suprmous );
VIDEO_START( thepit );
SCREEN_UPDATE( thepit );
WRITE8_HANDLER( thepit_videoram_w );
WRITE8_HANDLER( thepit_colorram_w );
WRITE8_HANDLER( thepit_flip_screen_x_w );
WRITE8_HANDLER( thepit_flip_screen_y_w );
READ8_HANDLER( thepit_input_port_0_r );
WRITE8_HANDLER( intrepid_graphics_bank_w );
