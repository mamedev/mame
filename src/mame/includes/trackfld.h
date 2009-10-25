/***************************************************************************

    Track'n'Field

***************************************************************************/

#include "sound/msm5205.h"

/*----------- defined in audio/trackfld.c -----------*/

WRITE8_HANDLER( konami_sh_irqtrigger_w );
READ8_HANDLER( trackfld_sh_timer_r );
READ8_DEVICE_HANDLER( trackfld_speech_r );
WRITE8_DEVICE_HANDLER( trackfld_sound_w );
READ8_HANDLER( hyperspt_sh_timer_r );
WRITE8_DEVICE_HANDLER( hyperspt_sound_w );
WRITE8_HANDLER( konami_SN76496_latch_w );
WRITE8_DEVICE_HANDLER( konami_SN76496_w );

/*----------- defined in drivers/trackfld.c -----------*/
extern const msm5205_interface hyprolyb_msm5205_config;
extern WRITE8_HANDLER( hyprolyb_adpcm_w );
ADDRESS_MAP_EXTERN( hyprolyb_adpcm_map, 8 );

/*----------- defined in video/trackfld.c -----------*/

extern UINT8 *trackfld_scroll;
extern UINT8 *trackfld_scroll2;

WRITE8_HANDLER( trackfld_videoram_w );
WRITE8_HANDLER( trackfld_colorram_w );
WRITE8_HANDLER( trackfld_flipscreen_w );
WRITE8_HANDLER( atlantol_gfxbank_w );

PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
VIDEO_UPDATE( trackfld );
