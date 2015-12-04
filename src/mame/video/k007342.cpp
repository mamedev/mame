// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 007342
------
The 007342 manages 2 64x32 scrolling tilemaps with 8x8 characters, and
optionally generates timing clocks and interrupt signals. It uses 0x2000
bytes of RAM, plus 0x0200 bytes for scrolling, and a variable amount of ROM.
It cannot read the ROMs.

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
*/

#include "emu.h"
#include "k007342.h"
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K007342 = &device_creator<k007342_device>;

k007342_device::k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K007342, "K007342 Video Controller", tag, owner, clock, "k007342", __FILE__),
	m_ram(nullptr),
	m_scroll_ram(nullptr),
	m_videoram_0(nullptr),
	m_videoram_1(nullptr),
	m_colorram_0(nullptr),
	m_colorram_1(nullptr),
	//m_tilemap[2];
	m_flipscreen(0),
	m_int_enabled(0),
	//m_regs[8],
	//m_scrollx[2],
	//m_scrolly[2],
	m_gfxdecode(*this),
	m_gfxnum(0)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void k007342_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<k007342_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007342_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	// bind the init function
	m_callback.bind_relative_to(*owner());

	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k007342_device::get_tile_info0),this), tilemap_mapper_delegate(FUNC(k007342_device::scan),this), 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k007342_device::get_tile_info1),this), tilemap_mapper_delegate(FUNC(k007342_device::scan),this), 8, 8, 64, 32);

	m_ram = auto_alloc_array_clear(machine(), UINT8, 0x2000);
	m_scroll_ram = auto_alloc_array_clear(machine(), UINT8, 0x0200);

	m_colorram_0 = &m_ram[0x0000];
	m_colorram_1 = &m_ram[0x1000];
	m_videoram_0 = &m_ram[0x0800];
	m_videoram_1 = &m_ram[0x1800];

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
	int i;

	m_int_enabled = 0;
	m_flipscreen = 0;
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;

	for (i = 0; i < 8; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( k007342_device::read )
{
	return m_ram[offset];
}

WRITE8_MEMBER( k007342_device::write )
{
	m_ram[offset] = data;

	if (offset < 0x1000)    /* layer 0 */
		m_tilemap[0]->mark_tile_dirty(offset & 0x7ff);
	else                /* layer 1 */
		m_tilemap[1]->mark_tile_dirty(offset & 0x7ff);
}

READ8_MEMBER( k007342_device::scroll_r )
{
	return m_scroll_ram[offset];
}

WRITE8_MEMBER( k007342_device::scroll_w )
{
	m_scroll_ram[offset] = data;
}

WRITE8_MEMBER( k007342_device::vreg_w )
{
	switch(offset)
	{
		case 0x00:
			/* bit 1: INT control */
			m_int_enabled = data & 0x02;
			m_flipscreen = data & 0x10;
			m_tilemap[0]->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			m_tilemap[1]->set_flip(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			break;
		case 0x01:  /* used for banking in Rock'n'Rage */
			if (data != m_regs[1])
				space.machine().tilemap().mark_all_dirty();
		case 0x02:
			m_scrollx[0] = (m_scrollx[0] & 0xff) | ((data & 0x01) << 8);
			m_scrollx[1] = (m_scrollx[1] & 0xff) | ((data & 0x02) << 7);
			break;
		case 0x03:  /* scroll x (register 0) */
			m_scrollx[0] = (m_scrollx[0] & 0x100) | data;
			break;
		case 0x04:  /* scroll y (register 0) */
			m_scrolly[0] = data;
			break;
		case 0x05:  /* scroll x (register 1) */
			m_scrollx[1] = (m_scrollx[1] & 0x100) | data;
			break;
		case 0x06:  /* scroll y (register 1) */
			m_scrolly[1] = data;
		case 0x07:  /* unused */
			break;
	}
	m_regs[offset] = data;
}

void k007342_device::tilemap_update( )
{
	int offs;

	/* update scroll */
	switch (m_regs[2] & 0x1c)
	{
		case 0x00:
		case 0x08:  /* unknown, blades of steel shootout between periods */
			m_tilemap[0]->set_scroll_rows(1);
			m_tilemap[0]->set_scroll_cols(1);
			m_tilemap[0]->set_scrollx(0, m_scrollx[0]);
			m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
			break;

		case 0x0c:  /* 32 columns */
			m_tilemap[0]->set_scroll_rows(1);
			m_tilemap[0]->set_scroll_cols(512);
			m_tilemap[0]->set_scrollx(0, m_scrollx[0]);
			for (offs = 0; offs < 256; offs++)
				m_tilemap[0]->set_scrolly((offs + m_scrollx[0]) & 0x1ff,
						m_scroll_ram[2 * (offs / 8)] + 256 * m_scroll_ram[2 * (offs / 8) + 1]);
			break;

		case 0x14:  /* 256 rows */
			m_tilemap[0]->set_scroll_rows(256);
			m_tilemap[0]->set_scroll_cols(1);
			m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
			for (offs = 0; offs < 256; offs++)
				m_tilemap[0]->set_scrollx((offs + m_scrolly[0]) & 0xff,
						m_scroll_ram[2 * offs] + 256 * m_scroll_ram[2 * offs + 1]);
			break;

		default:
//          popmessage("unknown scroll ctrl %02x", m_regs[2] & 0x1c);
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

void k007342_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, UINT32 priority )
{
	m_tilemap[num]->draw(screen, bitmap, cliprect, flags, priority);
}

int k007342_device::is_int_enabled( )
{
	return m_int_enabled;
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
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5);
}

void k007342_device::get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram )
{
	int color, code, flags;

	color = cram[tile_index];
	code = vram[tile_index];
	flags = TILE_FLIPYX((color & 0x30) >> 4);

	tileinfo.category = (color & 0x80) >> 7;

	if (!m_callback.isnull())
		m_callback(layer, m_regs[1], &code, &color, &flags);


	SET_TILE_INFO_MEMBER(m_gfxnum,
			code,
			color,
			flags);
}

TILE_GET_INFO_MEMBER(k007342_device::get_tile_info0)
{
	get_tile_info(tileinfo, tile_index, 0, m_colorram_0, m_videoram_0);
}

TILE_GET_INFO_MEMBER(k007342_device::get_tile_info1)
{
	get_tile_info(tileinfo, tile_index, 1, m_colorram_1, m_videoram_1);
}
