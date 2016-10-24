// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#include "sound/discrete.h"

class sprint8_state : public driver_device
{
public:
	sprint8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_team(*this, "team"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	int m_steer_dir[8];
	int m_steer_flag[8];
	int m_collision_reset;
	int m_collision_index;
	uint8_t m_dial[8];
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_pos_h_ram;
	required_shared_ptr<uint8_t> m_pos_v_ram;
	required_shared_ptr<uint8_t> m_pos_d_ram;
	required_shared_ptr<uint8_t> m_team;
	required_device<discrete_device> m_discrete;
	tilemap_t* m_tilemap1;
	tilemap_t* m_tilemap2;
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	uint8_t sprint8_collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sprint8_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sprint8_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprint8_int_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprint8_video_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_sprint8(palette_device &palette);
	uint32_t screen_update_sprint8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sprint8(screen_device &screen, bool state);
	void sprint8_collision_callback(void *ptr, int32_t param);
	void input_callback(timer_device &timer, void *ptr, int32_t param);
	void sprint8_crash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprint8_screech_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprint8_attract_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sprint8_motor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void set_pens();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sprint8_set_collision(int n);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/sprint8.c -----------*/
DISCRETE_SOUND_EXTERN( sprint8 );
