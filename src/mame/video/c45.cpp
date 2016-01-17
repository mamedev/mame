// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Aaron Giles, Alex W. Jackson
/**************************************************************************************************************/
/*
    Land Line Buffer
    Land Generator
        0xf,0x7,0xe,0x6,0xd,0x5,0xc,0x4,
        0xb,0x3,0xa,0x2,0x9,0x1,0x8,0x0

*/

/* Preliminary!  The road circuitry is identical for all the driving games.
 *
 * There are several chunks of RAM
 *
 *  Road Tilemap:
 *      0x00000..0x0ffff    64x512 tilemap
 *
 *  Road Tiles:
 *      0x10000..0x1f9ff    16x16x2bpp tiles
 *
 *
 *  Line Attributes:
 *
 *      0x1fa00..0x1fbdf    xxx- ---- ---- ----     priority
 *                          ---- xxxx xxxx xxxx     xscroll
 *
 *      0x1fbfe             horizontal adjust?
 *                          0x0017
 *                          0x0018 (Final Lap3)
 *
 *      0x1fc00..0x1fddf    selects line in source bitmap
 *      0x1fdfe             yscroll
 *
 *      0x1fe00..0x1ffdf    ---- --xx xxxx xxxx     zoomx
 *      0x1fffd             always 0xffff 0xffff?
 */

#include "emu.h"
#include "video/c45.h"


//****************************************************************************
//  CONSTANTS
//****************************************************************************

// device type definition
const device_type NAMCO_C45_ROAD = &device_creator<namco_c45_road_device>;


const gfx_layout namco_c45_road_device::tilelayout =
{
	ROAD_TILE_SIZE, ROAD_TILE_SIZE,
	RGN_FRAC(1,1),
	2,
	{ 0, 8 },
	{ STEP8(0, 1), STEP8(16, 1) },
	{ STEP16(0, 32) },
	0x200 // offset to next tile
};


GFXDECODE_MEMBER( namco_c45_road_device::gfxinfo )
	GFXDECODE_DEVICE_RAM( "tileram", 0, tilelayout, 0xf00, 64 )
GFXDECODE_END


DEVICE_ADDRESS_MAP_START(map, 16, namco_c45_road_device)
	AM_RANGE(0x00000, 0x0ffff) AM_RAM_WRITE(tilemap_w) AM_SHARE("tmapram")
	AM_RANGE(0x10000, 0x1f9ff) AM_RAM_WRITE(tileram_w) AM_SHARE("tileram")
	AM_RANGE(0x1fa00, 0x1ffff) AM_RAM AM_SHARE("lineram")
ADDRESS_MAP_END


//-------------------------------------------------
//  namco_c45_road_device -- constructor
//-------------------------------------------------

namco_c45_road_device::namco_c45_road_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_C45_ROAD, "Namco C45 Road", tag, owner, clock, "namco_c45_road", __FILE__),
		device_gfx_interface(mconfig, *this, gfxinfo),
		device_memory_interface(mconfig, *this),
		m_space_config("c45", ENDIANNESS_BIG, 16, 17, 0, address_map_delegate(FUNC(namco_c45_road_device::map), this)),
		m_tmapram(*this, "tmapram"),
		m_tileram(*this, "tileram"),
		m_lineram(*this, "lineram"),
		m_transparent_color(~0)
{
}



// We need these trampolines for now because uplift_submaps()
// can't deal with address maps that contain RAM.
// We need to explicitly use device_memory_interface::space()
// because read/write handlers have a parameter called 'space'

//-------------------------------------------------
//  read -- CPU read from our address space
//-------------------------------------------------

READ16_MEMBER( namco_c45_road_device::read )
{
	return device_memory_interface::space().read_word(offset*2);
}


//-------------------------------------------------
//  write -- CPU write to our address space
//-------------------------------------------------

WRITE16_MEMBER( namco_c45_road_device::write )
{
	device_memory_interface::space().write_word(offset*2, data, mem_mask);
}


//-------------------------------------------------
//  tilemap_w -- write to tilemap RAM
//-------------------------------------------------

WRITE16_MEMBER( namco_c45_road_device::tilemap_w )
{
	COMBINE_DATA(&m_tmapram[offset]);
	m_tilemap->mark_tile_dirty(offset);
}


//-------------------------------------------------
//  tileram_w -- write to tile RAM
//-------------------------------------------------

WRITE16_MEMBER( namco_c45_road_device::tileram_w )
{
	COMBINE_DATA(&m_tileram[offset]);
	m_gfx[0]->mark_dirty(offset / WORDS_PER_ROAD_TILE);
}


//-------------------------------------------------
//  draw -- render to the target bitmap
//-------------------------------------------------

void namco_c45_road_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	bitmap_ind16 &source_bitmap = m_tilemap->pixmap();
	unsigned yscroll = m_lineram[0x3fe/2];

	// loop over scanlines
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// skip if we are not the right priority
		int screenx = m_lineram[y + 15];
		if (pri != ((screenx & 0xf000) >> 12))
			continue;

		// skip if we don't have a valid zoom factor
		unsigned zoomx = m_lineram[0x400/2 + y + 15] & 0x3ff;
		if (zoomx == 0)
			continue;

		// skip if we don't have a valid source increment
		unsigned sourcey = m_lineram[0x200/2 + y + 15] + yscroll;
		const UINT16 *source_gfx = &source_bitmap.pix(sourcey & (ROAD_TILEMAP_HEIGHT - 1));
		unsigned dsourcex = (ROAD_TILEMAP_WIDTH << 16) / zoomx;
		if (dsourcex == 0)
			continue;

		// mask off priority bits and sign-extend
		screenx &= 0x0fff;
		if (screenx & 0x0800)
			screenx |= ~0x7ff;

		// adjust the horizontal placement
		screenx -= 64; // needs adjustment to left

		int numpixels = (44 * ROAD_TILE_SIZE << 16) / dsourcex;
		unsigned sourcex = 0;

		// crop left
		int clip_pixels = cliprect.min_x - screenx;
		if (clip_pixels > 0)
		{
			numpixels -= clip_pixels;
			sourcex += dsourcex*clip_pixels;
			screenx = cliprect.min_x;
		}

		// crop right
		clip_pixels = (screenx + numpixels) - (cliprect.max_x + 1);
		if (clip_pixels > 0)
			numpixels -= clip_pixels;

		// TBA: work out palette mapping for Final Lap, Suzuka

		// BUT: support transparent color for Thunder Ceptor
		UINT16 *dest = &bitmap.pix(y);
		if (m_transparent_color != ~0)
		{
			while (numpixels-- > 0)
			{
				int pen = source_gfx[sourcex >> 16];
				if (m_palette->pen_indirect(pen) != m_transparent_color)
				{
					if (m_clut != nullptr)
						pen = (pen & ~0xff) | m_clut[pen & 0xff];
					dest[screenx] = pen;
				}
				screenx++;
				sourcex += dsourcex;
			}
		}
		else
		{
			while (numpixels-- > 0)
			{
				int pen = source_gfx[sourcex >> 16];
				if (m_clut != nullptr)
					pen = (pen & ~0xff) | m_clut[pen & 0xff];
				dest[screenx++] = pen;
				sourcex += dsourcex;
			}
		}
	}
}


//-------------------------------------------------
//  device_start -- device startup
//-------------------------------------------------

void namco_c45_road_device::device_start()
{
	m_clut = memregion("clut")->base();

	// create a tilemap for the road
	m_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(namco_c45_road_device::get_road_info), this),
		TILEMAP_SCAN_ROWS, ROAD_TILE_SIZE, ROAD_TILE_SIZE, ROAD_COLS, ROAD_ROWS);
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *namco_c45_road_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  get_road_info -- tilemap callback
//-------------------------------------------------

TILE_GET_INFO_MEMBER( namco_c45_road_device::get_road_info )
{
	// ------xx xxxxxxxx tile number
	// xxxxxx-- -------- palette select
	UINT16 data = m_tmapram[tile_index];
	int tile = data & 0x3ff;
	int color = data >> 10;
	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}
