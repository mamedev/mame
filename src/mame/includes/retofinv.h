class retofinv_state : public driver_device
{
public:
	retofinv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_cpu2_m6000;
	UINT8 *m_fg_videoram;
	UINT8 *m_bg_videoram;
	UINT8 *m_sharedram;
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_ddrA;
	UINT8 m_portB_in;
	UINT8 m_portB_out;
	UINT8 m_ddrB;
	UINT8 m_portC_in;
	UINT8 m_portC_out;
	UINT8 m_ddrC;
	int m_fg_bank;
	int m_bg_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	DECLARE_WRITE8_MEMBER(cpu1_reset_w);
	DECLARE_WRITE8_MEMBER(cpu2_reset_w);
	DECLARE_WRITE8_MEMBER(mcu_reset_w);
	DECLARE_WRITE8_MEMBER(cpu2_m6000_w);
	DECLARE_READ8_MEMBER(cpu0_mf800_r);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_WRITE8_MEMBER(irq0_ack_w);
	DECLARE_WRITE8_MEMBER(irq1_ack_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(coinlockout_w);
	DECLARE_READ8_MEMBER(retofinv_68705_portA_r);
	DECLARE_WRITE8_MEMBER(retofinv_68705_portA_w);
	DECLARE_WRITE8_MEMBER(retofinv_68705_ddrA_w);
	DECLARE_READ8_MEMBER(retofinv_68705_portB_r);
	DECLARE_WRITE8_MEMBER(retofinv_68705_portB_w);
	DECLARE_WRITE8_MEMBER(retofinv_68705_ddrB_w);
	DECLARE_READ8_MEMBER(retofinv_68705_portC_r);
	DECLARE_WRITE8_MEMBER(retofinv_68705_portC_w);
	DECLARE_WRITE8_MEMBER(retofinv_68705_ddrC_w);
	DECLARE_WRITE8_MEMBER(retofinv_mcu_w);
	DECLARE_READ8_MEMBER(retofinv_mcu_r);
	DECLARE_READ8_MEMBER(retofinv_mcu_status_r);
	DECLARE_WRITE8_MEMBER(retofinv_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(retofinv_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(retofinv_gfx_ctrl_w);
};


/*----------- defined in machine/retofinv.c -----------*/



/*----------- defined in video/retofinv.c -----------*/

VIDEO_START( retofinv );
PALETTE_INIT( retofinv );
SCREEN_UPDATE_IND16( retofinv );
