/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "machine/6532riot.h"


/*----------- defined in audio/gottlieb.c -----------*/

MACHINE_DRIVER_EXTERN( gottlieb_soundrev1 );
MACHINE_DRIVER_EXTERN( gottlieb_soundrev2 );

WRITE8_HANDLER( gottlieb_sh_w );


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
