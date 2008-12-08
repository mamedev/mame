#include "sound/custom.h"

/*----------- defined in audio/tiamc1.c -----------*/

CUSTOM_START( tiamc1_sh_start );
WRITE8_HANDLER( tiamc1_timer0_w );
WRITE8_HANDLER( tiamc1_timer1_w );
WRITE8_HANDLER( tiamc1_timer1_gate_w );


/*----------- defined in video/tiamc1.c -----------*/

extern UINT8 *tiamc1_charram;
extern UINT8 *tiamc1_tileram;
extern UINT8 *tiamc1_spriteram_x;
extern UINT8 *tiamc1_spriteram_y;
extern UINT8 *tiamc1_spriteram_n;
extern UINT8 *tiamc1_spriteram_a;

PALETTE_INIT( tiamc1 );
VIDEO_START( tiamc1 );
VIDEO_UPDATE( tiamc1 );

WRITE8_HANDLER( tiamc1_palette_w );
WRITE8_HANDLER( tiamc1_videoram_w );
WRITE8_HANDLER( tiamc1_bankswitch_w );
WRITE8_HANDLER( tiamc1_sprite_x_w );
WRITE8_HANDLER( tiamc1_sprite_y_w );
WRITE8_HANDLER( tiamc1_sprite_a_w );
WRITE8_HANDLER( tiamc1_sprite_n_w );
WRITE8_HANDLER( tiamc1_bg_vshift_w );
WRITE8_HANDLER( tiamc1_bg_hshift_w );
