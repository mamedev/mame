#include "devlegcy.h"

class tiamc1_state : public driver_device
{
public:
	tiamc1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *tileram;
	UINT8 *charram;
	UINT8 *spriteram_x;
	UINT8 *spriteram_y;
	UINT8 *spriteram_a;
	UINT8 *spriteram_n;
	UINT8 layers_ctrl;
	UINT8 bg_vshift;
	UINT8 bg_hshift;
	tilemap_t *bg_tilemap1;
	tilemap_t *bg_tilemap2;
	rgb_t *palette;
};


/*----------- defined in audio/tiamc1.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(TIAMC1, tiamc1_sound);

WRITE8_DEVICE_HANDLER( tiamc1_timer0_w );
WRITE8_DEVICE_HANDLER( tiamc1_timer1_w );
WRITE8_DEVICE_HANDLER( tiamc1_timer1_gate_w );


/*----------- defined in video/tiamc1.c -----------*/

PALETTE_INIT( tiamc1 );
VIDEO_START( tiamc1 );
SCREEN_UPDATE( tiamc1 );

WRITE8_HANDLER( tiamc1_palette_w );
WRITE8_HANDLER( tiamc1_videoram_w );
WRITE8_HANDLER( tiamc1_bankswitch_w );
WRITE8_HANDLER( tiamc1_sprite_x_w );
WRITE8_HANDLER( tiamc1_sprite_y_w );
WRITE8_HANDLER( tiamc1_sprite_a_w );
WRITE8_HANDLER( tiamc1_sprite_n_w );
WRITE8_HANDLER( tiamc1_bg_vshift_w );
WRITE8_HANDLER( tiamc1_bg_hshift_w );
