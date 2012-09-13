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
		: driver_device(mconfig, type, tag) ,
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_team(*this, "team"){ }

	int m_collision_index;
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_pos_h_ram;
	required_shared_ptr<UINT8> m_pos_v_ram;
	required_shared_ptr<UINT8> m_pos_d_ram;
	required_shared_ptr<UINT8> m_team;
	tilemap_t *m_tilemap;
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	bitmap_ind16 m_helper3;
	DECLARE_READ8_MEMBER(tank8_collision_r);
	DECLARE_WRITE8_MEMBER(tank8_lockout_w);
	DECLARE_WRITE8_MEMBER(tank8_int_reset_w);
	DECLARE_WRITE8_MEMBER(tank8_video_ram_w);
	DECLARE_WRITE8_MEMBER(tank8_crash_w);
	DECLARE_WRITE8_MEMBER(tank8_explosion_w);
	DECLARE_WRITE8_MEMBER(tank8_bugle_w);
	DECLARE_WRITE8_MEMBER(tank8_bug_w);
	DECLARE_WRITE8_MEMBER(tank8_attract_w);
	DECLARE_WRITE8_MEMBER(tank8_motor_w);
	DECLARE_DRIVER_INIT(decode);
	TILE_GET_INFO_MEMBER(tank8_get_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in audio/tank8.c -----------*/

DISCRETE_SOUND_EXTERN( tank8 );


/*----------- defined in drivers/tank8.c -----------*/

void tank8_set_collision(running_machine &machine, int index);


/*----------- defined in video/tank8.c -----------*/


SCREEN_VBLANK( tank8 );

SCREEN_UPDATE_IND16( tank8 );



