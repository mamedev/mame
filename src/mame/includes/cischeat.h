/* TODO: some variables are per-game specifics */

class cischeat_state : public driver_device
{
public:
	cischeat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_scrollram[3];
	UINT16 *m_objectram;
	UINT16 *m_vregs;
	UINT16 *m_ram;
	tilemap_t *m_tmap[3];
	tilemap_t *m_tilemap[3][2][4];
	int m_scrollx[3];
	int m_scrolly[3];
	int m_active_layers;
	int m_bits_per_color_code;
	int m_scroll_flag[3];

	int m_prev;
	int m_armold;
	UINT16 m_scudhamm_motor_command;
	UINT16 *m_roadram[2];
	UINT16 *m_f1gpstr2_ioready;
	int m_ip_select;
	UINT8 m_drawmode_table[16];
	int m_debugsprites;
	int m_show_unknown;
	UINT16 *m_spriteram;
	DECLARE_WRITE16_MEMBER(bigrun_paletteram16_w);
	DECLARE_WRITE16_MEMBER(cischeat_paletteram16_w);
	DECLARE_WRITE16_MEMBER(f1gpstar_paletteram16_w);
	DECLARE_WRITE16_MEMBER(scudhamm_paletteram16_w);
	DECLARE_WRITE16_MEMBER(scudhamm_motor_command_w);
	DECLARE_WRITE16_MEMBER(scudhamm_leds_w);
	DECLARE_WRITE16_MEMBER(scudhamm_enable_w);
	DECLARE_WRITE16_MEMBER(scudhamm_oki_bank_w);
	DECLARE_READ16_MEMBER(armchmp2_motor_status_r);
	DECLARE_WRITE16_MEMBER(armchmp2_motor_command_w);
	DECLARE_READ16_MEMBER(armchmp2_analog_r);
	DECLARE_READ16_MEMBER(armchmp2_buttons_r);
	DECLARE_WRITE16_MEMBER(armchmp2_leds_w);
	DECLARE_WRITE16_MEMBER(bigrun_soundbank_w);
	DECLARE_READ16_MEMBER(f1gpstr2_io_r);
	DECLARE_WRITE16_MEMBER(f1gpstr2_io_w);
	DECLARE_READ16_MEMBER(scudhamm_motor_status_r);
	DECLARE_READ16_MEMBER(scudhamm_motor_pos_r);
	DECLARE_READ16_MEMBER(scudhamm_analog_r);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_0_w);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_1_w);
	DECLARE_WRITE16_MEMBER(cischeat_scrollram_2_w);
	DECLARE_READ16_MEMBER(bigrun_vregs_r);
	DECLARE_WRITE16_MEMBER(bigrun_vregs_w);
	DECLARE_READ16_MEMBER(cischeat_vregs_r);
	DECLARE_WRITE16_MEMBER(cischeat_vregs_w);
	DECLARE_READ16_MEMBER(f1gpstar_vregs_r);
	DECLARE_READ16_MEMBER(f1gpstr2_vregs_r);
	DECLARE_READ16_MEMBER(wildplt_vregs_r);
	DECLARE_WRITE16_MEMBER(f1gpstar_vregs_w);
	DECLARE_WRITE16_MEMBER(f1gpstr2_vregs_w);
	DECLARE_WRITE16_MEMBER(scudhamm_vregs_w);
	void cischeat_set_vreg_flag(int which, int data);
};


/*----------- defined in video/cischeat.c -----------*/




VIDEO_START( bigrun );
VIDEO_START( cischeat );
VIDEO_START( f1gpstar );

SCREEN_UPDATE_IND16( bigrun );
SCREEN_UPDATE_IND16( cischeat );
SCREEN_UPDATE_IND16( f1gpstar );
SCREEN_UPDATE_IND16( scudhamm );
