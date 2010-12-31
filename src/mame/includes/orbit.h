/*************************************************************************

    Atari Orbit hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define ORBIT_NOTE_FREQ       NODE_01
#define ORBIT_ANOTE1_AMP      NODE_02
#define ORBIT_ANOTE2_AMP      NODE_03
#define ORBIT_NOISE1_AMP      NODE_04
#define ORBIT_NOISE2_AMP      NODE_05
#define ORBIT_WARNING_EN      NODE_06
#define ORBIT_NOISE_EN        NODE_07

class orbit_state : public driver_device
{
public:
	orbit_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    playfield_ram;
	UINT8 *    sprite_ram;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int        flip_screen;

	/* misc */
	UINT8      misc_flags;

	/* devices */
	device_t *maincpu;
	device_t *discrete;
};


/*----------- defined in audio/orbit.c -----------*/

WRITE8_DEVICE_HANDLER( orbit_note_w );
WRITE8_DEVICE_HANDLER( orbit_note_amp_w );
WRITE8_DEVICE_HANDLER( orbit_noise_amp_w );
WRITE8_DEVICE_HANDLER( orbit_noise_rst_w );

DISCRETE_SOUND_EXTERN( orbit );

/*----------- defined in video/orbit.c -----------*/

VIDEO_START( orbit );
VIDEO_UPDATE( orbit );

extern WRITE8_HANDLER( orbit_playfield_w );
