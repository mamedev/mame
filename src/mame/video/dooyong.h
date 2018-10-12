// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_VIDEO_DOOYONG_H
#define MAME_VIDEO_DOOYONG_H

#pragma once

#include "video/bufsprite.h"

#define MCFG_DOOYONG_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset) \
		MCFG_DEVICE_ADD(tag, DOOYONG_ROM_TILEMAP, 0) \
		downcast<dooyong_rom_tilemap_device &>(*device).set_gfxdecode_tag(gfx); \
		downcast<dooyong_rom_tilemap_device &>(*device).set_tilerom_tag(rom); \
		downcast<dooyong_rom_tilemap_device &>(*device).set_gfxnum((num)); \
		downcast<dooyong_rom_tilemap_device &>(*device).set_tilerom_offset((offset));
#define MCFG_DOOYONG_ROM_TILEMAP_TRANSPARENT_PEN(pen) \
		downcast<dooyong_rom_tilemap_device &>(*device).set_transparent_pen((pen));
#define MCFG_DOOYONG_ROM_TILEMAP_PRIMELLA_CODE_BITS(bits) \
		downcast<dooyong_rom_tilemap_device &>(*device).set_primella_code_bits((bits));

#define MCFG_RSHARK_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset, rom2, offset2) \
		MCFG_DEVICE_ADD(tag, RSHARK_ROM_TILEMAP, 0) \
		downcast<rshark_rom_tilemap_device &>(*device).set_gfxdecode_tag(gfx); \
		downcast<rshark_rom_tilemap_device &>(*device).set_tilerom_tag(rom); \
		downcast<rshark_rom_tilemap_device &>(*device).set_gfxnum((num)); \
		downcast<rshark_rom_tilemap_device &>(*device).set_tilerom_offset((offset)); \
		downcast<rshark_rom_tilemap_device &>(*device).set_primella_code_bits(13); \
		downcast<rshark_rom_tilemap_device &>(*device).set_colorrom_tag(rom2); \
		downcast<rshark_rom_tilemap_device &>(*device).set_colorrom_offset((offset2));

#define MCFG_DOOYONG_RAM_TILEMAP_ADD(tag, gfx, num) \
		MCFG_DEVICE_ADD(tag, DOOYONG_RAM_TILEMAP, 0) \
		downcast<dooyong_ram_tilemap_device &>(*device).set_gfxdecode_tag(gfx); \
		downcast<dooyong_ram_tilemap_device &>(*device).set_gfxnum((num));


DECLARE_DEVICE_TYPE(DOOYONG_ROM_TILEMAP, dooyong_rom_tilemap_device)
DECLARE_DEVICE_TYPE(RSHARK_ROM_TILEMAP,  rshark_rom_tilemap_device)
DECLARE_DEVICE_TYPE(DOOYONG_RAM_TILEMAP, dooyong_ram_tilemap_device)


class dooyong_tilemap_device_base : public device_t
{
public:
	void set_gfxdecode_tag(char const *tag) { m_gfxdecode.set_tag(tag); }
	void set_gfxnum(int gfxnum) { m_gfxnum = gfxnum; }

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

	void set_tilerom_tag(char const *tag) { m_tilerom.set_tag(tag); }
	void set_tilerom_offset(int offset) { m_tilerom_offset = offset; }
	void set_transparent_pen(unsigned pen) { m_transparent_pen = pen; }
	void set_primella_code_bits(unsigned bits)
	{
		m_primella_code_mask = (1U << bits) - 1U;
		m_primella_color_mask = ((1U << 14) - 1) & ~m_primella_code_mask;
		m_primella_color_shift = bits;
	}

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

	void set_colorrom_tag(char const *tag) { m_colorrom.set_tag(tag); }
	void set_colorrom_offset(int offset) { m_colorrom_offset = offset; }

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
