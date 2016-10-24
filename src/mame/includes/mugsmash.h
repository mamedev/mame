// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/gen_latch.h"

class mugsmash_state : public driver_device
{
public:
	mugsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_regs1(*this, "regs1"),
		m_regs2(*this, "regs2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_regs1;
	required_shared_ptr<uint16_t> m_regs2;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	void mugsmash_reg2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mugsmash_videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mugsmash_videoram2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mugsmash_reg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_mugsmash_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_mugsmash_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update_mugsmash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
