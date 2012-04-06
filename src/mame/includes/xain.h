class xain_state : public driver_device
{
public:
	xain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_vblank;
	int m_from_main;
	int m_from_mcu;
	UINT8 m_ddr_a;
	UINT8 m_ddr_b;
	UINT8 m_ddr_c;
	UINT8 m_port_a_out;
	UINT8 m_port_b_out;
	UINT8 m_port_c_out;
	UINT8 m_port_a_in;
	UINT8 m_port_b_in;
	UINT8 m_port_c_in;
	int m_mcu_ready;
	int m_mcu_accept;
	UINT8 *m_charram;
	UINT8 *m_bgram0;
	UINT8 *m_bgram1;
	UINT8 m_pri;
	tilemap_t *m_char_tilemap;
	tilemap_t *m_bgram0_tilemap;
	tilemap_t *m_bgram1_tilemap;
	UINT8 m_scrollxP0[2];
	UINT8 m_scrollyP0[2];
	UINT8 m_scrollxP1[2];
	UINT8 m_scrollyP1[2];
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(xainCPUA_bankswitch_w);
	DECLARE_WRITE8_MEMBER(xainCPUB_bankswitch_w);
	DECLARE_WRITE8_MEMBER(xain_sound_command_w);
	DECLARE_WRITE8_MEMBER(xain_main_irq_w);
	DECLARE_WRITE8_MEMBER(xain_irqA_assert_w);
	DECLARE_WRITE8_MEMBER(xain_irqB_clear_w);
	DECLARE_READ8_MEMBER(xain_68705_r);
	DECLARE_WRITE8_MEMBER(xain_68705_w);
	DECLARE_READ8_MEMBER(xain_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(xain_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(xain_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(xain_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(xain_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(xain_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(xain_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(xain_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(xain_68705_ddr_c_w);
	DECLARE_READ8_MEMBER(mcu_comm_reset_r);
	DECLARE_WRITE8_MEMBER(xain_bgram0_w);
	DECLARE_WRITE8_MEMBER(xain_bgram1_w);
	DECLARE_WRITE8_MEMBER(xain_charram_w);
	DECLARE_WRITE8_MEMBER(xain_scrollxP0_w);
	DECLARE_WRITE8_MEMBER(xain_scrollyP0_w);
	DECLARE_WRITE8_MEMBER(xain_scrollxP1_w);
	DECLARE_WRITE8_MEMBER(xain_scrollyP1_w);
	DECLARE_WRITE8_MEMBER(xain_flipscreen_w);
};


/*----------- defined in video/xain.c -----------*/

SCREEN_UPDATE_IND16( xain );
VIDEO_START( xain );
