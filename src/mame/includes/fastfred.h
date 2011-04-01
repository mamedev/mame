/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

class fastfred_state : public driver_device
{
public:
	fastfred_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_imago_sprites[0x800*3];
	UINT16 m_imago_sprites_address;
	UINT8 m_imago_sprites_bank;
	int m_hardware_type;
	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 *m_attributesram;
	UINT8 *m_background_color;
	UINT8 *m_imago_fg_videoram;
	UINT16 m_charbank;
	UINT8 m_colorbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_web_tilemap;
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
