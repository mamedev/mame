/*************************************************************************

    rokola hardware

*************************************************************************/

#include "sound/discrete.h"


/*----------- defined in sndhrdw/rokola.c -----------*/

extern const struct CustomSound_interface custom_interface;
extern const struct Samplesinterface sasuke_samples_interface;
extern const struct Samplesinterface vanguard_samples_interface;
extern const struct Samplesinterface fantasy_samples_interface;
extern const struct SN76477interface sasuke_sn76477_intf_1;
extern const struct SN76477interface sasuke_sn76477_intf_2;
extern const struct SN76477interface sasuke_sn76477_intf_3;
extern const struct SN76477interface satansat_sn76477_intf;
extern const struct SN76477interface vanguard_sn76477_intf_1;
extern const struct SN76477interface vanguard_sn76477_intf_2;
extern const struct SN76477interface fantasy_sn76477_intf;

extern WRITE8_HANDLER( sasuke_sound_w );
extern WRITE8_HANDLER( satansat_sound_w );
extern WRITE8_HANDLER( vanguard_sound_w );
extern WRITE8_HANDLER( vanguard_speech_w );
extern WRITE8_HANDLER( fantasy_sound_w );
extern WRITE8_HANDLER( fantasy_speech_w );

void *rockola_sh_start(int clock, const struct CustomSound_interface *config);
void rockola_set_music_clock(double clock_time);
void rockola_set_music_freq(int freq);
int rockola_music0_playing(void);

DISCRETE_SOUND_EXTERN( fantasy );
