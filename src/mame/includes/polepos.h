/*************************************************************************

    Pole Position hardware

*************************************************************************/

#include "sound/discrete.h"
#include "sound/custom.h"


/*----------- defined in audio/polepos.c -----------*/

void *polepos_sh_start(int clock, const struct CustomSound_interface *config);
void polepos_sh_reset(void *token);
WRITE8_HANDLER( polepos_engine_sound_lsb_w );
WRITE8_HANDLER( polepos_engine_sound_msb_w );

DISCRETE_SOUND_EXTERN( polepos );


/*----------- defined in video/polepos.c -----------*/

extern UINT16 *polepos_view16_memory;
extern UINT16 *polepos_road16_memory;
extern UINT16 *polepos_alpha16_memory;
extern UINT16 *polepos_sprite16_memory;
extern int polepos_gear_bit;

VIDEO_START( polepos );
PALETTE_INIT( polepos );
VIDEO_UPDATE( polepos );

WRITE16_HANDLER( polepos_view16_w );
WRITE16_HANDLER( polepos_road16_w );
WRITE16_HANDLER( polepos_alpha16_w );
WRITE16_HANDLER( polepos_sprite16_w );
WRITE8_HANDLER( polepos_view_w );
WRITE8_HANDLER( polepos_road_w );
WRITE8_HANDLER( polepos_alpha_w );
WRITE8_HANDLER( polepos_sprite_w );
WRITE8_HANDLER( polepos_chacl_w );

READ16_HANDLER( polepos_view16_r );
READ16_HANDLER( polepos_road16_r );
READ16_HANDLER( polepos_alpha16_r );
READ16_HANDLER( polepos_sprite16_r );
READ8_HANDLER( polepos_view_r );
READ8_HANDLER( polepos_road_r );
READ8_HANDLER( polepos_alpha_r );
READ8_HANDLER( polepos_sprite_r );
WRITE16_HANDLER( polepos_view16_hscroll_w );
WRITE16_HANDLER( polepos_road16_vscroll_w );
