
class nycaptor_state : public driver_device
{
public:
	nycaptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_sharedram;
	UINT8 *      m_scrlram;
	UINT8 *      m_videoram;
	UINT8 *      m_spriteram;
	size_t       m_videoram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_char_bank;
	int m_palette_bank;
	int m_gfxctrl;

	/* mcu */
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;
	UINT8 m_port_c_in;
	UINT8 m_port_c_out;
	UINT8 m_ddr_c;

	/* misc */
	int m_generic_control_reg;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	UINT8 m_snd_data;
	int m_vol_ctrl[16];
	int  m_gametype;
	int m_mask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_mcu;
	DECLARE_WRITE8_MEMBER(sub_cpu_halt_w);
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_READ8_MEMBER(nycaptor_sharedram_r);
	DECLARE_WRITE8_MEMBER(nycaptor_sharedram_w);
	DECLARE_READ8_MEMBER(nycaptor_b_r);
	DECLARE_READ8_MEMBER(nycaptor_by_r);
	DECLARE_READ8_MEMBER(nycaptor_bx_r);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(nycaptor_generic_control_r);
	DECLARE_WRITE8_MEMBER(nycaptor_generic_control_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_r);
	DECLARE_WRITE8_MEMBER(cyclshtg_mcu_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r1);
	DECLARE_WRITE8_MEMBER(cyclshtg_generic_control_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(nycaptor_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(nycaptor_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(nycaptor_mcu_w);
	DECLARE_READ8_MEMBER(nycaptor_mcu_r);
	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r1);
	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r2);
	DECLARE_WRITE8_MEMBER(nycaptor_spriteram_w);
	DECLARE_READ8_MEMBER(nycaptor_spriteram_r);
	DECLARE_WRITE8_MEMBER(nycaptor_videoram_w);
	DECLARE_READ8_MEMBER(nycaptor_videoram_r);
	DECLARE_WRITE8_MEMBER(nycaptor_palette_w);
	DECLARE_READ8_MEMBER(nycaptor_palette_r);
	DECLARE_WRITE8_MEMBER(nycaptor_gfxctrl_w);
	DECLARE_READ8_MEMBER(nycaptor_gfxctrl_r);
	DECLARE_READ8_MEMBER(nycaptor_scrlram_r);
	DECLARE_WRITE8_MEMBER(nycaptor_scrlram_w);
};


/*----------- defined in machine/nycaptor.c -----------*/




/*----------- defined in video/nycaptor.c -----------*/



VIDEO_START( nycaptor );
SCREEN_UPDATE_IND16( nycaptor );
