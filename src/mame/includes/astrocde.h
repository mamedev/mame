/***************************************************************************

    Bally Astrocade-based hardware

***************************************************************************/

#include "sound/custom.h"

#define ASTROCADE_CLOCK		(14318180/2)

#define AC_SOUND_PRESENT	(0x01)
#define AC_LIGHTPEN_INTS	(0x02)
#define AC_STARS			(0x04)


/*----------- defined in video/astrocde.c -----------*/

extern UINT8 astrocade_video_config;
extern UINT8 astrocade_sparkle[4];

PALETTE_INIT( astrocde );
PALETTE_INIT( profpac );

VIDEO_START( astrocde );
VIDEO_START( profpac );

VIDEO_UPDATE( astrocde );
VIDEO_UPDATE( profpac );

void astrocade_trigger_lightpen(UINT8 vfeedback, UINT8 hfeedback);

WRITE8_HANDLER( astrocade_pattern_board_w );
READ8_HANDLER( astrocade_data_chip_register_r );
WRITE8_HANDLER( astrocade_data_chip_register_w );
WRITE8_HANDLER( astrocade_funcgen_w );

READ8_HANDLER( profpac_videoram_r );
WRITE8_HANDLER( profpac_videoram_w );
READ8_HANDLER( profpac_intercept_r );
WRITE8_HANDLER( profpac_page_select_w );
WRITE8_HANDLER( profpac_screenram_ctrl_w );


/*----------- defined in audio/wow.c -----------*/

extern const char *wow_sample_names[];

READ8_HANDLER( wow_speech_r );
UINT32 wow_speech_status_r(void *param);


/*----------- defined in audio/gorf.c -----------*/

extern const char *gorf_sample_names[];

READ8_HANDLER( gorf_speech_r );
UINT32 gorf_speech_status_r(void *param);
