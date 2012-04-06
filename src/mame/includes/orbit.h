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
	orbit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_playfield_ram;
	UINT8 *    m_sprite_ram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int        m_flip_screen;

	/* misc */
	UINT8      m_misc_flags;

	/* devices */
	device_t *m_maincpu;
	device_t *m_discrete;
	DECLARE_WRITE8_MEMBER(orbit_misc_w);
	DECLARE_WRITE8_MEMBER(orbit_playfield_w);
};


/*----------- defined in audio/orbit.c -----------*/

WRITE8_DEVICE_HANDLER( orbit_note_w );
WRITE8_DEVICE_HANDLER( orbit_note_amp_w );
WRITE8_DEVICE_HANDLER( orbit_noise_amp_w );
WRITE8_DEVICE_HANDLER( orbit_noise_rst_w );

DISCRETE_SOUND_EXTERN( orbit );

/*----------- defined in video/orbit.c -----------*/

VIDEO_START( orbit );
SCREEN_UPDATE_IND16( orbit );

