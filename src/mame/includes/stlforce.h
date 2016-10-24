// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "sound/okim6295.h"
#include "machine/eepromser.h"

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mlow_videoram(*this, "mlow_videoram"),
		m_mhigh_videoram(*this, "mhigh_videoram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_bg_scrollram(*this, "bg_scrollram"),
		m_mlow_scrollram(*this, "mlow_scrollram"),
		m_mhigh_scrollram(*this, "mhigh_scrollram"),
		m_vidattrram(*this, "vidattrram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_mlow_videoram;
	required_shared_ptr<uint16_t> m_mhigh_videoram;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr<uint16_t> m_bg_scrollram;
	required_shared_ptr<uint16_t> m_mlow_scrollram;
	required_shared_ptr<uint16_t> m_mhigh_scrollram;
	required_shared_ptr<uint16_t> m_vidattrram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	int m_sprxoffs;

	void stlforce_bg_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stlforce_mlow_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stlforce_mhigh_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stlforce_tx_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void oki_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void init_twinbrat();
	void init_stlforce();

	void get_stlforce_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stlforce_mlow_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stlforce_mhigh_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_stlforce_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	virtual void video_start() override;
	uint32_t screen_update_stlforce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
