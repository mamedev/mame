/*************************************************************************

    Atari Cloak & Dagger hardware

*************************************************************************/

class cloak_state : public driver_device
{
public:
	cloak_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *spriteram;
	int nvram_enabled;
	UINT8 bitmap_videoram_selected;
	UINT8 bitmap_videoram_address_x;
	UINT8 bitmap_videoram_address_y;
	UINT8 *bitmap_videoram1;
	UINT8 *bitmap_videoram2;
	UINT8 *current_bitmap_videoram_accessed;
	UINT8 *current_bitmap_videoram_displayed;
	UINT16 *palette_ram;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/cloak.c -----------*/

WRITE8_HANDLER( cloak_videoram_w );
WRITE8_HANDLER( cloak_flipscreen_w );

WRITE8_HANDLER( cloak_paletteram_w );
READ8_HANDLER( graph_processor_r );
WRITE8_HANDLER( graph_processor_w );
WRITE8_HANDLER( cloak_clearbmp_w );

VIDEO_START( cloak );
VIDEO_UPDATE( cloak );
