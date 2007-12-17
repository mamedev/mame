/***************************************************************************

    Midway/Williams Audio Boards

****************************************************************************/

MACHINE_DRIVER_EXTERN( williams_cvsd_sound );
MACHINE_DRIVER_EXTERN( williams_adpcm_sound );
MACHINE_DRIVER_EXTERN( williams_narc_sound );

void williams_cvsd_init(int pianum);
void williams_cvsd_data_w(int data);
void williams_cvsd_reset_w(int state);

void williams_adpcm_init(void);
void williams_adpcm_data_w(int data);
void williams_adpcm_reset_w(int state);

void williams_narc_init(void);
void williams_narc_data_w(int data);
void williams_narc_reset_w(int state);

