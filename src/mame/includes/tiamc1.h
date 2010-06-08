#include "devlegcy.h"

/*----------- defined in audio/tiamc1.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(TIAMC1, tiamc1_sound);

WRITE8_HANDLER( tiamc1_timer0_w );
WRITE8_HANDLER( tiamc1_timer1_w );
WRITE8_HANDLER( tiamc1_timer1_gate_w );


/*----------- defined in video/tiamc1.c -----------*/

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
