// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*

Konami 007342
------
The 007342 manages 2 64x32 scrolling tilemaps with 8x8 characters, and
optionally generates timing clocks and interrupt signals. It uses 0x2000
bytes of RAM, plus 0x0200 bytes for scrolling, and a variable amount of ROM.
It cannot read the ROMs.

Some of the control flags are specifically meant for other Konami chips,
such as the sprite wraparound flag for 007420.

control registers
000: ------x- INT control
     ---x---- flip screen (TODO: doesn't work with thehustl)
001: Used for banking in Rock'n'Rage
002: -------x MSB of x scroll 1
     ------x- MSB of x scroll 2
     ---xxx-- layer 1 row/column scroll control
              000 = disabled
              010 = unknown (bladestl shootout between periods)
              011 = 32 columns (Blades of Steel)
              101 = 256 rows (Battlantis, Rock 'n Rage)
     x------- enable sprite wraparound from bottom to top (see Blades of Steel
              high score table)
003: x scroll 1
004: y scroll 1
005: x scroll 2
006: y scroll 2
007: not used

TODO:
- device should be combined with 007420? see comment from AWJ here:
  https://mametesters.org/view.php?id=4902

*/

#include "emu.h"
#include "k007342.h"
#include "konami_helper.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K007342, k007342_device, "k007342", "K007342 Video Controller")

k007342_device::k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, K007342, tag, owner, clock),
	device_gfx_interface(mconfig, *this),
	m_ram(nullptr),
	m_scroll_ram(nullptr),
	m_videoram{ nullptr, nullptr },
	m_colorram{ nullptr, nullptr },
	m_tilemap{ nullptr, nullptr },
	m_flipscreen(false),
	m_int_enabled(false),
	m_callback(*this),
	m_flipscreen_cb(*this),
	m_sprite_wrap_y_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007342_device::device_start()
{
	// bind the init function
	m_callback.resolve();

	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k007342_device::get_tile_info0)), tilemap_mapper_delegate(*this, FUNC(k007342_device::scan)), 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k007342_device::get_tile_info1)), tilemap_mapper_delegate(*this, FUNC(k007342_device::scan)), 8, 8, 64, 32);

	m_ram = make_unique_clear<uint8_t[]>(0x2000);
	m_scroll_ram = make_unique_clear<uint8_t[]>(0x0200);

	m_colorram[0] = &m_ram[0x0000];
	m_colorram[1] = &m_ram[0x1000];
	m_videoram[0] = &m_ram[0x0800];
	m_videoram[1] = &m_ram[0x1800];

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	save_pointer(NAME(m_ram), 0x2000);
	save_pointer(NAME(m_scroll_ram), 0x0200);
	save_item(NAME(m_int_enabled));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k007342_device::device_reset()
{
	m_int_enabled = false;
	m_flipscreen = false;

	m_flipscreen_cb(0);
	m_sprite_wrap_y_cb(0);

	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;

	for (int i = 0; i < 8; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

uint8_t k007342_device::read(offs_t offset)
{
	return m_ram[offset];
}

void k007342_device::write(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;

	if (offset < 0x1000)
		m_tilemap[0]->mark_tile_dirty(offset & 0x7ff);
	else
		m_tilemap[1]->mark_tile_dirty(offset & 0x7ff);
}

uint8_t k007342_device::scroll_r(offs_t offset)
{
	return m_scroll_ram[offset];
}

void k007342_device::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll_ram[offset] = data;
}

void k007342_device::vreg_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:
			// bit 1: INT control
			m_int_enabled = BIT(data, 1);

			// bit 4: flip screen
			m_flipscreen = BIT(data, 4);
			m_flipscreen_cb(BIT(data, 4));
			m_tilemap[0]->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_tilemap[1]->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			break;
		case 0x01: // used for banking in Rock'n'Rage
			if (data != m_regs[1])
				machine().tilemap().mark_all_dirty();
			break;
		case 0x02:
			m_scrollx[0] = (m_scrollx[0] & 0xff) | ((data & 0x01) << 8);
			m_scrollx[1] = (m_scrollx[1] & 0xff) | ((data & 0x02) << 7);
			m_sprite_wrap_y_cb(BIT(data, 7));
			break;
		case 0x03: // scroll x (register 0)
			m_scrollx[0] = (m_scrollx[0] & 0x100) | data;
			break;
		case 0x04: // scroll y (register 0)
			m_scrolly[0] = data;
			break;
		case 0x05: // scroll x (register 1)
			m_scrollx[1] = (m_scrollx[1] & 0x100) | data;
			break;
		case 0x06: // scroll y (register 1)
			m_scrolly[1] = data;
			break;
		case 0x07: // unused
			break;
	}
	m_regs[offset] = data;
}

void k007342_device::tilemap_update()
{
	// update scroll
	switch (m_regs[2] & 0x1c)
	{
		case 0x00:
		case 0x08: // unknown, blades of steel shootout between periods
			m_tilemap[0]->set_scroll_rows(1);
			m_tilemap[0]->set_scroll_cols(1);
			m_tilemap[0]->set_scrollx(0, m_scrollx[0]);
			m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
			break;

		case 0x0c: // 32 columns
			m_tilemap[0]->set_scroll_rows(1);
			m_tilemap[0]->set_scroll_cols(512);
			m_tilemap[0]->set_scrollx(0, m_scrollx[0]);
			for (int offs = 0; offs < 256; offs++)
				m_tilemap[0]->set_scrolly((offs + m_scrollx[0]) & 0x1ff,
						m_scroll_ram[2 * (offs / 8)] + 256 * m_scroll_ram[2 * (offs / 8) + 1]);
			break;

		case 0x14: // 256 rows
			m_tilemap[0]->set_scroll_rows(256);
			m_tilemap[0]->set_scroll_cols(1);
			m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
			for (int offs = 0; offs < 256; offs++)
				m_tilemap[0]->set_scrollx((offs + m_scrolly[0]) & 0xff,
						m_scroll_ram[2 * offs] + 256 * m_scroll_ram[2 * offs + 1]);
			break;

		default:
			//popmessage("unknown scroll ctrl %02x", m_regs[2] & 0x1c);
			break;
	}

	m_tilemap[1]->set_scrollx(0, m_scrollx[1]);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);

#if 0
	{
		static int current_layer = 0;

		if (machine.input().code_pressed_once(KEYCODE_Z)) current_layer = !current_layer;
		m_tilemap[current_layer]->enable(1);
		m_tilemap[!current_layer]->enable(0);

		popmessage("regs:%02x %02x %02x %02x-%02x %02x %02x %02x:%02x",
			m_regs[0], m_regs[1], m_regs[2], m_regs[3],
			m_regs[4], m_regs[5], m_regs[6], m_regs[7],
			current_layer);
	}
#endif
}

void k007342_device::tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, uint32_t priority)
{
	m_tilemap[num]->draw(screen, bitmap, cliprect, flags, priority);
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/*
  data format:
  video RAM     xxxxxxxx    tile number (bits 0-7)
  color RAM     x-------    tiles with priority over the sprites
  color RAM     -x------    depends on external conections
  color RAM     --x-----    flip Y
  color RAM     ---x----    flip X
  color RAM     ----xxxx    depends on external connections (usually color and banking)
*/

TILEMAP_MAPPER_MEMBER(k007342_device::scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

void k007342_device::get_tile_info( tile_data &tileinfo, int tile_index, uint8_t layer )
{
	uint32_t color = m_colorram[layer][tile_index];
	uint32_t code = m_videoram[layer][tile_index];
	uint8_t flags = TILE_FLIPYX((color & 0x30) >> 4);

	tileinfo.category = (color & 0x80) >> 7;

	if (!m_callback.isnull())
		m_callback(layer, m_regs[1], code, color, flags);

	tileinfo.set(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(k007342_device::get_tile_info0)
{
	get_tile_info(tileinfo, tile_index, 0);
}

TILE_GET_INFO_MEMBER(k007342_device::get_tile_info1)
{
	get_tile_info(tileinfo, tile_index, 1);
}
