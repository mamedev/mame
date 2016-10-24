// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo driver

*************************************************************************/

#include "machine/watchdog.h"
#include "sound/discrete.h"

#define FIRETRUCK_MOTOR_DATA    NODE_01
#define FIRETRUCK_HORN_EN       NODE_02
#define FIRETRUCK_SIREN_DATA    NODE_03
#define FIRETRUCK_CRASH_DATA    NODE_04
#define FIRETRUCK_SKID_EN       NODE_05
#define FIRETRUCK_BELL_EN       NODE_06
#define FIRETRUCK_ATTRACT_EN    NODE_07
#define FIRETRUCK_XTNDPLY_EN    NODE_08

#define SUPERBUG_SPEED_DATA     FIRETRUCK_MOTOR_DATA
#define SUPERBUG_CRASH_DATA     FIRETRUCK_CRASH_DATA
#define SUPERBUG_SKID_EN        FIRETRUCK_SKID_EN
#define SUPERBUG_ASR_EN         FIRETRUCK_XTNDPLY_EN
#define SUPERBUG_ATTRACT_EN     FIRETRUCK_ATTRACT_EN

#define MONTECAR_MOTOR_DATA         FIRETRUCK_MOTOR_DATA
#define MONTECAR_CRASH_DATA         FIRETRUCK_CRASH_DATA
#define MONTECAR_DRONE_MOTOR_DATA   FIRETRUCK_SIREN_DATA
#define MONTECAR_SKID_EN            FIRETRUCK_SKID_EN
#define MONTECAR_DRONE_LOUD_DATA    FIRETRUCK_BELL_EN
#define MONTECAR_BEEPER_EN          FIRETRUCK_XTNDPLY_EN
#define MONTECAR_ATTRACT_INV        FIRETRUCK_ATTRACT_EN


class firetrk_state : public driver_device
{
public:
	firetrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_playfield_ram(*this, "playfield_ram"),
		m_scroll_y(*this, "scroll_y"),
		m_scroll_x(*this, "scroll_x"),
		m_car_rot(*this, "car_rot"),
		m_blink(*this, "blink"),
		m_drone_x(*this, "drone_x"),
		m_drone_y(*this, "drone_y"),
		m_drone_rot(*this, "drone_rot"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bit_0(*this, "BIT_0"),
		m_bit_6(*this, "BIT_6"),
		m_bit_7(*this, "BIT_7"),
		m_dips(*this, {"DIP_0", "DIP_1"}),
		m_steer(*this, {"STEER_1", "STEER_2"})
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_car_rot;
	optional_shared_ptr<uint8_t> m_blink;
	optional_shared_ptr<uint8_t> m_drone_x;
	optional_shared_ptr<uint8_t> m_drone_y;
	optional_shared_ptr<uint8_t> m_drone_rot;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_ioport m_bit_0;
	optional_ioport m_bit_6;
	optional_ioport m_bit_7;
	required_ioport_array<2> m_dips;
	optional_ioport_array<2> m_steer;

	uint8_t m_in_service_mode;
	uint32_t m_dial[2];
	uint8_t m_steer_dir[2];
	uint8_t m_steer_flag[2];
	uint8_t m_gear;

	uint8_t m_flash;
	uint8_t m_crash[2];
	uint8_t m_skid[2];
	bitmap_ind16 m_helper1;
	bitmap_ind16 m_helper2;
	uint32_t m_color1_mask;
	uint32_t m_color2_mask;
	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	void firetrk_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void superbug_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void montecar_output_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void montecar_output_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t firetrk_dip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t montecar_dip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t firetrk_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t montecar_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void blink_on_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void montecar_car_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void montecar_drone_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void steer_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crash_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value steer_dir_r(ioport_field &field, void *param);
	ioport_value steer_flag_r(ioport_field &field, void *param);
	ioport_value skid_r(ioport_field &field, void *param);
	ioport_value crash_r(ioport_field &field, void *param);
	ioport_value gear_r(ioport_field &field, void *param);
	void service_mode_switch_changed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void firetrk_horn_changed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void gear_changed(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void firetrk_get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void superbug_get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void montecar_get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void firetrk_get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void superbug_get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void montecar_get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_firetrk(palette_device &palette);
	void video_start_superbug();
	void video_start_montecar();
	void palette_init_montecar(palette_device &palette);
	uint32_t screen_update_firetrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_superbug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_montecar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void firetrk_scanline(timer_device &timer, void *ptr, int32_t param);
	void firetrk_skid_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void montecar_skid_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrk_crash_snd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrk_skid_snd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrk_motor_snd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void superbug_motor_snd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firetrk_xtndply_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prom_to_palette(int number, uint8_t val);
	void firetrk_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flash);
	void superbug_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int flash);
	void montecar_draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int is_collision_detection);
	void check_collision(int which);
	void set_service_mode(int enable);
	void draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *alpha_ram, int x, int count, int height);
};


/*----------- defined in audio/firetrk.c -----------*/

DISCRETE_SOUND_EXTERN( firetrk );
DISCRETE_SOUND_EXTERN( superbug );
DISCRETE_SOUND_EXTERN( montecar );
