
#define TAITOL_SPRITERAM_SIZE 0x400

class taitol_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitol_state(machine)); }

	taitol_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *       rambanks;
	UINT8 *       palette_ram;
	UINT8 *       empty_ram;
	UINT8 *       shared_ram;

	/* video-related */
	tilemap_t *bg18_tilemap, *bg19_tilemap, *ch1a_tilemap;
	UINT8 buff_spriteram[TAITOL_SPRITERAM_SIZE];
	int cur_ctrl;
	int horshoes_gfxbank;
	int bankc[4];
	int flipscreen;

	/* misc */
	void (*current_notifier[4])(running_machine *, int);
	UINT8 *current_base[4];

	int cur_rombank, cur_rombank2, cur_rambank[4];
	int irq_adr_table[3];
	int irq_enable;
	int adpcm_pos;
	int adpcm_data;
	int trackx, tracky;
	int mux_ctrl;
	int extport;
	int last_irq_level;
	int high;
	int high2;
	int last_data_adr, last_data;
	int cur_bank;

	const UINT8 *mcu_reply;
	int mcu_pos, mcu_reply_len;

	const char *porte0_tag;
	const char *porte1_tag;
	const char *portf0_tag;
	const char *portf1_tag;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/taito_l.c -----------*/

VIDEO_EOF( taitol );
VIDEO_START( taitol );
VIDEO_UPDATE( taitol );

void taitol_chardef14_m(running_machine *machine, int offset);
void taitol_chardef15_m(running_machine *machine, int offset);
void taitol_chardef16_m(running_machine *machine, int offset);
void taitol_chardef17_m(running_machine *machine, int offset);
void taitol_chardef1c_m(running_machine *machine, int offset);
void taitol_chardef1d_m(running_machine *machine, int offset);
void taitol_chardef1e_m(running_machine *machine, int offset);
void taitol_chardef1f_m(running_machine *machine, int offset);
void taitol_bg18_m(running_machine *machine, int offset);
void taitol_bg19_m(running_machine *machine, int offset);
void taitol_char1a_m(running_machine *machine, int offset);
void taitol_obj1b_m(running_machine *machine, int offset);

WRITE8_HANDLER( taitol_control_w );
READ8_HANDLER( taitol_control_r );
WRITE8_HANDLER( horshoes_bankg_w );
WRITE8_HANDLER( taitol_bankc_w );
READ8_HANDLER( taitol_bankc_r );
