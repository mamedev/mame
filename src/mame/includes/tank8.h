// license:BSD-3-Clause
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
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_team(*this, "team") { }

	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_pos_h_ram;
	required_shared_ptr<uint8_t> m_pos_v_ram;
	required_shared_ptr<uint8_t> m_pos_d_ram;
	required_shared_ptr<uint8_t> m_team;

	int m_collision_index;
	tilemap_t *m_tilemap;
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	bitmap_ind16 m_helper3;
	emu_timer *m_collision_timer;

	uint8_t collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void int_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void explosion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bugle_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bug_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void attract_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void motor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_decode();
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_tank8(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void set_pens();
	inline int get_x_pos(int n);
	inline int get_y_pos(int n);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_collision(int index);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in audio/tank8.c -----------*/

DISCRETE_SOUND_EXTERN( tank8 );
