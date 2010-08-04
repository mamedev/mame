
class taitob_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitob_state(machine)); }

	taitob_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mb87078;
	running_device *ym;
	running_device *tc0180vcu;
	running_device *tc0640fio;
	running_device *tc0220ioc;
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
