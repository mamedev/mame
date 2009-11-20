/*************************************************************************

    Pandora's Palace

*************************************************************************/

typedef struct _pandoras_state pandoras_state;
struct _pandoras_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;

	/* video-related */
	tilemap     *layer0;
	int         flipscreen;

	int irq_enable_a, irq_enable_b;
	int firq_old_data_a, firq_old_data_b;
	int i8039_status;
};


/* defined in video/pandoras.c */
PALETTE_INIT( pandoras );

WRITE8_HANDLER( pandoras_vram_w );
WRITE8_HANDLER( pandoras_cram_w );
WRITE8_HANDLER( pandoras_flipscreen_w );
WRITE8_HANDLER( pandoras_scrolly_w );

VIDEO_START( pandoras );
VIDEO_UPDATE( pandoras );

