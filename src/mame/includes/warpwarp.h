#include "devlegcy.h"

class warpwarp_state : public driver_device
{
public:
	warpwarp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *geebee_videoram;
	UINT8 *videoram;
	int geebee_bgw;
	int ball_on;
	int ball_h;
	int ball_v;
	int ball_pen;
	int ball_sizex;
	int ball_sizey;
	int handle_joystick;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/warpwarp.c -----------*/

PALETTE_INIT( geebee );
PALETTE_INIT( navarone );
PALETTE_INIT( warpwarp );
VIDEO_START( geebee );
VIDEO_START( navarone );
VIDEO_START( warpwarp );
SCREEN_UPDATE( geebee );
WRITE8_HANDLER( warpwarp_videoram_w );
WRITE8_HANDLER( geebee_videoram_w );


/*----------- defined in audio/geebee.c -----------*/

WRITE8_DEVICE_HANDLER( geebee_sound_w );

DECLARE_LEGACY_SOUND_DEVICE(GEEBEE, geebee_sound);


/*----------- defined in audio/warpwarp.c -----------*/

WRITE8_DEVICE_HANDLER( warpwarp_sound_w );
WRITE8_DEVICE_HANDLER( warpwarp_music1_w );
WRITE8_DEVICE_HANDLER( warpwarp_music2_w );

DECLARE_LEGACY_SOUND_DEVICE(WARPWARP, warpwarp_sound);
