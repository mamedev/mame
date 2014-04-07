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


const gfx_layout namco_c45_road_device::s_tile_layout =
{
	ROAD_TILE_SIZE, ROAD_TILE_SIZE,
	ROAD_TILE_COUNT_MAX,
	2,
	{ 0, 8 },
	{// x offset
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17
	},
	{// y offset
		0x000,0x020,0x040,0x060,0x080,0x0a0,0x0c0,0x0e0,
		0x100,0x120,0x140,0x160,0x180,0x1a0,0x1c0,0x1e0
	},
	0x200 // offset to next tile
};

//-------------------------------------------------
//  namco_c45_road_device -- constructor
//-------------------------------------------------

namco_c45_road_device::namco_c45_road_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_C45_ROAD, "Namco C45 Road", tag, owner, clock, "namco_c45_road", __FILE__),
		m_transparent_color(~0),
		m_tilemap(NULL),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this)
{
}

//-------------------------------------------------
//  read -- read from RAM
//-------------------------------------------------

READ16_MEMBER( namco_c45_road_device::read )
{
	return m_ram[offset];
}


//-------------------------------------------------
//  write -- write to RAM
//-------------------------------------------------

WRITE16_MEMBER( namco_c45_road_device::write )
{
	COMBINE_DATA(&m_ram[offset]);

	// first half maps to the tilemap
	if (offset < 0x10000/2)
		m_tilemap->mark_tile_dirty(offset);

	// second half maps to the gfx elements
	else
	{
		offset -= 0x10000/2;
		m_gfxdecode->gfx(0)->mark_dirty(offset / WORDS_PER_ROAD_TILE);
	}
}


//-------------------------------------------------
//  draw -- render to the target bitmap
//-------------------------------------------------

void namco_c45_road_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	const UINT8 *clut = (const UINT8 *)memregion("clut")->base();
	bitmap_ind16 &source_bitmap = m_tilemap->pixmap();
	unsigned yscroll = m_ram[0x1fdfe/2];

	// loop over scanlines
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		// skip if we are not the right priority
		int screenx = m_ram[0x1fa00/2 + y + 15];
		if (pri != ((screenx & 0xf000) >> 12))
			continue;

		// skip if we don't have a valid zoom factor
		unsigned zoomx = m_ram[0x1fe00/2 + y + 15] & 0x3ff;
		if (zoomx == 0)
			continue;

		// skip if we don't have a valid source increment
		unsigned sourcey = m_ram[0x1fc00/2 + y + 15] + yscroll;
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
					if (clut != NULL)
						pen = (pen & ~0xff) | clut[pen & 0xff];
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
				if (clut != NULL)
					pen = (pen & ~0xff) | clut[pen & 0xff];
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
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	// create a gfx_element describing the road graphics
	m_gfxdecode->set_gfx(0, global_alloc(gfx_element(m_palette, s_tile_layout, 0x10000 + (UINT8 *)&m_ram[0], NATIVE_ENDIAN_VALUE_LE_BE(8,0), 0x3f, 0xf00)));

	// create a tilemap for the road
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namco_c45_road_device::get_road_info), this),
		TILEMAP_SCAN_ROWS, ROAD_TILE_SIZE, ROAD_TILE_SIZE, ROAD_COLS, ROAD_ROWS);
}

MACHINE_CONFIG_FRAGMENT( namco_c45_road )
	MCFG_GFXDECODE_ADD("gfxdecode", "^palette", empty) // FIXME
MACHINE_CONFIG_END
//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_c45_road_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_c45_road );
}

//-------------------------------------------------
//  device_stop -- device shutdown
//-------------------------------------------------

void namco_c45_road_device::device_stop()
{
}


//-------------------------------------------------
//  get_road_info -- tilemap callback
//-------------------------------------------------

TILE_GET_INFO_MEMBER( namco_c45_road_device::get_road_info )
{
	// ------xx xxxxxxxx tile number
	// xxxxxx-- -------- palette select
	UINT16 data = m_ram[tile_index];
	int tile = data & 0x3ff;
	int color = data >> 10;
	SET_TILE_INFO_MEMBER(0, tile, color, 0);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void namco_c45_road_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<namco_c45_road_device &>(device).m_palette.set_tag(tag);
}
