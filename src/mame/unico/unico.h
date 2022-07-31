// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_UNICO_H
#define MAME_INCLUDES_UNICO_H

#pragma once

#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class unico_state : public driver_device
{
public:
	unico_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_leds(*this, "led%u", 0U),
		m_vram(*this, "vram", 0xc000, ENDIANNESS_BIG),
		m_scroll(*this, "scroll", 22, ENDIANNESS_BIG),
		m_spriteram(*this, "spriteram", 0x800, ENDIANNESS_BIG)
	{ }

	void burglarx(machine_config &config);

protected:
	static rgb_t unico_R6G6B6X(uint32_t raw);
	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scroll_r(offs_t offset);
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void burglarx_okibank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);

	void burglarx_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<okim6295_device> m_oki;
	output_finder<2> m_leds;

private:
	memory_share_creator<uint16_t> m_vram;
	memory_share_creator<uint16_t> m_scroll;
	tilemap_t *m_tilemap[3]{};
	int m_sprites_scrolldx = 0;
	int m_sprites_scrolldy = 0;
	memory_share_creator<uint16_t> m_spriteram;
};

class zeropnt_state : public unico_state
{
public:
	zeropnt_state(const machine_config &mconfig, device_type type, const char *tag) :
		unico_state(mconfig, type, tag),
		m_okibank(*this, "okibank"),
		m_screen(*this, "screen"),
		m_gun_axes(*this, { "Y0", "X0", "Y1", "X1" })
	{ }

	void zeropnt(machine_config &config);

protected:
	virtual void machine_start() override;

	void zeropnt_okibank_leds_w(uint8_t data);
	uint16_t gunx_0_msb_r();
	uint16_t guny_0_msb_r();
	uint16_t gunx_1_msb_r();
	uint16_t guny_1_msb_r();

	required_memory_bank m_okibank;

	required_device<screen_device> m_screen;

	void zeropnt_map(address_map &map);
	void zeropnt_oki_map(address_map &map);

private:
	enum { Y0, X0, Y1, X1 }; // gun axis indices

	required_ioport_array<4> m_gun_axes;
};

class zeropnt2_state : public zeropnt_state
{
public:
	zeropnt2_state(const machine_config &mconfig, device_type type, const char *tag) :
		zeropnt_state(mconfig, type, tag),
		m_eeprom(*this, "eeprom")
	{ }

	void zeropnt2(machine_config &config);

protected:
	virtual void machine_start() override;

	uint32_t zeropnt2_gunx_0_msb_r();
	uint32_t zeropnt2_guny_0_msb_r();
	uint32_t zeropnt2_gunx_1_msb_r();
	uint32_t zeropnt2_guny_1_msb_r();
	void zeropnt2_okibank(uint8_t data);
	void leds_w(uint8_t data);

	void eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void zeropnt2_map(address_map &map);

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

#endif // MAME_INCLUDES_UNICO_H
