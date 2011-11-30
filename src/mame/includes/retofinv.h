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
};


/*----------- defined in machine/retofinv.c -----------*/

READ8_HANDLER( retofinv_68705_portA_r );
WRITE8_HANDLER( retofinv_68705_portA_w );
WRITE8_HANDLER( retofinv_68705_ddrA_w );
READ8_HANDLER( retofinv_68705_portB_r );
WRITE8_HANDLER( retofinv_68705_portB_w );
WRITE8_HANDLER( retofinv_68705_ddrB_w );
READ8_HANDLER( retofinv_68705_portC_r );
WRITE8_HANDLER( retofinv_68705_portC_w );
WRITE8_HANDLER( retofinv_68705_ddrC_w );
WRITE8_HANDLER( retofinv_mcu_w );
READ8_HANDLER( retofinv_mcu_r );
READ8_HANDLER( retofinv_mcu_status_r );


/*----------- defined in video/retofinv.c -----------*/

VIDEO_START( retofinv );
PALETTE_INIT( retofinv );
SCREEN_UPDATE( retofinv );
WRITE8_HANDLER( retofinv_bg_videoram_w );
WRITE8_HANDLER( retofinv_fg_videoram_w );
WRITE8_HANDLER( retofinv_gfx_ctrl_w );
