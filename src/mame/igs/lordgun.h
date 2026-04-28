// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

                      -= IGS Lord Of Gun =-

*************************************************************************/
#ifndef MAME_IGS_LORDGUN_H
#define MAME_IGS_LORDGUN_H

#pragma once

#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class lordgun_base_state : public driver_device
{
protected:
	lordgun_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki%u", 1U),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_priority_ram(*this, "priority_ram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram.%u", 0),
		m_scroll_x(*this, "scroll_x.%u", 0),
		m_scroll_y(*this, "scroll_y.%u", 0)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fake_w(uint8_t data);
	void fake2_w(uint8_t data);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void common_map(address_map &map) ATTR_COLD;
	void soundmem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device_array<okim6295_device, 2> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	required_shared_ptr<uint16_t> m_priority_ram;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr_array<uint16_t, 4> m_scroll_x;
	required_shared_ptr_array<uint16_t, 4> m_scroll_y;

	uint16_t m_priority = 0U;
	bool m_whitescreen = false;
	tilemap_t *m_tilemap[4]{};
	bitmap_ind16 m_bitmaps[5];

	uint16_t m_protection_data = 0U;
};

// with multiplexed DIP switch
class aliencha_state : public lordgun_base_state
{
public:
	aliencha_state(const machine_config &mconfig, device_type type, const char *tag) :
		lordgun_base_state(mconfig, type, tag),
		m_in_dip(*this, "DIP%u", 1U)
	{ }

	void aliencha(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void aliencha_protection_w(offs_t offset, uint16_t data);
	uint16_t aliencha_protection_r(offs_t offset);

	void aliencha_eeprom_w(uint8_t data);
	uint8_t dip_r();
	void dip_w(uint8_t data);

	void aliencha_map(address_map &map) ATTR_COLD;
	void aliencha_soundio_map(address_map &map) ATTR_COLD;
	void ymf278_map(address_map &map) ATTR_COLD;

	required_ioport_array<3> m_in_dip;

	uint8_t m_dip_sel = 0U;
};

// with lightguns
class lordgun_state : public lordgun_base_state
{
public:
	lordgun_state(const machine_config &mconfig, device_type type, const char *tag) :
		lordgun_base_state(mconfig, type, tag),
		m_in_lightgun_x(*this, "LIGHT%u_X", 0U),
		m_in_lightgun_y(*this, "LIGHT%u_Y", 0U)
	{ }

	void lordgun(machine_config &config) ATTR_COLD;

	void init_lordgun() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	struct lordgun_gun_data
	{
		int32_t scr_x = 0, scr_y = 0;
		uint16_t hw_x = 0U, hw_y = 0U;
	};

	void lordgun_protection_w(offs_t offset, uint16_t data);
	uint16_t lordgun_protection_r(offs_t offset);

	template <unsigned Which> uint16_t gun_x_r();
	template <unsigned Which> uint16_t gun_y_r();
	void lordgun_eeprom_w(uint8_t data);
	void okibank_w(uint8_t data);

	void calc_gun_scr(int i);
	void update_gun(int i);
	void lordgun_map(address_map &map) ATTR_COLD;
	void lordgun_soundio_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_in_lightgun_x;
	required_ioport_array<2> m_in_lightgun_y;

	lordgun_gun_data m_gun[2];
	uint8_t m_old = 0U;
};

#endif // MAME_IGS_LORDGUN_H
