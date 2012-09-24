class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"){ }

	required_shared_ptr<UINT8> m_videoram;
	int m_nmi;
	required_shared_ptr<UINT8> m_videoram2;
	UINT8 m_fromz80;
	UINT8 m_toz80;
	int m_zaccept;
	int m_zready;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	int m_address;
	int m_h_heed;
	int m_v_heed;
	int m_ha;
	int m_scroll;
	int m_char_bank;
	int m_color_select;
	bitmap_ind16 *m_tmp_bitmap[4];
	tilemap_t *m_bg;
	tilemap_t *m_fg;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(pitnrun_hflip_w);
	DECLARE_WRITE8_MEMBER(pitnrun_vflip_w);
	DECLARE_READ8_MEMBER(pitnrun_mcu_data_r);
	DECLARE_WRITE8_MEMBER(pitnrun_mcu_data_w);
	DECLARE_READ8_MEMBER(pitnrun_mcu_status_r);
	DECLARE_READ8_MEMBER(pitnrun_68705_portA_r);
	DECLARE_WRITE8_MEMBER(pitnrun_68705_portA_w);
	DECLARE_READ8_MEMBER(pitnrun_68705_portB_r);
	DECLARE_WRITE8_MEMBER(pitnrun_68705_portB_w);
	DECLARE_READ8_MEMBER(pitnrun_68705_portC_r);
	DECLARE_WRITE8_MEMBER(pitnrun_videoram_w);
	DECLARE_WRITE8_MEMBER(pitnrun_videoram2_w);
	DECLARE_WRITE8_MEMBER(pitnrun_char_bank_select);
	DECLARE_WRITE8_MEMBER(pitnrun_scroll_w);
	DECLARE_WRITE8_MEMBER(pitnrun_ha_w);
	DECLARE_WRITE8_MEMBER(pitnrun_h_heed_w);
	DECLARE_WRITE8_MEMBER(pitnrun_v_heed_w);
	DECLARE_WRITE8_MEMBER(pitnrun_color_select_w);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_pitnrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pitnrun_nmi_source);
	TIMER_CALLBACK_MEMBER(pitnrun_mcu_real_data_r);
	TIMER_CALLBACK_MEMBER(pitnrun_mcu_real_data_w);
	TIMER_CALLBACK_MEMBER(pitnrun_mcu_data_real_r);
	TIMER_CALLBACK_MEMBER(pitnrun_mcu_status_real_w);
};
