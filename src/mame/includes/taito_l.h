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
