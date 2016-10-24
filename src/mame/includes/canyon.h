// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Canyon Bomber hardware

*************************************************************************/

#include "machine/watchdog.h"
#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define CANYON_MOTOR1_DATA      NODE_01
#define CANYON_MOTOR2_DATA      NODE_02
#define CANYON_EXPLODE_DATA     NODE_03
#define CANYON_WHISTLE1_EN      NODE_04
#define CANYON_WHISTLE2_EN      NODE_05
#define CANYON_ATTRACT1_EN      NODE_06
#define CANYON_ATTRACT2_EN      NODE_07



class canyon_state : public driver_device
{
public:
	canyon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	required_device<discrete_device> m_discrete;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	uint8_t canyon_switches_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t canyon_options_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void canyon_led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void canyon_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_canyon(palette_device &palette);
	uint32_t screen_update_canyon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void canyon_motor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void canyon_explode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void canyon_attract_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void canyon_whistle_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_bombs( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*----------- defined in audio/canyon.c -----------*/
DISCRETE_SOUND_EXTERN( canyon );
