/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "machine/6532riot.h"


/*----------- defined in audio/gottlieb.c -----------*/

extern const riot6532_interface gottlieb_riot6532_intf;

SOUND_START( gottlieb1 );
SOUND_START( gottlieb2 );

WRITE8_HANDLER( gottlieb_sh_w );

WRITE8_HANDLER( gottlieb_speech_w );
WRITE8_HANDLER( gottlieb_speech_clock_DAC_w );
void gottlieb_sound_init(void);
void stooges_sp0250_drq(int level);
READ8_HANDLER( stooges_sound_input_r );
WRITE8_HANDLER( stooges_8910_latch_w );
WRITE8_HANDLER( stooges_sp0250_latch_w );
WRITE8_HANDLER( stooges_sound_control_w );
WRITE8_HANDLER( gottlieb_nmi_rate_w );
WRITE8_HANDLER( gottlieb_cause_dac_nmi_w );


/*----------- defined in video/gottlieb.c -----------*/

extern UINT8 gottlieb_gfxcharlo;
extern UINT8 gottlieb_gfxcharhi;
extern UINT8 *gottlieb_charram;
extern UINT8 *gottlieb_riot_regs;

extern WRITE8_HANDLER( gottlieb_videoram_w );
extern WRITE8_HANDLER( gottlieb_charram_w );
extern WRITE8_HANDLER( gottlieb_video_outputs_w );
extern WRITE8_HANDLER( usvsthem_video_outputs_w );
extern WRITE8_HANDLER( gottlieb_paletteram_w );

VIDEO_START( gottlieb );
VIDEO_UPDATE( gottlieb );
