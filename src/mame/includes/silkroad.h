// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
#include "sound/okim6295.h"

class silkroad_state : public driver_device
{
public:
	silkroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki1(*this, "oki1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_vidram(*this, "vidram"),
		m_vidram2(*this, "vidram2"),
		m_vidram3(*this, "vidram3"),
		m_sprram(*this, "sprram"),
		m_regs(*this, "regs") { }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint32_t> m_generic_paletteram_32;
	required_shared_ptr<uint32_t> m_vidram;
	required_shared_ptr<uint32_t> m_vidram2;
	required_shared_ptr<uint32_t> m_vidram3;
	required_shared_ptr<uint32_t> m_sprram;
	required_shared_ptr<uint32_t> m_regs;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_fg3_tilemap;

	void paletteram32_xRRRRRGGGGGBBBBB_dword_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void silk_coin_counter_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void silkroad_fgram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void silkroad_fgram2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void silkroad_fgram3_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void silk_6295_bank_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_fg3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;

	uint32_t screen_update_silkroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
