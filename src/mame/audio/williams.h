/***************************************************************************

    Midway/Williams Audio Boards

****************************************************************************/

MACHINE_CONFIG_EXTERN( williams_cvsd_sound );
MACHINE_CONFIG_EXTERN( williams_adpcm_sound );
MACHINE_CONFIG_EXTERN( williams_narc_sound );

void williams_cvsd_init(running_machine *machine);
void williams_cvsd_data_w(running_machine *machine, int data);
void williams_cvsd_reset_w(int state);

void williams_adpcm_init(running_machine *machine);
void williams_adpcm_data_w(int data);
void williams_adpcm_reset_w(int state);
int williams_adpcm_sound_irq_r(void);

void williams_narc_init(running_machine *machine);
void williams_narc_data_w(int data);
void williams_narc_reset_w(int state);
int williams_narc_talkback_r(void);
