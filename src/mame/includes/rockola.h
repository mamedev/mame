/*************************************************************************

    rokola hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"


/*----------- defined in audio/rockola.c -----------*/

extern const samples_interface sasuke_samples_interface;
extern const samples_interface vanguard_samples_interface;
extern const samples_interface fantasy_samples_interface;
extern const sn76477_interface sasuke_sn76477_intf_1;
extern const sn76477_interface sasuke_sn76477_intf_2;
extern const sn76477_interface sasuke_sn76477_intf_3;
extern const sn76477_interface satansat_sn76477_intf;
extern const sn76477_interface vanguard_sn76477_intf_1;
extern const sn76477_interface vanguard_sn76477_intf_2;
extern const sn76477_interface fantasy_sn76477_intf;

extern WRITE8_HANDLER( sasuke_sound_w );
extern WRITE8_HANDLER( satansat_sound_w );
extern WRITE8_HANDLER( vanguard_sound_w );
extern WRITE8_HANDLER( vanguard_speech_w );
extern WRITE8_HANDLER( fantasy_sound_w );
extern WRITE8_HANDLER( fantasy_speech_w );

DEVICE_GET_INFO( rockola_sound );
#define SOUND_ROCKOLA DEVICE_GET_INFO_NAME(rockola_sound)

void rockola_set_music_clock(double clock_time);
void rockola_set_music_freq(int freq);
int rockola_music0_playing(void);

DISCRETE_SOUND_EXTERN( fantasy );


/*----------- defined in video/rockola.c -----------*/

extern UINT8 *rockola_videoram2;
extern UINT8 *rockola_charram;

WRITE8_HANDLER( rockola_videoram_w );
WRITE8_HANDLER( rockola_videoram2_w );
WRITE8_HANDLER( rockola_colorram_w );
WRITE8_HANDLER( rockola_charram_w );
WRITE8_HANDLER( rockola_flipscreen_w );
WRITE8_HANDLER( rockola_scrollx_w );
WRITE8_HANDLER( rockola_scrolly_w );

PALETTE_INIT( rockola );
VIDEO_START( rockola );
VIDEO_UPDATE( rockola );

WRITE8_HANDLER( satansat_b002_w );
WRITE8_HANDLER( satansat_backcolor_w );

PALETTE_INIT( satansat );
VIDEO_START( satansat );

