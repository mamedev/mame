// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "video/decospr.h"

class esd16_state : public driver_device
{
public:
	esd16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scroll_0(*this, "scroll_0"),
		m_scroll_1(*this, "scroll_1"),
		m_spriteram(*this, "spriteram"),
		m_head_layersize(*this, "head_layersize"),
		m_headpanic_platform_x(*this, "platform_x"),
		m_headpanic_platform_y(*this, "platform_y"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprgen(*this, "spritegen"),
		m_eeprom(*this, "eeprom"),
		m_soundlatch(*this, "soundlatch")
		{}

	/* memory pointers */
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_scroll_0;
	required_shared_ptr<uint16_t> m_scroll_1;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_head_layersize;
	required_shared_ptr<uint16_t> m_headpanic_platform_x;
	required_shared_ptr<uint16_t> m_headpanic_platform_y;

	/* video-related */
	tilemap_t       *m_tilemap_0_16x16;
	tilemap_t       *m_tilemap_1_16x16;
	tilemap_t       *m_tilemap_0;
	tilemap_t       *m_tilemap_1;
	int           m_tilemap0_color;
	int           m_tilemap1_color;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<decospr_device> m_sprgen;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<generic_latch_8_device> m_soundlatch;

	void esd16_sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hedpanic_platform_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t esd_eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void esd_eeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void esd16_sound_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t esd16_sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void esd16_vram_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void esd16_vram_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void esd16_tilemap0_color_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void esd16_tilemap0_color_jumppop_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void get_tile_info_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_0_16x16(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_1_16x16(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_hedpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECOSPR_PRIORITY_CB_MEMBER(hedpanic_pri_callback);
};
