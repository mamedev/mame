// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito PC080SN
-------
Tilemap generator. Two tilemaps, with gfx data fetched from ROM.
Darius uses 3xPC080SN and has double width tilemaps. (NB: it
has not been verified that Topspeed uses this chip. Possibly
it had a variant with added rowscroll capability.)

Standard memory layout (two 64x64 tilemaps with 8x8 tiles)

0000-3fff BG
4000-41ff BG rowscroll      (only verified to exist on Topspeed)
4200-7fff unknown/unused?
8000-bfff FG    (FG/BG layer order fixed per game; Topspeed has BG on top)
c000-c1ff FG rowscroll      (only verified to exist on Topspeed)
c200-ffff unknown/unused?

Double width memory layout (two 128x64 tilemaps with 8x8 tiles)

0000-7fff BG
8000-ffff FG
(Tile layout is different; tiles and colors are separated:
0x0000-3fff  color / flip words
0x4000-7fff  tile number words)

Control registers

+0x20000 (on from tilemaps)
000-001 BG scroll Y
002-003 FG scroll Y

+0x40000
000-001 BG scroll X
002-003 FG scroll X

+0x50000 control word (written infrequently, only 2 bits used)
       ---------------x flip screen
       ----------x----- 0x20 poked here in Topspeed init, followed
                        by zero (Darius does the same).
*/

#include "emu.h"
#include "pc080sn.h"
#include "video/taito_helper.h"

#define PC080SN_RAM_SIZE 0x10000
#define TOPSPEED_ROAD_COLORS

const device_type PC080SN = &device_creator<pc080sn_device>;

pc080sn_device::pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC080SN, "Taito PC080SN", tag, owner, clock, "pc080sn", __FILE__),
	m_ram(nullptr),
	m_gfxnum(0),
	m_x_offset(0),
	m_y_offset(0),
	m_y_invert(0),
	m_dblwidth(0),
	m_gfxdecode(*this)
{
	for (auto & elem : m_ctrl)
		elem = 0;

	for (int i = 0; i < 2; i++)
	{
		m_bg_ram[i] = nullptr;
		m_bgscroll_ram[i] = nullptr;
		m_bgscrollx[i] = 0;
		m_bgscrolly[i] = 0;
	}
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void pc080sn_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<pc080sn_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc080sn_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	/* use the given gfx set for bg tiles */
	if (!m_dblwidth) /* standard tilemaps */
	{
		m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pc080sn_device::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
		m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pc080sn_device::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	}
	else    /* double width tilemaps */
	{
		m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pc080sn_device::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
		m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pc080sn_device::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	}

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap[0]->set_scrolldx(-16 + m_x_offset, -16 - m_x_offset);
	m_tilemap[0]->set_scrolldy(m_y_offset, -m_y_offset);
	m_tilemap[1]->set_scrolldx(-16 + m_x_offset, -16 - m_x_offset);
	m_tilemap[1]->set_scrolldy(m_y_offset, -m_y_offset);

	if (!m_dblwidth)
	{
		m_tilemap[0]->set_scroll_rows(512);
		m_tilemap[1]->set_scroll_rows(512);
	}

	m_ram = make_unique_clear<UINT16[]>(PC080SN_RAM_SIZE / 2);

	m_bg_ram[0]       = m_ram.get() + 0x0000 /2;
	m_bg_ram[1]       = m_ram.get() + 0x8000 /2;
	m_bgscroll_ram[0] = m_ram.get() + 0x4000 /2;
	m_bgscroll_ram[1] = m_ram.get() + 0xc000 /2;

	save_pointer(NAME(m_ram.get()), PC080SN_RAM_SIZE / 2);
	save_item(NAME(m_ctrl));
	machine().save().register_postload(save_prepost_delegate(FUNC(pc080sn_device::restore_scroll), this));

}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void pc080sn_device::common_get_pc080sn_bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	UINT16 code, attr;

	if (!m_dblwidth)
	{
		code = (ram[2 * tile_index + 1] & 0x3fff);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO_MEMBER(gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

TILE_GET_INFO_MEMBER(pc080sn_device::get_bg_tile_info)
{
	common_get_pc080sn_bg_tile_info( tileinfo, tile_index, m_bg_ram[0], m_gfxnum );
}

void pc080sn_device::common_get_pc080sn_fg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	UINT16 code,attr;

	if (!m_dblwidth)
	{
		code = (ram[2 * tile_index + 1] & 0x3fff);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO_MEMBER(gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

TILE_GET_INFO_MEMBER(pc080sn_device::get_fg_tile_info)
{
	common_get_pc080sn_fg_tile_info( tileinfo, tile_index, m_bg_ram[1], m_gfxnum );
}

READ16_MEMBER( pc080sn_device::word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( pc080sn_device::word_w )
{
	COMBINE_DATA(&m_ram[offset]);

	if (!m_dblwidth)
	{
		if (offset < 0x2000)
			m_tilemap[0]->mark_tile_dirty(offset / 2);
		else if (offset >= 0x4000 && offset < 0x6000)
			m_tilemap[1]->mark_tile_dirty((offset & 0x1fff) / 2);
	}
	else
	{
		if (offset < 0x4000)
			m_tilemap[0]->mark_tile_dirty((offset & 0x1fff));
		else if (offset >= 0x4000 && offset < 0x8000)
			m_tilemap[1]->mark_tile_dirty((offset & 0x1fff));
	}
}

WRITE16_MEMBER( pc080sn_device::xscroll_word_w )
{
	COMBINE_DATA(&m_ctrl[offset]);

	data = m_ctrl[offset];

	switch (offset)
	{
		case 0x00:
			m_bgscrollx[0] = -data;
			break;

		case 0x01:
			m_bgscrollx[1] = -data;
			break;
	}
}

WRITE16_MEMBER( pc080sn_device::yscroll_word_w )
{
	COMBINE_DATA(&m_ctrl[offset + 2]);

	data = m_ctrl[offset + 2];

	if (m_y_invert)
		data = -data;

	switch (offset)
	{
		case 0x00:
			m_bgscrolly[0] = -data;
			break;

		case 0x01:
			m_bgscrolly[1] = -data;
			break;
	}
}

WRITE16_MEMBER( pc080sn_device::ctrl_word_w )
{
	COMBINE_DATA(&m_ctrl[offset + 4]);

	data = m_ctrl[offset + 4];

	switch (offset)
	{
		case 0x00:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			m_tilemap[0]->set_flip(flip);
			m_tilemap[1]->set_flip(flip);
			break;
		}
	}
#if 0
	popmessage("pc080sn ctrl = %4x", data);
#endif
}


/* This routine is needed as an override by Jumping, which
   doesn't set proper scroll values for foreground tilemap */

void pc080sn_device::set_scroll( int tilemap_num, int scrollx, int scrolly )
{
	m_tilemap[tilemap_num]->set_scrollx(0, scrollx);
	m_tilemap[tilemap_num]->set_scrolly(0, scrolly);
}

/* This routine is needed as an override by Jumping */

void pc080sn_device::set_trans_pen( int tilemap_num, int pen )
{
	m_tilemap[tilemap_num]->set_transparent_pen(pen);
}


void pc080sn_device::tilemap_update( )
{
	int j;

	m_tilemap[0]->set_scrolly(0, m_bgscrolly[0]);
	m_tilemap[1]->set_scrolly(0, m_bgscrolly[1]);

	if (!m_dblwidth)
	{
		for (j = 0; j < 256; j++)
			m_tilemap[0]->set_scrollx((j + m_bgscrolly[0]) & 0x1ff,   m_bgscrollx[0] - m_bgscroll_ram[0][j]);

		for (j = 0; j < 256; j++)
			m_tilemap[1]->set_scrollx((j + m_bgscrolly[1]) & 0x1ff, m_bgscrollx[1] - m_bgscroll_ram[1][j]);
	}
	else
	{
		m_tilemap[0]->set_scrollx(0, m_bgscrollx[0]);
		m_tilemap[1]->set_scrollx(0, m_bgscrollx[1]);
	}
}


static UINT16 topspeed_get_road_pixel_color( UINT16 pixel, UINT16 color )
{
	UINT16 road_body_color, off_road_color, pixel_type;

	/* Color changes based on screenshots from game flyer */
	pixel_type = (pixel % 0x10);
	road_body_color = (pixel & 0x7ff0) + 4;
	off_road_color = road_body_color + 1;

	if ((color & 0xffe0) == 0xffe0)
	{
		pixel += 10;    /* Tunnel colors */
		road_body_color += 10;
		off_road_color  += 10;
	}
	else
	{
		/* Unsure which way round these bits go */
		if (color & 0x10)   road_body_color += 5;
		if (color & 0x02)   off_road_color  += 5;
	}

	switch (pixel_type)
	{
	case 0x01:      /* Center lines */
		if (color & 0x08)
			pixel = road_body_color;
		break;
	case 0x02:      /* Road edge (inner) */
		if (color & 0x08)
			pixel = road_body_color;
		break;
	case 0x03:      /* Road edge (outer) */
		if (color & 0x04)
			pixel = road_body_color;
		break;
	case 0x04:      /* Road body */
		pixel = road_body_color;
		break;
	case 0x05:      /* Off road */
		pixel = off_road_color;
		break;
	default:
		{}
	}
	return pixel;
}


void pc080sn_device::topspeed_custom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *color_ctrl_ram )
{
	UINT16 *dst16, *src16;
	UINT8 *tsrc;
	UINT16 scanline[1024];  /* won't be called by a wide-screen game, but just in case... */

	bitmap_ind16 &srcbitmap = m_tilemap[layer]->pixmap();
	bitmap_ind8 &flagsbitmap = m_tilemap[layer]->flagsmap();

	UINT16 a, color;
	int sx, x_index;
	int y_index, src_y_index, row_index;

	int flip = 0;

	int min_x = cliprect.min_x;
	int max_x = cliprect.max_x;
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	int screen_width = max_x - min_x + 1;
	int width_mask = 0x1ff; /* underlying tilemap */

	if (!flip)
	{
		sx = m_bgscrollx[layer] + 16 - m_x_offset;
		y_index = m_bgscrolly[layer] + min_y - m_y_offset;
	}
	else    // never used
	{
		sx = 0;
		y_index = 0;
	}

	for (int y = min_y; y <= max_y; y++)
	{
		src_y_index = y_index & 0x1ff;  /* tilemaps are 512 px up/down */
		row_index = (src_y_index - m_bgscrolly[layer]) & 0x1ff;
		color = color_ctrl_ram[(row_index + m_y_offset - 2) & 0xff];

		x_index = sx - (m_bgscroll_ram[layer][row_index]);

		src16 = &srcbitmap.pix16(src_y_index);
		tsrc  = &flagsbitmap.pix8(src_y_index);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (int i = 0; i < screen_width; i++)
			{
				a = src16[x_index & width_mask];
#ifdef TOPSPEED_ROAD_COLORS
				a = topspeed_get_road_pixel_color(a, color);
#endif
				*dst16++ = a;
				x_index++;
			}
		}
		else
		{
			for (int i = 0; i < screen_width; i++)
			{
				if (tsrc[x_index & width_mask])
				{
					a = src16[x_index & width_mask];
#ifdef TOPSPEED_ROAD_COLORS
					a = topspeed_get_road_pixel_color(a,color);
#endif
					*dst16++ = a;
				}
				else
					*dst16++ = 0x8000;
				x_index++;
			}
		}

		taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, screen.priority(), priority);
		y_index++;
	}
}

void pc080sn_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	m_tilemap[layer]->draw(screen, bitmap, cliprect, flags, priority);
}

void pc080sn_device::tilemap_draw_offset( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, int x_offset, int y_offset )
{
	int basedx = -16 - m_x_offset;
	int basedxflip = -16 + m_x_offset;
	int basedy = m_y_offset;
	int basedyflip = -m_y_offset;

	m_tilemap[layer]->set_scrolldx(basedx + x_offset, basedxflip + x_offset);
	m_tilemap[layer]->set_scrolldy(basedy + y_offset, basedyflip + y_offset);
	m_tilemap[layer]->draw(screen, bitmap, cliprect, flags, priority);
	m_tilemap[layer]->set_scrolldx(basedx, basedxflip);
	m_tilemap[layer]->set_scrolldy(basedy, basedyflip);
}

void pc080sn_device::tilemap_draw_special( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *ram )
{
	pc080sn_device::topspeed_custom_draw(screen, bitmap, cliprect, layer, flags, priority, ram);
}


void pc080sn_device::restore_scroll()
{
	int flip;

	m_bgscrollx[0] = -m_ctrl[0];
	m_bgscrollx[1] = -m_ctrl[1];
	m_bgscrolly[0] = -m_ctrl[2];
	m_bgscrolly[1] = -m_ctrl[3];

	flip = (m_ctrl[4] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	m_tilemap[0]->set_flip(flip);
	m_tilemap[1]->set_flip(flip);
}
