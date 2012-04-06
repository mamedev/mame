
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
	bitmap_ind16      *m_framebuffer[2];
	bitmap_ind16      *m_pixel_bitmap;
	bitmap_ind16      *m_realpunc_bitmap;

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
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ16_MEMBER(tracky1_hi_r);
	DECLARE_READ16_MEMBER(tracky1_lo_r);
	DECLARE_READ16_MEMBER(trackx1_hi_r);
	DECLARE_READ16_MEMBER(trackx1_lo_r);
	DECLARE_READ16_MEMBER(tracky2_hi_r);
	DECLARE_READ16_MEMBER(tracky2_lo_r);
	DECLARE_READ16_MEMBER(trackx2_hi_r);
	DECLARE_READ16_MEMBER(trackx2_lo_r);
	DECLARE_WRITE16_MEMBER(gain_control_w);
	DECLARE_READ16_MEMBER(eep_latch_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_READ16_MEMBER(player_34_coin_ctrl_r);
	DECLARE_WRITE16_MEMBER(player_34_coin_ctrl_w);
	DECLARE_READ16_MEMBER(pbobble_input_bypass_r);
	DECLARE_WRITE16_MEMBER(spacedxo_tc0220ioc_w);
	DECLARE_WRITE16_MEMBER(realpunc_output_w);
	DECLARE_WRITE16_MEMBER(hitice_pixelram_w);
	DECLARE_WRITE16_MEMBER(hitice_pixel_scroll_w);
	DECLARE_WRITE16_MEMBER(realpunc_video_ctrl_w);
	DECLARE_READ16_MEMBER(tc0180vcu_framebuffer_word_r);
	DECLARE_WRITE16_MEMBER(tc0180vcu_framebuffer_word_w);
};


/*----------- defined in video/taito_b.c -----------*/




VIDEO_START( taitob_color_order0 );
VIDEO_START( taitob_color_order1 );
VIDEO_START( taitob_color_order2 );
VIDEO_START( hitice );
VIDEO_START( realpunc );

VIDEO_RESET( hitice );

SCREEN_UPDATE_RGB32( realpunc );
SCREEN_UPDATE_IND16( taitob );

SCREEN_VBLANK( taitob );
