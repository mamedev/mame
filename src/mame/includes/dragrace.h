/*************************************************************************

    Atari Drag Race hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define DRAGRACE_SCREECH1_EN    NODE_01
#define DRAGRACE_SCREECH2_EN    NODE_02
#define DRAGRACE_LOTONE_EN      NODE_03
#define DRAGRACE_HITONE_EN      NODE_04
#define DRAGRACE_EXPLODE1_EN    NODE_05
#define DRAGRACE_EXPLODE2_EN    NODE_06
#define DRAGRACE_MOTOR1_DATA    NODE_07
#define DRAGRACE_MOTOR2_DATA    NODE_08
#define DRAGRACE_MOTOR1_EN      NODE_80
#define DRAGRACE_MOTOR2_EN      NODE_81
#define DRAGRACE_KLEXPL1_EN     NODE_82
#define DRAGRACE_KLEXPL2_EN     NODE_83
#define DRAGRACE_ATTRACT_EN     NODE_09


class dragrace_state : public driver_device
{
public:
	dragrace_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  playfield_ram;
	UINT8 *  position_ram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	unsigned  misc_flags;
	int       gear[2];

	/* devices */
	running_device *discrete;
};


/*----------- defined in audio/dragrace.c -----------*/

DISCRETE_SOUND_EXTERN( dragrace );

/*----------- defined in video/dragrace.c -----------*/

VIDEO_START( dragrace );
VIDEO_UPDATE( dragrace );
