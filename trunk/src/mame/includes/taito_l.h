
#define TAITOL_SPRITERAM_SIZE 0x400

class taitol_state : public driver_device
{
public:
	taitol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *       m_shared_ram;

	/* video-related */
	tilemap_t *m_bg18_tilemap;
	tilemap_t *m_bg19_tilemap;
	tilemap_t *m_ch1a_tilemap;
	UINT8 m_buff_spriteram[TAITOL_SPRITERAM_SIZE];
	int m_cur_ctrl;
	int m_horshoes_gfxbank;
	int m_bankc[4];
	int m_flipscreen;

	/* misc */
	void (*m_current_notifier[4])(running_machine &, int);
	UINT8 *m_current_base[4];

	int m_cur_rombank;
	int m_cur_rombank2;
	int m_cur_rambank[4];
	int m_irq_adr_table[3];
	int m_irq_enable;
	int m_adpcm_pos;
	int m_adpcm_data;
	int m_trackx;
	int m_tracky;
	int m_mux_ctrl;
	int m_extport;
	int m_last_irq_level;
	int m_high;
	int m_high2;
	int m_last_data_adr;
	int m_last_data;
	int m_cur_bank;

	const UINT8 *m_mcu_reply;
	int m_mcu_pos;
	int m_mcu_reply_len;

	const char *m_porte0_tag;
	const char *m_porte1_tag;
	const char *m_portf0_tag;
	const char *m_portf1_tag;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	/* memory buffers */
	UINT8         m_rambanks[0x1000 * 12];
	UINT8         m_palette_ram[0x1000];
	UINT8         m_empty_ram[0x1000];
	DECLARE_WRITE8_MEMBER(irq_adr_w);
	DECLARE_READ8_MEMBER(irq_adr_r);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_READ8_MEMBER(irq_enable_r);
	DECLARE_WRITE8_MEMBER(rombankswitch_w);
	DECLARE_WRITE8_MEMBER(rombank2switch_w);
	DECLARE_READ8_MEMBER(rombankswitch_r);
	DECLARE_READ8_MEMBER(rombank2switch_r);
	DECLARE_WRITE8_MEMBER(rambankswitch_w);
	DECLARE_READ8_MEMBER(rambankswitch_r);
	DECLARE_WRITE8_MEMBER(bank0_w);
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	DECLARE_WRITE8_MEMBER(bank3_w);
	DECLARE_WRITE8_MEMBER(control2_w);
	DECLARE_WRITE8_MEMBER(mcu_data_w);
	DECLARE_WRITE8_MEMBER(mcu_control_w);
	DECLARE_READ8_MEMBER(mcu_data_r);
	DECLARE_READ8_MEMBER(mcu_control_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(mux_ctrl_w);
	DECLARE_WRITE8_MEMBER(champwr_msm5205_lo_w);
	DECLARE_WRITE8_MEMBER(champwr_msm5205_hi_w);
	DECLARE_READ8_MEMBER(horshoes_tracky_reset_r);
	DECLARE_READ8_MEMBER(horshoes_trackx_reset_r);
	DECLARE_READ8_MEMBER(horshoes_tracky_lo_r);
	DECLARE_READ8_MEMBER(horshoes_tracky_hi_r);
	DECLARE_READ8_MEMBER(horshoes_trackx_lo_r);
	DECLARE_READ8_MEMBER(horshoes_trackx_hi_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(horshoes_bankg_w);
	DECLARE_WRITE8_MEMBER(taitol_bankc_w);
	DECLARE_READ8_MEMBER(taitol_bankc_r);
	DECLARE_WRITE8_MEMBER(taitol_control_w);
	DECLARE_READ8_MEMBER(taitol_control_r);
};

/*----------- defined in video/taito_l.c -----------*/

SCREEN_VBLANK( taitol );
VIDEO_START( taitol );
SCREEN_UPDATE_IND16( taitol );

void taitol_chardef14_m(running_machine &machine, int offset);
void taitol_chardef15_m(running_machine &machine, int offset);
void taitol_chardef16_m(running_machine &machine, int offset);
void taitol_chardef17_m(running_machine &machine, int offset);
void taitol_chardef1c_m(running_machine &machine, int offset);
void taitol_chardef1d_m(running_machine &machine, int offset);
void taitol_chardef1e_m(running_machine &machine, int offset);
void taitol_chardef1f_m(running_machine &machine, int offset);
void taitol_bg18_m(running_machine &machine, int offset);
void taitol_bg19_m(running_machine &machine, int offset);
void taitol_char1a_m(running_machine &machine, int offset);
void taitol_obj1b_m(running_machine &machine, int offset);

