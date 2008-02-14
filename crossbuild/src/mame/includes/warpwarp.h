#include "sound/custom.h"

/*----------- defined in video/warpwarp.c -----------*/

extern UINT8 *geebee_videoram,*warpwarp_videoram;
extern int geebee_bgw;
extern int warpwarp_ball_on;
extern int warpwarp_ball_h, warpwarp_ball_v;
extern int warpwarp_ball_sizex, warpwarp_ball_sizey;
extern int geebee_handleoverlay;

PALETTE_INIT( geebee );
PALETTE_INIT( navarone );
PALETTE_INIT( warpwarp );
VIDEO_START( geebee );
VIDEO_START( navarone );
VIDEO_START( warpwarp );
VIDEO_UPDATE( geebee );
VIDEO_UPDATE( warpwarp );
WRITE8_HANDLER( warpwarp_videoram_w );
WRITE8_HANDLER( geebee_videoram_w );


/*----------- defined in audio/geebee.c -----------*/

WRITE8_HANDLER( geebee_sound_w );
void *geebee_sh_start(int clock, const struct CustomSound_interface *config);


/*----------- defined in audio/warpwarp.c -----------*/

WRITE8_HANDLER( warpwarp_sound_w );
WRITE8_HANDLER( warpwarp_music1_w );
WRITE8_HANDLER( warpwarp_music2_w );
void *warpwarp_sh_start(int clock, const struct CustomSound_interface *config);
