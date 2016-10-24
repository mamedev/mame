// license:BSD-3-Clause
// copyright-holders:Roberto Fresca

#include "machine/watchdog.h"

class ampoker2_state : public driver_device
{
public:
	ampoker2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<uint8_t> m_videoram;
	tilemap_t *m_bg_tilemap;
	void ampoker2_port30_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port31_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port32_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port33_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port34_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port35_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_port36_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_watchdog_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ampoker2_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_rabbitpk();
	void init_piccolop();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void s2k_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void video_start() override;
	void palette_init_ampoker2(palette_device &palette);
	void video_start_sigma2k();
	uint32_t screen_update_ampoker2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
};
