#include "devlegcy.h"

class tiamc1_state : public driver_device
{
public:
	tiamc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_tileram;
	UINT8 *m_charram;
	UINT8 *m_spriteram_x;
	UINT8 *m_spriteram_y;
	UINT8 *m_spriteram_a;
	UINT8 *m_spriteram_n;
	UINT8 m_layers_ctrl;
	UINT8 m_bg_vshift;
	UINT8 m_bg_hshift;
	tilemap_t *m_bg_tilemap1;
	tilemap_t *m_bg_tilemap2;
	rgb_t *m_palette;
	DECLARE_WRITE8_MEMBER(tiamc1_control_w);
	DECLARE_WRITE8_MEMBER(tiamc1_videoram_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_x_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_y_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_a_w);
	DECLARE_WRITE8_MEMBER(tiamc1_sprite_n_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_vshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_bg_hshift_w);
	DECLARE_WRITE8_MEMBER(tiamc1_palette_w);
};


/*----------- defined in audio/tiamc1.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(TIAMC1, tiamc1_sound);

WRITE8_DEVICE_HANDLER( tiamc1_timer0_w );
WRITE8_DEVICE_HANDLER( tiamc1_timer1_w );
WRITE8_DEVICE_HANDLER( tiamc1_timer1_gate_w );


/*----------- defined in video/tiamc1.c -----------*/

PALETTE_INIT( tiamc1 );
VIDEO_START( tiamc1 );
SCREEN_UPDATE_IND16( tiamc1 );

