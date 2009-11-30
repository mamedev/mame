

typedef struct _fuuki16_state fuuki16_state;
struct _fuuki16_state
{
	/* memory pointers */
	UINT16 *    vram_0;
	UINT16 *    vram_1;
	UINT16 *    vram_2;
	UINT16 *    vram_3;
	UINT16 *    vregs;
	UINT16 *    priority;
	UINT16 *    unknown;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap     *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;

	/* misc */
	emu_timer   *raster_interrupt_timer;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
};


/*----------- defined in video/fuukifg2.c -----------*/

WRITE16_HANDLER( fuuki16_vram_0_w );
WRITE16_HANDLER( fuuki16_vram_1_w );
WRITE16_HANDLER( fuuki16_vram_2_w );
WRITE16_HANDLER( fuuki16_vram_3_w );

VIDEO_START( fuuki16 );
VIDEO_UPDATE( fuuki16 );
