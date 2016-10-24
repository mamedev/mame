// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Pool Shark hardware

*************************************************************************/

#include "machine/watchdog.h"
#include "sound/discrete.h"

/* Discrete Sound Input Nodes */

class poolshrk_state : public driver_device
{
public:
	poolshrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_hpos_ram;
	required_shared_ptr<uint8_t> m_vpos_ram;

	tilemap_t* m_bg_tilemap;
	int m_da_latch;

	void da_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void watchdog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t irq_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scratch_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void score_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void click_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bump_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_poolshrk();
	virtual void video_start() override;
	void palette_init_poolshrk(palette_device &palette);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in audio/poolshrk.c -----------*/
DISCRETE_SOUND_EXTERN( poolshrk );
