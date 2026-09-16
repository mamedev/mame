// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Jaleco Road Generator Hardware

    1024 pixel width road layer generator
    later hardware has horizontal zoom support.

    used by:
    - jaleco/cischeat.cpp

    TODO:
    - Verify priority behaviours
    - Use the Tilemap Manager for the road layers (when this kind of layers
      will be supported) for performance and better priority support.
      A line based zooming is additionally needed for f1gpstar.

    -- Original docs from jaleco/cischeat.cpp:

    Each of the 256 (not all visible) lines of the screen
    can display any of the lines of gfx in ROM, which are
    larger than the screen and can therefore be scrolled

                                    Cisco Heat              F1 GP Star
                Line Width          1024                    1024
                Zoom                No                      Yes

***************************************************************************/

#include "emu.h"
#include "jaleco_road.h"

/* horizontal size of 1 line and 1 tile of the road */
static constexpr int X_SIZE_SHIFT = 10;
static constexpr int X_SIZE = 1 << X_SIZE_SHIFT;
static constexpr int X_SIZE_MASK = X_SIZE - 1;

static constexpr int TILE_SIZE_SHIFT = 6;
static constexpr int TILE_SIZE = 1 << TILE_SIZE_SHIFT;
static constexpr int TILE_SIZE_MASK = TILE_SIZE - 1;


DEFINE_DEVICE_TYPE(JALECO_ROAD,      jaleco_road_device,      "jaleco_road",      "Jaleco Road Generator")
DEFINE_DEVICE_TYPE(JALECO_ZOOM_ROAD, jaleco_zoom_road_device, "jaleco_zoom_road", "Jaleco Zoomable Road Generator")

jaleco_road_device::jaleco_road_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_roadram(*this, DEVICE_SELF)
	, m_colorbase(0)
{
}

jaleco_road_device::jaleco_road_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaleco_road_device(mconfig, JALECO_ROAD, tag, owner, clock)
{
}

jaleco_zoom_road_device::jaleco_zoom_road_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaleco_road_device(mconfig, JALECO_ZOOM_ROAD, tag, owner, clock)
{
}


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const u32 road_layout_xoffset[64] =
{
	STEP64(0,4)
};

/* Road: 64 x 1 x 4 */
static const gfx_layout road_layout =
{
	64,1,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	EXTENDED_XOFFS,
	{ 0 },
	64*1*4,
	road_layout_xoffset,
	nullptr
};

GFXDECODE_MEMBER(jaleco_road_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, road_layout, 0, 64)
GFXDECODE_END

//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void jaleco_road_device::device_start()
{
	// decode our graphics
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colorbase);
}

void jaleco_road_device::device_reset()
{
}

/***************************************************************************


                                Road Drawing


***************************************************************************/

/**************************************************************************

                        Cisco Heat road format


    Offset:     Bits:                   Value:

    00.w                                Code

    02.w                                X Scroll

    04.w        fedc ---- ---- ----     unused?
                ---- ba98 ---- ----     Priority
                ---- ---- 76-- ----     unused?
                ---- ---- --54 3210     Color

    06.w                                Unused

**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void jaleco_road_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, bool transparency)
{
	const int min_y = cliprect.min_y;
	const int max_y = cliprect.max_y;

	const int max_x = cliprect.max_x;

	int min_priority, max_priority;

	if (priority1 < priority2)  { min_priority = priority1; max_priority = priority2; }
	else                        { min_priority = priority2; max_priority = priority1; }

	/* Move the priority values in place */
	min_priority &= 7;
	max_priority &= 7;

	/* Let's draw from the top to the bottom of the visible screen */
	for (int sy = min_y; sy <= max_y; sy++)
	{
		const u16 attr = m_roadram[sy * 4 + 2];
		const int prival = (attr & 0x700) >> 8;

		/* high byte is a priority information */
		if ((prival < min_priority) || (prival > max_priority))
			continue;

		u32 code    = m_roadram[sy * 4 + 0];
		int xscroll = m_roadram[sy * 4 + 1];

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code = code << (X_SIZE_SHIFT - TILE_SIZE_SHIFT);

		xscroll &= X_SIZE_MASK;
		u32 curr_code = code + (xscroll >> TILE_SIZE_SHIFT);

		for (int sx = -(xscroll & TILE_SIZE_MASK); sx <= max_x; sx += TILE_SIZE)
		{
			gfx(0)->transpen(bitmap,cliprect,
					curr_code++,
					attr,
					0,0,
					sx,sy,
					transparency ? 15 : -1);

			/* wrap around */
			if ((curr_code & (X_SIZE_MASK >> TILE_SIZE_SHIFT)) == 0)
				curr_code = code;
		}
	}
}


/**************************************************************************

                        F1 GrandPrix Star road format

    Offset:     Bits:                   Value:

    00.w        fedc ---- ---- ----     Priority
                ---- ba98 7654 3210     X Scroll (After Zoom)

    02.w        fedc ba-- ---- ----     unused?
                ---- --98 7654 3210     X Zoom

    04.w        fe-- ---- ---- ----     unused?
                --dc ba98 ---- ----     Color
                ---- ---- 7654 3210     ?

    06.w                                Code


    Imagine an "empty" line, 2 * X_SIZE wide, with the gfx from
    the ROM - whose original size is X_SIZE - in the middle.

    Zooming acts on this latter and can shrink it to 1 pixel or
    widen it to 2 * X_SIZE, while *keeping it centered* in the
    empty line. Scrolling acts on the resulting line.


**************************************************************************/

/*  Draw the road in the given bitmap. The priority1 and priority2 parameters
    specify the range of lines to draw  */

void jaleco_zoom_road_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, bool transparency)
{
	const int min_y = cliprect.min_y;
	const int max_y = cliprect.max_y;

	const int max_x = cliprect.max_x << 16;   // use fixed point values (16.16), for accuracy

	int min_priority, max_priority;

	if (priority1 < priority2)  { min_priority = priority1; max_priority = priority2; }
	else                        { min_priority = priority2; max_priority = priority1; }

	/* Move the priority values in place */
	min_priority &= 7;
	max_priority &= 7;

	/* Let's draw from the top to the bottom of the visible screen */
	for (int sy = min_y; sy <= max_y; sy++)
	{
		int xscroll = m_roadram[sy * 4 + 0];
		const int prival = (xscroll & 0x7000) >> 12;
		/* highest nibble is a priority information */
		if ((prival < min_priority) || (prival > max_priority))
			continue;

		const int xzoom = m_roadram[sy * 4 + 1];
		const u16 attr  = m_roadram[sy * 4 + 2];
		u32 code        = m_roadram[sy * 4 + 3];

		/* zoom code range: 000-3ff     scale range: 0.0-2.0 */
		int xscale = (((xzoom & 0x3ff) + 1) << (16 + 1)) / 0x400;

		/* line number converted to tile number (each tile is TILE_SIZE x 1) */
		code = code << (X_SIZE_SHIFT - TILE_SIZE_SHIFT);

		/* dimension of a tile after zoom */
		const int xdim = xscale << TILE_SIZE_SHIFT;

		xscroll %= 2 * X_SIZE;

		int xstart = (X_SIZE - xscroll) << 16;
		xstart -= (X_SIZE * xscale) >> 1;

		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		xscale += (1 << 16) >> TILE_SIZE_SHIFT;

		/* Draw the line */
		for (int sx = xstart; sx <= max_x; sx += xdim)
		{
			gfx(0)->zoom_transpen(bitmap,cliprect,
						code++,
						attr >> 8,
						0,0,
						sx >> 16, sy,
						xscale, 1 << 16,
						transparency ? 15 : -1);

			/* stop when the end of the line of gfx is reached */
			if ((code & (X_SIZE_MASK >> TILE_SIZE_SHIFT)) == 0)
				break;
		}
	}
}
