// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/*************************************************************************

    Atari Skydiver hardware

*************************************************************************/

#include "machine/watchdog.h"
#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SKYDIVER_RANGE_DATA     NODE_01
#define SKYDIVER_NOTE_DATA      NODE_02
#define SKYDIVER_RANGE3_EN      NODE_03
#define SKYDIVER_NOISE_DATA     NODE_04
#define SKYDIVER_NOISE_RST      NODE_05
#define SKYDIVER_WHISTLE1_EN    NODE_06
#define SKYDIVER_WHISTLE2_EN    NODE_07
#define SKYDIVER_OCT1_EN        NODE_08
#define SKYDIVER_OCT2_EN        NODE_09
#define SKYDIVER_SOUND_EN       NODE_10


class skydiver_state : public driver_device
{
public:
	skydiver_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;

	int m_nmion;
	tilemap_t *m_bg_tilemap;
	int m_width;

	void nmion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t wram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void wram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void width_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coin_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void start_lamp_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void start_lamp_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_s_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_k_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_d_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _2000_201F_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void whistle_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_skydiver(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt(device_t &device);
};

/*----------- defined in audio/skydiver.c -----------*/
DISCRETE_SOUND_EXTERN( skydiver );
