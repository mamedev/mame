
class paradise_state : public driver_device
{
public:
	paradise_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  vram_0;
	UINT8 *  vram_1;
	UINT8 *  vram_2;
	UINT8 *  videoram;
	UINT8 *  paletteram;
	UINT8 *  spriteram;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t *tilemap_0, *tilemap_1, *tilemap_2;
	bitmap_t *tmpbitmap;
	UINT8 palbank, priority;
	int sprite_inc;
};

/*----------- defined in video/paradise.c -----------*/

WRITE8_HANDLER( paradise_vram_0_w );
WRITE8_HANDLER( paradise_vram_1_w );
WRITE8_HANDLER( paradise_vram_2_w );

WRITE8_HANDLER( paradise_flipscreen_w );
WRITE8_HANDLER( tgtball_flipscreen_w );
WRITE8_HANDLER( paradise_palette_w );
WRITE8_HANDLER( paradise_pixmap_w );

WRITE8_HANDLER( paradise_priority_w );
WRITE8_HANDLER( paradise_palbank_w );

VIDEO_START( paradise );

VIDEO_UPDATE( paradise );
VIDEO_UPDATE( torus );
VIDEO_UPDATE( madball );
