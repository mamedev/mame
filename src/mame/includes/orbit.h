/*************************************************************************

    Atari Orbit hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define ORBIT_NOTE_FREQ		NODE_01
#define ORBIT_ANOTE1_AMP	NODE_02
#define ORBIT_ANOTE2_AMP	NODE_03
#define ORBIT_NOISE1_AMP	NODE_04
#define ORBIT_NOISE2_AMP	NODE_05
#define ORBIT_WARNING_EN	NODE_06
#define ORBIT_NOISE_EN		NODE_07


/*----------- defined in audio/orbit.c -----------*/

WRITE8_HANDLER( orbit_note_w );
WRITE8_HANDLER( orbit_note_amp_w );
WRITE8_HANDLER( orbit_noise_amp_w );
WRITE8_HANDLER( orbit_noise_rst_w );

DISCRETE_SOUND_EXTERN( orbit );

/*----------- defined in video/orbit.c -----------*/

VIDEO_START( orbit );
VIDEO_UPDATE( orbit );

extern UINT8* orbit_playfield_ram;
extern UINT8* orbit_sprite_ram;

extern WRITE8_HANDLER( orbit_playfield_w );
