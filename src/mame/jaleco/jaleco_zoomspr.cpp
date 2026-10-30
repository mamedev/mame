// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Jaleco Zoomable Sprite Generator Hardware

    used by:
    - jaleco/cischeat.cpp
    - jaleco/scudhamm.cpp

    TODO:
    - Verify priority bits

    -- Original docs from jaleco/cischeat.cpp:

    Sprites are made of several 16x16 tiles (up to 256x256 pixels)
    and can be zoomed in and out. See below for sprite RAM format.

***************************************************************************/

#include "emu.h"
#include "jaleco_zoomspr.h"

DEFINE_DEVICE_TYPE(JALECO_ZOOMSPR,        jaleco_zoomspr_device,        "jaleco_zoomspr",        "Jaleco Zoomable Sprite Generator")
DEFINE_DEVICE_TYPE(JALECO_ZOOMSPR_BIGRUN, jaleco_zoomspr_bigrun_device, "jaleco_zoomspr_bigrun", "Jaleco Zoomable Sprite Generator (Big Run configuration)")

jaleco_zoomspr_device::jaleco_zoomspr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_drawmode_table{0}
{
}

jaleco_zoomspr_device::jaleco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaleco_zoomspr_device(mconfig, JALECO_ZOOMSPR, tag, owner, clock)
{
}

jaleco_zoomspr_bigrun_device::jaleco_zoomspr_bigrun_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jaleco_zoomspr_device(mconfig, JALECO_ZOOMSPR_BIGRUN, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start: Start up the device
//-------------------------------------------------

void jaleco_zoomspr_device::device_start()
{
	for (int i = 0; i < 16; i++)
		m_drawmode_table[i] = DRAWMODE_SOURCE;

	m_drawmode_table[0] = DRAWMODE_SHADOW;
	m_drawmode_table[15] = DRAWMODE_NONE;
}

void jaleco_zoomspr_device::device_reset()
{
}

/***************************************************************************

                Cisco Heat & F1 GP Star Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02/04   fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Flip X/Y
            ---- ba9- ---- ----     ? X/Y zoom ?
            ---- ---8 7654 3210     X/Y zoom

    06/08   fedc ba-- ---- ----     ? X/Y position ?
            ---- --98 7654 3210     X/Y position

    0A                              0 ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 7--- ----     unused?
            ---- ---- -654 3210     Color

***************************************************************************/

constexpr int jaleco_zoomspr_shrink(int org, int fact)
{
	return ((org << 16) * (fact & 0x01ff)) >> 7;
}

/*  Draw sprites, in the given priority range, to a bitmap.

    Priorities between 0 and 15 cover sprites whose priority nibble
    is between 0 and 15. Priorities between 0+16 and 15+16 cover
    sprites whose priority nibble is between 0 and 15 and whose
    colour code's high bit is set.  */

void jaleco_zoomspr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, const u16 *source, u32 ramsize)
{
	u16 const *const finish = source + ramsize;

	/* Move the priority values in place */
	const bool high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 &= 0xf;
	priority2 &= 0xf;

	const int min_priority = std::min(priority1, priority2);
	const int max_priority = std::max(priority1, priority2);

	for (; source < finish; source += 0x10 / 2)
	{
		const u16 size = source[0];
		if (BIT(size, 12))
			continue;

		const u16 attr = source[7];
		const int pri = (attr & 0x700) >> 8; // (attr & 0xf00) >> 8 ?

		/* high byte is priority information */
		if ((pri < min_priority) || (pri > max_priority))
			continue;

		const u32 color = attr & 0x007f;
		if (high_sprites && !BIT(color, 7))
			continue;

		const u16 xzoom = source[1];
		const u16 yzoom = source[2];

		/* dimension of a tile after zoom */
		const int xdim = jaleco_zoomspr_shrink(16, xzoom);
		const int ydim = jaleco_zoomspr_shrink(16, yzoom);

		if (((xdim >> 16) == 0) || ((ydim >> 16) == 0))
			continue;

		const bool shadow = BIT(attr, 12);

		/* number of tiles */
		const u8 xnum = ((size & 0x0f) >> 0) + 1;
		const u8 ynum = ((size & 0xf0) >> 4) + 1;

		const bool flipx = BIT(xzoom, 12);
		const bool flipy = BIT(yzoom, 12);

		int sx = source[3];
		int sy = source[4];
		// TODO: was & 0x1ff with 0x200 as sprite wrap sign, looks incorrect with Grand Prix Star
		//       during big car on side view in attract mode (a tyre gets stuck on the right of the screen)
		//       this arrangement works with both games (otherwise Part 2 gets misaligned bleachers sprites)
		sx = util::sext(sx & 0x7ff, 11/*10 ?*/);
		sy = util::sext(sy & 0x7ff, 11/*10 ?*/);

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

		/* the y pos passed to the hardware is the that of the last line,
		   we need the y pos of the first line  */
		sy -= (ydim * ynum);

		u32 code = source[6];
		int xscale = xdim >> 4;
		int yscale = ydim >> 4;

		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1 << 16) >> 4;
		if (yscale & 0xffff)    yscale += (1 << 16) >> 4;

		int xstart, xend, xinc;
		if (flipx)  { xstart = xnum - 1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;         xend = xnum;  xinc = +1; }

		int ystart, yend, yinc;
		if (flipy)  { ystart = ynum - 1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;         yend = ynum;  yinc = +1; }

		m_drawmode_table[0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (int y = ystart; y != yend; y += yinc)
		{
			for (int x = xstart; x != xend; x += xinc)
			{
				gfx(0)->zoom_transtable(bitmap, cliprect,
						code++, color,
						flipx, flipy,
						(sx + x * xdim) >> 16, (sy + y * ydim) >> 16,
						xscale, yscale, m_drawmode_table);
			}
		}
	}   /* end sprite loop */
}


/***************************************************************************

                            Big Run Sprites Drawing

    Offset: Bits:                   Value:

    00      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Don't display this sprite
            ---- ba98 ---- ----     unused?
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    02      fedc ba98 ---- ----     Y zoom
            ---- ---- 7654 3210     X zoom

    04/06   fed- ---- ---- ----
            ---c ---- ---- ----     X/Y flip
            ---- ba9- ---- ----
            ---- ---8 7654 3210     X/Y position (signed)

    08                              ?
    0A                              ?

    0C                              Code

    0E      fed- ---- ---- ----     unused?
            ---c ---- ---- ----     Use pen 0 as shadow
            ---- ba98 ---- ----     Priority
            ---- ---- 76-- ----     unused?
            ---- ---- --54 3210     Color

***************************************************************************/

void jaleco_zoomspr_bigrun_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, const u16 *source, u32 ramsize)
{
	u16 const *const finish = source + ramsize;

	/* Move the priority values in place */
	const bool high_sprites = (priority1 >= 16) | (priority2 >= 16);
	priority1 &= 0xf;
	priority2 &= 0xf;

	const int min_priority = std::min(priority1, priority2);
	const int max_priority = std::max(priority1, priority2);

	for (; source < finish; source += 0x10 / 2)
	{
		const u16 size = source[0];
		if (BIT(size, 12))
			continue;

		const u16 attr = source[7];
		const int pri = (attr & 0x700) >> 8; // (attr & 0xf00) >> 8 ?

		/* high byte is a priority information */
		if ((pri < min_priority) || (pri > max_priority))
			continue;

		const u32 color = attr & 0x007f;
		if (high_sprites && !BIT(color, 7))
			continue;

		const u8 yzoom = (source[1] >> 8) & 0xff;
		const u8 xzoom = (source[1] >> 0) & 0xff;

		/* dimension of a tile after zoom */
		const int xdim = jaleco_zoomspr_shrink(16, xzoom);
		const int ydim = jaleco_zoomspr_shrink(16, yzoom);

		if (((xdim >> 16) == 0) || ((ydim >> 16) == 0))    continue;

		const bool shadow = BIT(attr, 12);

		/* number of tiles */
		const u8 xnum = ((size & 0x0f) >> 0) + 1;
		const u8 ynum = ((size & 0xf0) >> 4) + 1;

		int sx = source[2];
		int sy = source[3];
		const bool flipx = BIT(sx, 12);
		const bool flipy = BIT(sy, 12);
//      sx = util::sext(sx & 0x3ff, 10);
//      sy = util::sext(sy & 0x3ff, 10);
		sx = util::sext(sx & 0x1ff, 9);
		sy = util::sext(sy & 0x1ff, 9);

		/* use fixed point values (16.16), for accuracy */
		sx <<= 16;
		sy <<= 16;

//      sy -= (ydim * ynum);

		u32 code = source[6];
		int xscale = xdim >> 4;
		int yscale = ydim >> 4;

		/* let's approximate to the nearest greater integer value
		   to avoid holes in between tiles */
		if (xscale & 0xffff)    xscale += (1 << 16) >> 4;
		if (yscale & 0xffff)    yscale += (1 << 16) >> 4;

		int xstart, xend, xinc;
		if (flipx)  { xstart = xnum - 1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;         xend = xnum;  xinc = +1; }

		int ystart, yend, yinc;
		if (flipy)  { ystart = ynum - 1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;         yend = ynum;  yinc = +1; }

		m_drawmode_table[0] = shadow ? DRAWMODE_SHADOW : DRAWMODE_SOURCE;

		for (int y = ystart; y != yend; y += yinc)
		{
			for (int x = xstart; x != xend; x += xinc)
			{
				gfx(0)->zoom_transtable(bitmap, cliprect,
						code++, color,
						flipx, flipy,
						(sx + x * xdim) >> 16, (sy + y * ydim) >> 16,
						xscale, yscale, m_drawmode_table);
			}
		}
	}   /* end sprite loop */
}
