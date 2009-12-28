/*************************************************************************

    Glass

*************************************************************************/

typedef struct _glass_state glass_state;
struct _glass_state
{
	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    vregs;
	UINT16 *    spriteram;
//      UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *pant[2];
	bitmap_t    *screen_bitmap;

	/* misc */
	int         current_bit, current_command, cause_interrupt;
	int         blitter_serial_buffer[5];
};


/*----------- defined in video/glass.c -----------*/

WRITE16_HANDLER( glass_vram_w );
WRITE16_HANDLER( glass_blitter_w );

VIDEO_START( glass );
VIDEO_UPDATE( glass );
