class galpanic_state : public driver_device
{
public:
	galpanic_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *bgvideoram;
	UINT16 *fgvideoram;
	size_t fgvideoram_size;
	bitmap_t *sprites_bitmap;
	UINT16 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/galpanic.c -----------*/

PALETTE_INIT( galpanic );
WRITE16_HANDLER( galpanic_bgvideoram_w );
WRITE16_HANDLER( galpanic_paletteram_w );
VIDEO_START( galpanic );
SCREEN_UPDATE( galpanic );
SCREEN_UPDATE( comad );


