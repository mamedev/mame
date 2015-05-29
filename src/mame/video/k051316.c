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
        bit 1 = unknown
        bit 2 = unknown
00f     unused

*/

#include "emu.h"
#include "k051316.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K051316 = &device_creator<k051316_device>;


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


k051316_device::k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K051316, "K051316 PSAC", tag, owner, clock, "k051316", __FILE__),
		device_gfx_interface(mconfig, *this, gfxinfo),
		m_zoom_rom(NULL),
		m_zoom_size(0),
		m_dx(0),
		m_dy(0),
		m_wrap(0),
		m_pixels_per_byte(2), // 4bpp layout is default
		m_layermask(0)
{
}

void k051316_device::set_bpp(device_t &device, int bpp)
{
	k051316_device &dev = downcast<k051316_device &>(device);

	switch(bpp)
	{
		case 4:
			device_gfx_interface::static_set_info(dev, gfxinfo);
			dev.m_pixels_per_byte = 2;
			break;
		case 7:
			device_gfx_interface::static_set_info(dev, gfxinfo7);
			dev.m_pixels_per_byte = 1;
			break;
		case 8:
			device_gfx_interface::static_set_info(dev, gfxinfo8);
			dev.m_pixels_per_byte = 1;
			break;
		case -4:
			device_gfx_interface::static_set_info(dev, gfxinfo4_ram);
			dev.m_pixels_per_byte = 2;
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
	memory_region *ROM = region();
	if (ROM != NULL)
	{
		m_zoom_rom = ROM->base();
		m_zoom_size = ROM->bytes();
	}

	decode_gfx();
	m_gfx[0]->set_colors(m_palette->entries() / m_gfx[0]->depth());

	m_tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(k051316_device::get_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_ram.resize(0x800);
	memset(&m_ram[0], 0, 0x800);

	if (m_layermask)
	{
		m_tmap->map_pens_to_layer(0, 0, 0, TILEMAP_PIXEL_LAYER1);
		m_tmap->map_pens_to_layer(0, m_layermask, m_layermask, TILEMAP_PIXEL_LAYER0);
	}
	else
		m_tmap->set_transparent_pen(0);

	// bind callbacks
	m_k051316_cb.bind_relative_to(*owner());

	save_item(NAME(m_ram));
	save_item(NAME(m_ctrlram));
	save_item(NAME(m_wrap));

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051316_device::device_reset()
{
	memset(m_ctrlram, 0, 0x10);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( k051316_device::read )
{
	return m_ram[offset];
}

WRITE8_MEMBER( k051316_device::write )
{
	m_ram[offset] = data;
	m_tmap->mark_tile_dirty(offset & 0x3ff);
}


READ8_MEMBER( k051316_device::rom_r )
{
	assert (m_zoom_size != 0);

	if ((m_ctrlram[0x0e] & 0x01) == 0)
	{
		int addr = offset + (m_ctrlram[0x0c] << 11) + (m_ctrlram[0x0d] << 19);
		addr /= m_pixels_per_byte;
		addr &= m_zoom_size - 1;

		//  popmessage("%s: offset %04x addr %04x", space.machine().describe_context(), offset, addr);

		return m_zoom_rom[addr];
	}
	else
	{
		//logerror("%s: read 051316 ROM offset %04x but reg 0x0c bit 0 not clear\n", space.machine().describe_context(), offset);
		return 0;
	}
}

WRITE8_MEMBER( k051316_device::ctrl_w )
{
	m_ctrlram[offset] = data;
	//if (offset >= 0x0c) logerror("%s: write %02x to 051316 reg %x\n", space.machine().describe_context(), data, offset);
}

// some games (ajax, rollerg, ultraman, etc.) have external logic that can enable or disable wraparound dynamically
void k051316_device::wraparound_enable( int status )
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
	int flags = 0;

	m_k051316_cb(&code, &color, &flags);

	SET_TILE_INFO_MEMBER(0,
							code,
							color,
							flags);
}


void k051316_device::zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	startx = 256 * ((INT16)(256 * m_ctrlram[0x00] + m_ctrlram[0x01]));
	incxx  =        (INT16)(256 * m_ctrlram[0x02] + m_ctrlram[0x03]);
	incyx  =        (INT16)(256 * m_ctrlram[0x04] + m_ctrlram[0x05]);
	starty = 256 * ((INT16)(256 * m_ctrlram[0x06] + m_ctrlram[0x07]));
	incxy  =        (INT16)(256 * m_ctrlram[0x08] + m_ctrlram[0x09]);
	incyy  =        (INT16)(256 * m_ctrlram[0x0a] + m_ctrlram[0x0b]);

	startx -= (16 + m_dy) * incyx;
	starty -= (16 + m_dy) * incyy;

	startx -= (89 + m_dx) * incxx;
	starty -= (89 + m_dx) * incxy;

	m_tmap->draw_roz(screen, bitmap, cliprect, startx << 5, starty << 5,
			incxx << 5, incxy << 5, incyx << 5, incyy << 5,
			m_wrap,
			flags, priority);

#if 0
	popmessage("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x",
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
			m_ctrlram[0x0c], /* bank for ROM testing */
			m_ctrlram[0x0d],
			m_ctrlram[0x0e], /* 0 = test ROMs */
			m_ctrlram[0x0f]);
#endif
}
