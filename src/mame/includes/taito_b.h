
class taitob_state : public driver_device
{
public:
	taitob_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *      m_spriteram;
	UINT16 *      m_pixelram;
//  UINT16 *      m_paletteram;   // this currently uses generic palette handlers

	/* video-related */
	/* framebuffer is a raw bitmap, remapped as a last step */
	bitmap_t      *m_framebuffer[2];
	bitmap_t      *m_pixel_bitmap;
	bitmap_t      *m_realpunc_bitmap;

	UINT16        m_pixel_scroll[2];

	int           m_b_fg_color_base;
	int           m_b_sp_color_base;

	/* misc */
	UINT16        m_eep_latch;
	UINT16        m_coin_word;

	UINT16        m_realpunc_video_ctrl;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mb87078;
	device_t *m_ym;
	device_t *m_tc0180vcu;
	device_t *m_tc0640fio;
	device_t *m_tc0220ioc;
};


/*----------- defined in video/taito_b.c -----------*/

READ16_HANDLER( tc0180vcu_framebuffer_word_r );
WRITE16_HANDLER( tc0180vcu_framebuffer_word_w );

WRITE16_HANDLER( hitice_pixelram_w );
WRITE16_HANDLER( hitice_pixel_scroll_w );

WRITE16_HANDLER( realpunc_video_ctrl_w );

VIDEO_START( taitob_color_order0 );
VIDEO_START( taitob_color_order1 );
VIDEO_START( taitob_color_order2 );
VIDEO_START( hitice );
VIDEO_START( realpunc );

VIDEO_RESET( hitice );

SCREEN_UPDATE( realpunc );
SCREEN_UPDATE( taitob );

SCREEN_EOF( taitob );
