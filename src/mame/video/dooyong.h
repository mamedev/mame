// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_VIDEO_DOOYONG_H
#define MAME_VIDEO_DOOYONG_H

#pragma once

#include "video/bufsprite.h"

#define MCFG_DOOYONG_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset) \
		MCFG_DEVICE_ADD(tag, DOOYONG_ROM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_tilerom_tag(*device, "^" rom); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num)); \
		dooyong_rom_tilemap_device::static_set_tilerom_offset(*device, (offset));
#define MCFG_DOOYONG_ROM_TILEMAP_TRANSPARENT_PEN(pen) \
		dooyong_rom_tilemap_device::static_set_transparent_pen(*device, (pen));
#define MCFG_DOOYONG_ROM_TILEMAP_PRIMELLA_CODE_BITS(bits) \
		dooyong_rom_tilemap_device::static_set_primella_code_bits(*device, (bits));

#define MCFG_RSHARK_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset, rom2, offset2) \
		MCFG_DEVICE_ADD(tag, RSHARK_ROM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_tilerom_tag(*device, "^" rom); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num)); \
		dooyong_rom_tilemap_device::static_set_tilerom_offset(*device, (offset)); \
		dooyong_rom_tilemap_device::static_set_primella_code_bits(*device, 13); \
		rshark_rom_tilemap_device::static_set_colorrom_tag(*device, "^" rom2); \
		rshark_rom_tilemap_device::static_set_colorrom_offset(*device, (offset2));

#define MCFG_DOOYONG_RAM_TILEMAP_ADD(tag, gfx, num) \
		MCFG_DEVICE_ADD(tag, DOOYONG_RAM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num));


DECLARE_DEVICE_TYPE(DOOYONG_ROM_TILEMAP, dooyong_rom_tilemap_device)
DECLARE_DEVICE_TYPE(RSHARK_ROM_TILEMAP,  rshark_rom_tilemap_device)
DECLARE_DEVICE_TYPE(DOOYONG_RAM_TILEMAP, dooyong_ram_tilemap_device)


class dooyong_tilemap_device_base : public device_t
{
public:
	static void static_set_gfxdecode_tag(device_t &device, char const *tag);
	static void static_set_gfxnum(device_t &device, int gfxnum);

	void draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, uint32_t flags, uint8_t priority);

	void set_palette_bank(uint16_t bank);

protected:
	dooyong_tilemap_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock);

	gfx_element const &gfx() const { return *m_gfxdecode->gfx(m_gfxnum); }

	required_device<gfxdecode_device> m_gfxdecode;
	int m_gfxnum;

	tilemap_t *m_tilemap;
	uint16_t m_palette_bank;
};

class dooyong_rom_tilemap_device : public dooyong_tilemap_device_base
{
public:
	dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static void static_set_tilerom_tag(device_t &device, char const *tag);
	static void static_set_tilerom_offset(device_t &device, int offset);
	static void static_set_transparent_pen(device_t &device, unsigned pen);
	static void static_set_primella_code_bits(device_t &device, unsigned bits);

	DECLARE_WRITE8_MEMBER(ctrl_w);

protected:
	dooyong_rom_tilemap_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock);

	virtual void device_start() override;

	virtual TILE_GET_INFO_MEMBER(tile_info);

	tilemap_memory_index adjust_tile_index(tilemap_memory_index tile_index) const
	{ return tile_index + ((unsigned(m_registers[1]) * 256U / gfx().width()) * m_rows); }

	int m_rows;

private:
	required_region_ptr<uint16_t> m_tilerom;
	int m_tilerom_offset;
	unsigned m_transparent_pen;
	unsigned m_primella_code_mask;
	unsigned m_primella_color_mask;
	unsigned m_primella_color_shift;

	uint8_t m_registers[0x10];
};

class rshark_rom_tilemap_device : public dooyong_rom_tilemap_device
{
public:
	rshark_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static void static_set_colorrom_tag(device_t &device, char const *tag);
	static void static_set_colorrom_offset(device_t &device, int offset);

protected:
	virtual void device_start() override;

	virtual TILE_GET_INFO_MEMBER(tile_info) override;

private:
	required_region_ptr<uint8_t> m_colorrom;
	int m_colorrom_offset;
};

class dooyong_ram_tilemap_device : public dooyong_tilemap_device_base
{
public:
	dooyong_ram_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(tileram_r) { return m_tileram[offset & ((64U * 32U) - 1)]; }
	DECLARE_WRITE16_MEMBER(tileram_w);
	void set_scrolly(int value) { m_tilemap->set_scrolly(value); }

protected:
	virtual void device_start() override;

private:
	TILE_GET_INFO_MEMBER(tile_info);

	std::unique_ptr<uint16_t[]> m_tileram;
};

#endif // MAME_VIDEO_DOOYONG_H
