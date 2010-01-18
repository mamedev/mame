/***************************************************************************

    Ninja Gaiden

***************************************************************************/

typedef struct _gaiden_state gaiden_state;
struct _gaiden_state
{
	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    videoram2;
	UINT16 *    videoram3;
	UINT16 *    spriteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *text_layer,*foreground,*background;
	bitmap_t    *sprite_bitmap, *tile_bitmap_bg, *tile_bitmap_fg;
	UINT16      tx_scroll_x, tx_scroll_y;
	UINT16      bg_scroll_x, bg_scroll_y;
	UINT16      fg_scroll_x, fg_scroll_y;

	/* misc */
	int         sprite_sizey;
	int         prot, jumpcode;
	const int   *raiga_jumppoints;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/gaiden.c -----------*/

VIDEO_START( gaiden );
VIDEO_START( raiga );
VIDEO_START( drgnbowl );
VIDEO_START( mastninj );

VIDEO_UPDATE( gaiden );
VIDEO_UPDATE( raiga );
VIDEO_UPDATE( drgnbowl );

WRITE16_HANDLER( gaiden_videoram_w );
WRITE16_HANDLER( gaiden_videoram2_w );
READ16_HANDLER( gaiden_videoram2_r );
WRITE16_HANDLER( gaiden_videoram3_w );
READ16_HANDLER( gaiden_videoram3_r );

WRITE16_HANDLER( gaiden_txscrollx_w );
WRITE16_HANDLER( gaiden_txscrolly_w );
WRITE16_HANDLER( gaiden_fgscrollx_w );
WRITE16_HANDLER( gaiden_fgscrolly_w );
WRITE16_HANDLER( gaiden_bgscrollx_w );
WRITE16_HANDLER( gaiden_bgscrolly_w );
WRITE16_HANDLER( gaiden_flip_w );
