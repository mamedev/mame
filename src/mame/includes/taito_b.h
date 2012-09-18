
class taitob_state : public driver_device
{
public:
	taitob_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_pixelram;
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
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
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
	DECLARE_INPUT_CHANGED_MEMBER(realpunc_sensor);
	DECLARE_DRIVER_INIT(taito_b);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_VIDEO_START(taitob_color_order0);
	DECLARE_VIDEO_START(taitob_color_order1);
	DECLARE_VIDEO_START(taitob_color_order2);
	DECLARE_VIDEO_START(hitice);
	DECLARE_VIDEO_RESET(hitice);
	DECLARE_VIDEO_START(realpunc);
	DECLARE_VIDEO_START(taitob_core);
	UINT32 screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_taitob(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(rastansaga2_interrupt);
	INTERRUPT_GEN_MEMBER(crimec_interrupt);
	INTERRUPT_GEN_MEMBER(hitice_interrupt);
	INTERRUPT_GEN_MEMBER(rambo3_interrupt);
	INTERRUPT_GEN_MEMBER(pbobble_interrupt);
	INTERRUPT_GEN_MEMBER(viofight_interrupt);
	INTERRUPT_GEN_MEMBER(masterw_interrupt);
	INTERRUPT_GEN_MEMBER(silentd_interrupt);
	INTERRUPT_GEN_MEMBER(selfeena_interrupt);
	INTERRUPT_GEN_MEMBER(sbm_interrupt);
	INTERRUPT_GEN_MEMBER(realpunc_interrupt);
};
