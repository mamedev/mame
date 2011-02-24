/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

class fastfred_state : public driver_device
{
public:
	fastfred_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 imago_sprites[0x800*3];
	UINT16 imago_sprites_address;
	UINT8 imago_sprites_bank;
	int hardware_type;
	UINT8 *videoram;
	UINT8 *spriteram;
	size_t spriteram_size;
	UINT8 *attributesram;
	UINT8 *background_color;
	UINT8 *imago_fg_videoram;
	UINT16 charbank;
	UINT8 colorbank;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *web_tilemap;
};


/*----------- defined in video/fastfred.c -----------*/

PALETTE_INIT( fastfred );
VIDEO_START( fastfred );
WRITE8_HANDLER( fastfred_videoram_w );
WRITE8_HANDLER( fastfred_attributes_w );
WRITE8_HANDLER( fastfred_charbank1_w );
WRITE8_HANDLER( fastfred_charbank2_w );
WRITE8_HANDLER( fastfred_colorbank1_w );
WRITE8_HANDLER( fastfred_colorbank2_w );
WRITE8_HANDLER( fastfred_flip_screen_x_w );
WRITE8_HANDLER( fastfred_flip_screen_y_w );
SCREEN_UPDATE( fastfred );

VIDEO_START( imago );
SCREEN_UPDATE( imago );
WRITE8_HANDLER( imago_fg_videoram_w );
WRITE8_HANDLER( imago_charbank_w );
