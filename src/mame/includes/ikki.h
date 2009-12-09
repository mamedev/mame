/*************************************************************************

    Ikki

*************************************************************************/

typedef struct _ikki_state ikki_state;
struct _ikki_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    scroll;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	bitmap_t   *sprite_bitmap;
	UINT8      flipscreen;
	int        punch_through_pen;
};


/*----------- defined in video/ikki.c -----------*/

WRITE8_HANDLER( ikki_scrn_ctrl_w );

PALETTE_INIT( ikki );
VIDEO_START( ikki );
VIDEO_UPDATE( ikki );
