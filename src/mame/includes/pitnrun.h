class pitnrun_state : public driver_device
{
public:
	pitnrun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_nmi;
	UINT8* m_videoram2;
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
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
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
};


/*----------- defined in machine/pitnrun.c -----------*/



MACHINE_RESET( pitnrun );



/*----------- defined in video/pitnrun.c -----------*/


PALETTE_INIT(pitnrun);
VIDEO_START(pitnrun);
SCREEN_UPDATE_IND16(pitnrun);
