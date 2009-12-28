/*************************************************************************

    Capcom Baseball

*************************************************************************/

typedef struct _cbasebal_state cbasebal_state;
struct _cbasebal_state
{
	/* memory pointers */
	UINT8 *    spriteram;
//  UINT8 *    paletteram;    // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *fg_tilemap, *bg_tilemap;
	UINT8      *textram, *scrollram;
	UINT8      scroll_x[2], scroll_y[2];
	int        tilebank, spritebank;
	int        text_on, bg_on, obj_on;
	int        flipscreen;

	/* misc */
	UINT8      rambank;
};

/*----------- defined in video/cbasebal.c -----------*/

WRITE8_HANDLER( cbasebal_textram_w );
READ8_HANDLER( cbasebal_textram_r );
WRITE8_HANDLER( cbasebal_scrollram_w );
READ8_HANDLER( cbasebal_scrollram_r );
WRITE8_HANDLER( cbasebal_gfxctrl_w );
WRITE8_HANDLER( cbasebal_scrollx_w );
WRITE8_HANDLER( cbasebal_scrolly_w );

VIDEO_START( cbasebal );
VIDEO_UPDATE( cbasebal );
