// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "video/dooyong.h"

#include "screen.h"


/* These games all have ROM-based tilemaps for the backgrounds, title
   screens and sometimes "bosses" and special attacks. There are three
   schemes for tilemap encoding.  The scheme is chosen based on the
   contents of the tilemap control variables declared above.
   Note that although the tilemaps are arbitrarily wide (hundreds of
   thousands of pixels, depending on the size of the ROM), we only
   decode a 1024 pixel wide block at a time, and invalidate the tilemap
   when the x scroll moves out of range (trying to decode the whole lot
   at once uses hundreds of megabytes of RAM). */

DEFINE_DEVICE_TYPE(DOOYONG_ROM_TILEMAP, dooyong_rom_tilemap_device, "dooyong_rom_tilemap", "Dooyong ROM Tilemap")
DEFINE_DEVICE_TYPE(RSHARK_ROM_TILEMAP,  rshark_rom_tilemap_device,  "rshark_rom_tilemap",  "R-Shark ROM Tilemap")
DEFINE_DEVICE_TYPE(DOOYONG_RAM_TILEMAP, dooyong_ram_tilemap_device, "dooyong_ram_tilemap", "Dooyong RAM Tilemap")


dooyong_tilemap_device_base::dooyong_tilemap_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_gfxnum(0)
	, m_tilemap(nullptr)
	, m_palette_bank(0)
{
}

void dooyong_tilemap_device_base::draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, u32 flags, u8 priority, u8 priority_mask)
{
	m_tilemap->draw(screen, dest, cliprect, flags, priority, priority_mask);
}

void dooyong_tilemap_device_base::set_palette_bank(u16 bank)
{
	if (bank != m_palette_bank)
	{
		m_palette_bank = bank;
		m_tilemap->mark_all_dirty();
	}
}

dooyong_rom_tilemap_device::dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: dooyong_rom_tilemap_device(mconfig, DOOYONG_ROM_TILEMAP, tag, owner, clock)
{
}

dooyong_rom_tilemap_device::dooyong_rom_tilemap_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: dooyong_tilemap_device_base(mconfig, type, tag, owner, clock)
	, m_rows(8)
	, m_tilerom(*this, finder_base::DUMMY_TAG)
	, m_tilerom_offset(0)
	, m_tilerom_length(~0U)
	, m_transparent_pen(~0U)
	, m_registers{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
{
}

void dooyong_rom_tilemap_device::ctrl_w(offs_t offset, u8 data)
{
	offset &= 0x07U;
	const u8 old = m_registers[offset];
	if (old != data)
	{
		m_registers[offset] = data;
		switch (offset)
		{
		case 0: // Low byte of x scroll - scroll tilemap
			m_tilemap->set_scrollx(0, data);
			break;
		case 1: // High byte of x scroll - mark tilemap dirty so new tile gfx will be loaded
			m_tilemap->mark_all_dirty();
			break;
		case 3: // Low byte of y scroll
		case 4: // High byte of y scroll
			m_tilemap->set_scrolly(0, m_registers[3] | (unsigned(m_registers[4]) << 8));
			break;
		case 6: // Tilemap enable and mode control
			m_tilemap->enable(!BIT(data, 4));
			if (BIT(data ^ old, 5)) // This sets the tilemap data format
				m_tilemap->mark_all_dirty();
			break;
		default: // Other addresses are used but function is unknown
			// 0x05 and 0x07 are initialised on startup
			// 0x02 is initialised on startup by some games and written to continuously by others
			// printf("Unknown %s tilemap control: 0x%02x = 0x%02x\n", tag(), unsigned(offset), unsigned(data));
			break;
		}
	}
}

void dooyong_rom_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tmap_cb.bind_relative_to(*owner());

	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(dooyong_rom_tilemap_device::tile_info), this),
			TILEMAP_SCAN_COLS,
			gfx().width(),
			gfx().height(),
			1024 / gfx().width(),
			m_rows);
	if (~m_transparent_pen)
		m_tilemap->set_transparent_pen(m_transparent_pen);

	if (0 > m_tilerom_offset)
		m_tilerom_offset = m_tilerom.length() + m_tilerom_offset;

	if (m_tilerom_length < 0)
		m_tilerom_length = m_tilerom.length();

	std::fill(std::begin(m_registers), std::end(m_registers), 0U);
	m_palette_bank = 0U;

	save_item(NAME(m_registers));
	save_item(NAME(m_palette_bank));
}

TILE_GET_INFO_MEMBER(dooyong_rom_tilemap_device::tile_info)
{
	const int tile_index_tile = adjust_tile_index(tile_index) & (m_tilerom_length - 1);
	const u16 attr = m_tilerom[m_tilerom_offset + tile_index_tile];
	u32 code = 0, color = 0, flags;
	if (BIT(m_registers[6], 5))
	{   // lastday/gulfstrm/pollux/flytiger
		// Tiles take one word in ROM:
		// MSB             LSB
		// cCCC CYXc cccc cccc  (bit 9 of gfx code, bits 3-0 of color, Y flip, X flip, bits 8-0 of gfx code)
		// c = gfx code
		// C = color code
		// X = x flip
		// Y = y flip
		code = (BIT(attr, 15) << 9) | (attr & 0x01ff);
		color = (attr >> 11) & 0x0fU;
		flags = TILE_FLIPYX((attr >> 9) & 0x03U);
	}
	else
	{   // bluehawk/primella/popbingo
		// Tiles take one word in ROM:
		//          MSB             LSB
		// bluehawk YXCC CCcc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 9-0 of gfx code)
		// primella YXCC CCcc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 9-0 of gfx code)
		// popbingo YX?? ?ccc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 10-0 of gfx code)
		// rshark   YX?c cccc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 12-0 of gfx code)
		// c = gfx code
		// C = color code
		// X = x flip
		// Y = y flip
		// ? = unused?
		if (!m_tmap_cb.isnull())
			m_tmap_cb(attr & 0x3fff, code, color);
		else // just mimic old driver behavior (default configuration)
		{
			code = attr & 0x3ff;
			color = (attr & 0x3c00) >> 10;
		}

		flags = TILE_FLIPYX((attr >> 14) & 0x03U);
	}
	color |= m_palette_bank;

	tileinfo.set(m_gfxnum, code, color, flags);
}


rshark_rom_tilemap_device::rshark_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: dooyong_rom_tilemap_device(mconfig, RSHARK_ROM_TILEMAP, tag, owner, clock)
	, m_colorrom(*this, finder_base::DUMMY_TAG)
	, m_colorrom_offset(0)
	, m_colorrom_length(0)
{
	m_rows = 32;
}

void rshark_rom_tilemap_device::device_start()
{
	dooyong_rom_tilemap_device::device_start();

	if (0 > m_colorrom_offset)
		m_colorrom_offset = m_colorrom.length() + m_colorrom_offset;

	if (m_colorrom_length < 0)
		m_colorrom_length = m_colorrom.length();
}

TILE_GET_INFO_MEMBER(rshark_rom_tilemap_device::tile_info)
{
	dooyong_rom_tilemap_device::tile_info(tilemap, tileinfo, tile_index);

	const int tile_index_color = adjust_tile_index(tile_index) & (m_colorrom_length - 1);
	const u16 color = m_palette_bank | (m_colorrom[m_colorrom_offset + tile_index_color] & 0x0fU);
	tileinfo.palette_base = gfx().colorbase() + (gfx().granularity() * (color % gfx().colors()));
}


dooyong_ram_tilemap_device::dooyong_ram_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: dooyong_tilemap_device_base(mconfig, DOOYONG_RAM_TILEMAP, tag, owner, clock)
	, m_tileram()
{
}

void dooyong_ram_tilemap_device::tileram_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= (64U * 32U) - 1U;
	u16 value(m_tileram[offset]);
	COMBINE_DATA(&value);
	if (value != m_tileram[offset])
	{
		m_tileram[offset] = value;
		m_tilemap->mark_tile_dirty(offset);
	}
}

void dooyong_ram_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(dooyong_ram_tilemap_device::tile_info), this),
			TILEMAP_SCAN_COLS,
			8,
			8,
			64,
			32);
	m_tilemap->set_transparent_pen(15);

	m_tileram.reset(new u16[64 * 32]);
	std::fill(m_tileram.get(), m_tileram.get() + (64 * 32), 0U);
	m_palette_bank = 0U;

	save_pointer(NAME(m_tileram), 64 * 32);
	save_item(NAME(m_palette_bank));
}

TILE_GET_INFO_MEMBER(dooyong_ram_tilemap_device::tile_info)
{
	// Each tile takes one word of memory:
	// MSB             LSB
	// CCCC cccc cccc cccc  (bits 3-0 of color code, bits 11-0 of gfx code)
	// c = gfx code
	// C = color code
	const u16 attr(m_tileram[tile_index]);
	tileinfo.set(m_gfxnum, attr & 0x0fffU, m_palette_bank | ((attr >> 12) & 0x0fU), 0);
}
