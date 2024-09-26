// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/
#ifndef MAME_MISC_ESD16_H
#define MAME_MISC_ESD16_H

#pragma once

#include "decospr.h"

#include "machine/eepromser.h"
#include "machine/gen_latch.h"

#include "tilemap.h"


class esd16_state : public driver_device
{
public:
	esd16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram_%u", 0U),
		m_scroll(*this, "scroll_%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_layersize(*this, "head_layersize"),
		m_platform_x(*this, "platform_x"),
		m_platform_y(*this, "platform_y"),
		m_audiobank(*this, "audiobank"),
		m_io_eepromout(*this, "EEPROMOUT"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprgen(*this, "spritegen"),
		m_eeprom(*this, "eeprom"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void jumppop(machine_config &config);
	void esd16_nosound(machine_config &config);
	void esd16(machine_config &config);
	void tangtang(machine_config &config);
	void mchampdx(machine_config &config);
	void hedpanio(machine_config &config);
	void hedpanic(machine_config &config);
	void fantstry(machine_config &config);
	void fantstrya(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<u16, 2> m_vram;
	required_shared_ptr_array<u16, 2> m_scroll;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_layersize;
	required_shared_ptr<u16> m_platform_x;
	required_shared_ptr<u16> m_platform_y;

	optional_memory_bank m_audiobank;
	optional_ioport m_io_eepromout;

	// video-related
	tilemap_t       *m_tilemap_16x16[2]{};
	tilemap_t       *m_tilemap[2]{};
	u32             m_tilemap_color[2]{};

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<decospr_device> m_sprgen;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<generic_latch_8_device> m_soundlatch;

	void sound_command_w(u8 data);
	void platform_w(u16 data);
	u8 eeprom_r();
	void eeprom_w(u8 data);
	void sound_rombank_w(u8 data);
	template<unsigned Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tilemap0_color_w(u16 data);
	void tilemap0_color_jumppop_w(u16 data);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info_16x16);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);
	void fantstrya_map(address_map &map) ATTR_COLD;
	void hedpanic_map(address_map &map) ATTR_COLD;
	void jumppop_map(address_map &map) ATTR_COLD;
	void mchampdx_map(address_map &map) ATTR_COLD;
	void multchmp_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void tangtang_map(address_map &map) ATTR_COLD;

	void io_area_dsw(address_map &map, u32 base) ATTR_COLD;
	void io_area_eeprom(address_map &map, u32 base) ATTR_COLD;
	void palette_area(address_map &map, u32 base) ATTR_COLD;
	void sprite_area(address_map &map, u32 base) ATTR_COLD;
	void vid_attr_area(address_map &map, u32 base) ATTR_COLD;
	void vram_area(address_map &map, u32 base) ATTR_COLD;
};

#endif // MAME_MISC_ESD16_H
