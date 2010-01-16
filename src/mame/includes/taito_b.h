
typedef struct _taitob_state taitob_state;
struct _taitob_state
{
	/* memory pointers */
	UINT16 *      spriteram;
	UINT16 *      pixelram;
//  UINT16 *      paletteram;   // this currently uses generic palette handlers

	/* video-related */
	/* framebuffer is a raw bitmap, remapped as a last step */
	bitmap_t      *framebuffer[2], *pixel_bitmap;

	UINT16        pixel_scroll[2];

	int           b_fg_color_base;
	int           b_sp_color_base;

	/* misc */
	UINT16        eep_latch;
	UINT16        coin_word;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *mb87078;
	const device_config *ym;
	const device_config *tc0180vcu;
	const device_config *tc0640fio;
	const device_config *tc0220ioc;
};


/*----------- defined in video/taito_b.c -----------*/

READ16_HANDLER( tc0180vcu_framebuffer_word_r );
WRITE16_HANDLER( tc0180vcu_framebuffer_word_w );

WRITE16_HANDLER( hitice_pixelram_w );
WRITE16_HANDLER( hitice_pixel_scroll_w );

VIDEO_START( taitob_color_order0 );
VIDEO_START( taitob_color_order1 );
VIDEO_START( taitob_color_order2 );
VIDEO_START( hitice );

VIDEO_RESET( hitice );

VIDEO_UPDATE( taitob );

VIDEO_EOF( taitob );
