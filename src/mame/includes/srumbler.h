// license:BSD-3-Clause
// copyright-holders:Paul Leaman
#include "video/bufsprite.h"

class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_backgroundram(*this, "backgroundram"),
		m_foregroundram(*this, "foregroundram") { }

	required_device<cpu_device> m_maincpu;
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_backgroundram;
	required_shared_ptr<uint8_t> m_foregroundram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void foreground_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void background_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void _4009_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt(timer_device &timer, void *ptr, int32_t param);
};
