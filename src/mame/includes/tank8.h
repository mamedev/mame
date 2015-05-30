// license:???
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari tank8 hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define TANK8_CRASH_EN          NODE_01
#define TANK8_BUGLE_EN          NODE_02
#define TANK8_MOTOR1_EN         NODE_03
#define TANK8_MOTOR2_EN         NODE_04
#define TANK8_MOTOR3_EN         NODE_05
#define TANK8_MOTOR4_EN         NODE_06
#define TANK8_MOTOR5_EN         NODE_07
#define TANK8_MOTOR6_EN         NODE_08
#define TANK8_MOTOR7_EN         NODE_09
#define TANK8_MOTOR8_EN         NODE_10
#define TANK8_EXPLOSION_EN      NODE_11
#define TANK8_ATTRACT_EN        NODE_12
#define TANK8_BUGLE_DATA1       NODE_13
#define TANK8_BUGLE_DATA2       NODE_14


class tank8_state : public driver_device
{
public:
	enum
	{
		TIMER_COLLISION
	};

	tank8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_team(*this, "team"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

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
	DECLARE_PALETTE_INIT(tank8);
	UINT32 screen_update_tank8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tank8(screen_device &screen, bool state);
	void set_pens();
	inline int get_x_pos(int n);
	inline int get_y_pos(int n);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tank8_set_collision(int index);
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

/*----------- defined in audio/tank8.c -----------*/

DISCRETE_SOUND_EXTERN( tank8 );
