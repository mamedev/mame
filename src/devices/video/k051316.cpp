// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 051316 PSAC
------
Manages a 32x32 tilemap (16x16 tiles, 512x512 pixels) which can be zoomed,
distorted and rotated.
It uses two internal 24 bit counters which are incremented while scanning the
picture. The coordinates of the pixel in the tilemap that has to be drawn to
the current beam position are the counters / (2^11).
The chip doesn't directly generate the color information for the pixel, it
just generates a 24 bit address (whose top 16 bits are the contents of the
tilemap RAM), and a "visible" signal. It's up to external circuitry to convert
the address into a pixel color. Most games seem to use 4bpp graphics, but Ajax
uses 7bpp.
If the value in the internal counters is out of the visible range (0..511), it
is truncated and the corresponding address is still generated, but the "visible"
signal is not asserted. The external circuitry might ignore that signal and
still generate the pixel, therefore making the tilemap a continuous playfield
that wraps around instead of a large sprite.

control registers
000-001 X counter starting value / 256
002-003 amount to add to the X counter after each horizontal pixel
004-005 amount to add to the X counter after each line (0 = no rotation)
006-007 Y counter starting value / 256
008-009 amount to add to the Y counter after each horizontal pixel (0 = no rotation)
00a-00b amount to add to the Y counter after each line
00c-00d ROM bank to read, used during ROM testing
00e     bit 0 = enable ROM reading (active low). This only makes the chip output the
                requested address: the ROM is actually read externally, not through
                the chip's data bus.
        bit 1 = enable tile X flip when tile color attribute bit 6 is set
        bit 2 = enable tile Y flip when tile color attribute bit 7 is set
        bits 3-7 = internal tests, shouldn't be used
00f     unused

*/

#include "emu.h"
#include "k051316.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(K051316, k051316_device, "k051316", "K051316 PSAC")


const gfx_layout k051316_device::charlayout4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
		8*4, 9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

const gfx_layout k051316_device::charlayout7 =
{
	16,16,
	RGN_FRAC(1,1),
	7,
	{ 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8
};

const gfx_layout k051316_device::charlayout8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8
};


GFXDECODE_MEMBER( k051316_device::gfxinfo )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, charlayout4, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k051316_device::gfxinfo7 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, charlayout7, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k051316_device::gfxinfo8 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, charlayout8, 0, 1)
GFXDECODE_END

GFXDECODE_MEMBER( k051316_device::gfxinfo4_ram )
	GFXDECODE_DEVICE_RAM(DEVICE_SELF, 0, charlayout4, 0, 1)
GFXDECODE_END


k051316_device::k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K051316, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
	, m_zoom_rom(*this, DEVICE_SELF)
	, m_dx(0)
	, m_dy(0)
	, m_wrap(0)
	, m_pixels_per_byte(2) // 4bpp layout is default
	, m_layermask(0)
	, m_k051316_cb(*this)
	, m_readout_enabled(true)
	, m_flipx_enabled(false)
	, m_flipy_enabled(false)
{
}

void k051316_device::set_bpp(int bpp)
{
	switch(bpp)
	{
		case 4:
			set_info(gfxinfo);
			m_pixels_per_byte = 2;
			break;
		case 7:
			set_info(gfxinfo7);
			m_pixels_per_byte = 1;
			break;
		case 8:
			set_info(gfxinfo8);
			m_pixels_per_byte = 1;
			break;
		case -4:
			set_info(gfxinfo4_ram);
			m_pixels_per_byte = 2;
			break;
		default:
			fatalerror("Unsupported bpp\n");
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051316_device::device_start()
{
	// assumes it can make an address mask with .length() - 1
	assert(!(m_zoom_rom.length() & (m_zoom_rom.length() - 1)));

	if (!palette().device().started())
		throw device_missing_dependencies();

	// bind callbacks
	m_k051316_cb.resolve();

	decode_gfx();
	gfx(0)->set_colors(palette().entries() / gfx(0)->depth());

	m_tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(k051316_device::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_ram.resize(0x800);
	memset(&m_ram[0], 0, 0x800);

	std::fill(std::begin(m_ctrlram), std::end(m_ctrlram), 0);

	if (m_layermask)
	{
		m_tmap->map_pens_to_layer(0, 0, 0, TILEMAP_PIXEL_LAYER1);
		m_tmap->map_pens_to_layer(0, m_layermask, m_layermask, TILEMAP_PIXEL_LAYER0);
	}
	else
	{
		m_tmap->set_transparent_pen(0);
	}

	save_item(NAME(m_ram));
	save_item(NAME(m_ctrlram));
	save_item(NAME(m_wrap));
	save_item(NAME(m_readout_enabled));
	save_item(NAME(m_flipx_enabled));
	save_item(NAME(m_flipy_enabled));

}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

u8 k051316_device::read(offs_t offset)
{
	return m_ram[offset];
}

void k051316_device::write(offs_t offset, u8 data)
{
	m_ram[offset] = data;
	m_tmap->mark_tile_dirty(offset & 0x3ff);
}


u8 k051316_device::rom_r(offs_t offset)
{
	assert(m_zoom_rom.found());

	if (m_readout_enabled)
	{
		int addr = offset + (m_ctrlram[0x0c] << 11) + (m_ctrlram[0x0d] << 19);
		addr /= m_pixels_per_byte;
		addr &= m_zoom_rom.length() - 1;

		//  popmessage("%s: offset %04x addr %04x", machine().describe_context(), offset, addr);

		return m_zoom_rom[addr];
	}
	else
	{
		//logerror("%s: read 051316 ROM offset %04x but reg 0x0c bit 0 not clear\n", machine().describe_context(), offset);
		return 0;
	}
}

void k051316_device::ctrl_w(offs_t offset, u8 data)
{
	if (offset == 0x0e)
	{
		m_readout_enabled = !BIT(data, 0);

		bool flipx = bool(BIT(data, 1));
		bool flipy = bool(BIT(data, 2));
		if (m_flipx_enabled != flipx || m_flipy_enabled != flipy)
		{
			m_flipx_enabled = flipx;
			m_flipy_enabled = flipy;
			m_tmap->mark_all_dirty();
		}
	}
	else if (offset < 0x0e)
		m_ctrlram[offset] = data;

	//if (offset >= 0x0c) logerror("%s: write %02x to 051316 reg %x\n", machine().describe_context(), data, offset);
}

// some games (ajax, rollerg, ultraman, etc.) have external logic that can enable or disable wraparound dynamically
void k051316_device::wraparound_enable(int status)
{
	m_wrap = status;
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(k051316_device::get_tile_info)
{
	int code = m_ram[tile_index];
	int color = m_ram[tile_index + 0x400];
	u8 flags = 0;

	if (m_flipx_enabled && (color & 0x40))
		flags |= TILE_FLIPX;

	if (m_flipy_enabled && (color & 0x80))
		flags |= TILE_FLIPY;

	m_k051316_cb(&code, &color);

	tileinfo.set(0,
			code,
			color,
			flags);
}


void k051316_device::zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u32 priority )
{
	u32 startx, starty;
	int incxx, incxy, incyx, incyy;

	startx = 256 * ((int16_t)(256 * m_ctrlram[0x00] + m_ctrlram[0x01]));
	incxx  =        (int16_t)(256 * m_ctrlram[0x02] + m_ctrlram[0x03]);
	incyx  =        (int16_t)(256 * m_ctrlram[0x04] + m_ctrlram[0x05]);
	starty = 256 * ((int16_t)(256 * m_ctrlram[0x06] + m_ctrlram[0x07]));
	incxy  =        (int16_t)(256 * m_ctrlram[0x08] + m_ctrlram[0x09]);
	incyy  =        (int16_t)(256 * m_ctrlram[0x0a] + m_ctrlram[0x0b]);

	startx -= (16 + m_dy) * incyx;
	starty -= (16 + m_dy) * incyy;

	startx -= (-7 + m_dx) * incxx;
	starty -= (-7 + m_dx) * incxy;

	m_tmap->draw_roz(screen, bitmap, cliprect, startx << 5, starty << 5,
			incxx << 5, incxy << 5, incyx << 5, incyy << 5,
			m_wrap,
			flags, priority);

#if 0
	popmessage("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x",
			m_ctrlram[0x00],
			m_ctrlram[0x01],
			m_ctrlram[0x02],
			m_ctrlram[0x03],
			m_ctrlram[0x04],
			m_ctrlram[0x05],
			m_ctrlram[0x06],
			m_ctrlram[0x07],
			m_ctrlram[0x08],
			m_ctrlram[0x09],
			m_ctrlram[0x0a],
			m_ctrlram[0x0b],
			m_ctrlram[0x0c], // bank for ROM testing
			m_ctrlram[0x0d]);
#endif
}
