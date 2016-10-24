// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino
/*************************************************************************

    Taito Grand Champ hardware

*************************************************************************/

#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

class grchamp_state : public driver_device
{
public:
	grchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_radarram(*this, "radarram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_leftram(*this, "leftram"),
		m_rightram(*this, "rightram"),
		m_centerram(*this, "centerram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_radarram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_leftram;
	required_shared_ptr<uint8_t> m_rightram;
	required_shared_ptr<uint8_t> m_centerram;

	uint8_t       m_cpu0_out[16];
	uint8_t       m_cpu1_out[16];

	uint8_t       m_comm_latch;
	uint8_t       m_comm_latch2[4];

	uint16_t      m_ledlatch;
	uint8_t       m_ledaddr;
	uint16_t      m_ledram[8];

	uint16_t      m_collide;
	uint8_t       m_collmode;

	bitmap_ind16 m_work_bitmap;
	tilemap_t * m_text_tilemap;
	tilemap_t * m_left_tilemap;
	tilemap_t * m_center_tilemap;
	tilemap_t * m_right_tilemap;

	void cpu0_outputs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void led_board_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu1_outputs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pc3259_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc3259_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc3259_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pc3259_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sub_to_main_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void main_to_sub_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t main_to_sub_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t get_pc3259_bits(int offs);
	void left_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void center_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void right_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portA_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portB_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portA_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portB_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_left_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_right_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_center_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index get_memory_offset(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void palette_init_grchamp(palette_device &palette);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void cpu0_interrupt(device_t &device);
	void cpu1_interrupt(device_t &device);
	void main_to_sub_comm_sync_w(void *ptr, int32_t param);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_objects(int y, uint8_t *objdata);
};

/* Discrete Sound Input Nodes */
#define GRCHAMP_ENGINE_CS_EN                NODE_01
#define GRCHAMP_SIFT_DATA                   NODE_02
#define GRCHAMP_ATTACK_UP_DATA              NODE_03
#define GRCHAMP_IDLING_EN                   NODE_04
#define GRCHAMP_FOG_EN                      NODE_05
#define GRCHAMP_PLAYER_SPEED_DATA           NODE_06
#define GRCHAMP_ATTACK_SPEED_DATA           NODE_07
#define GRCHAMP_A_DATA                      NODE_08
#define GRCHAMP_B_DATA                      NODE_09

/*----------- defined in audio/grchamp.c -----------*/

DISCRETE_SOUND_EXTERN( grchamp );
