/*************************************************************************

    rokola hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"


/*----------- defined in audio/snk6502.c -----------*/

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

DEVICE_GET_INFO( snk6502_sound );
#define SOUND_snk6502 DEVICE_GET_INFO_NAME(snk6502_sound)

void snk6502_set_music_clock(double clock_time);
void snk6502_set_music_freq(int freq);
int snk6502_music0_playing(void);

DISCRETE_SOUND_EXTERN( fantasy );


/*----------- defined in video/snk6502.c -----------*/

extern UINT8 *snk6502_videoram;
extern UINT8 *snk6502_colorram;
extern UINT8 *snk6502_videoram2;
extern UINT8 *snk6502_charram;

WRITE8_HANDLER( snk6502_videoram_w );
WRITE8_HANDLER( snk6502_videoram2_w );
WRITE8_HANDLER( snk6502_colorram_w );
WRITE8_HANDLER( snk6502_charram_w );
WRITE8_HANDLER( snk6502_flipscreen_w );
WRITE8_HANDLER( snk6502_scrollx_w );
WRITE8_HANDLER( snk6502_scrolly_w );

PALETTE_INIT( snk6502 );
VIDEO_START( snk6502 );
VIDEO_UPDATE( snk6502 );

WRITE8_HANDLER( satansat_b002_w );
WRITE8_HANDLER( satansat_backcolor_w );

PALETTE_INIT( satansat );
VIDEO_START( satansat );

