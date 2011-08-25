/*************************************************************************

    Atari tank8 hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define TANK8_CRASH_EN			NODE_01
#define TANK8_BUGLE_EN			NODE_02
#define TANK8_MOTOR1_EN			NODE_03
#define TANK8_MOTOR2_EN			NODE_04
#define TANK8_MOTOR3_EN			NODE_05
#define TANK8_MOTOR4_EN			NODE_06
#define TANK8_MOTOR5_EN			NODE_07
#define TANK8_MOTOR6_EN			NODE_08
#define TANK8_MOTOR7_EN			NODE_09
#define TANK8_MOTOR8_EN			NODE_10
#define TANK8_EXPLOSION_EN		NODE_11
#define TANK8_ATTRACT_EN		NODE_12
#define TANK8_BUGLE_DATA1		NODE_13
#define TANK8_BUGLE_DATA2		NODE_14


class tank8_state : public driver_device
{
public:
	tank8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_collision_index;
	UINT8 *m_video_ram;
	UINT8 *m_pos_h_ram;
	UINT8 *m_pos_v_ram;
	UINT8 *m_pos_d_ram;
	UINT8 *m_team;
	tilemap_t *m_tilemap;
	bitmap_t *m_helper1;
	bitmap_t *m_helper2;
	bitmap_t *m_helper3;
};


/*----------- defined in audio/tank8.c -----------*/

DISCRETE_SOUND_EXTERN( tank8 );


/*----------- defined in drivers/tank8.c -----------*/

void tank8_set_collision(running_machine &machine, int index);


/*----------- defined in video/tank8.c -----------*/

PALETTE_INIT( tank8 );
SCREEN_EOF( tank8 );
VIDEO_START( tank8 );
SCREEN_UPDATE( tank8 );

WRITE8_HANDLER( tank8_video_ram_w );


