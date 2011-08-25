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
};


/*----------- defined in drivers/cischeat.c -----------*/

READ16_HANDLER( scudhamm_motor_pos_r );
READ16_HANDLER( scudhamm_motor_status_r );
READ16_HANDLER( scudhamm_analog_r );


/*----------- defined in video/cischeat.c -----------*/

WRITE16_HANDLER( cischeat_scrollram_0_w );
WRITE16_HANDLER( cischeat_scrollram_1_w );
WRITE16_HANDLER( cischeat_scrollram_2_w );

READ16_HANDLER( bigrun_vregs_r );
READ16_HANDLER( cischeat_vregs_r );
READ16_HANDLER( f1gpstar_vregs_r );
READ16_HANDLER( f1gpstr2_vregs_r );
READ16_HANDLER( wildplt_vregs_r );

WRITE16_HANDLER( bigrun_vregs_w );
WRITE16_HANDLER( cischeat_vregs_w );
WRITE16_HANDLER( f1gpstar_vregs_w );
WRITE16_HANDLER( f1gpstr2_vregs_w );
WRITE16_HANDLER( scudhamm_vregs_w );

VIDEO_START( bigrun );
VIDEO_START( cischeat );
VIDEO_START( f1gpstar );

SCREEN_UPDATE( bigrun );
SCREEN_UPDATE( cischeat );
SCREEN_UPDATE( f1gpstar );
SCREEN_UPDATE( scudhamm );
