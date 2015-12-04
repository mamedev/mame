// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0150ROD
---------
Road generator. Two roads allow for forking. Gfx data fetched from ROM. Refer to notes below.
*/

#include "emu.h"
#include "tc0150rod.h"
#include "video/taito_helper.h"

#define TC0150ROD_RAM_SIZE 0x2000

const device_type TC0150ROD = &device_creator<tc0150rod_device>;

tc0150rod_device::tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0150ROD, "Taito TC0150ROD", tag, owner, clock, "tc0150rod", __FILE__),
	m_roadgfx(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0150rod_device::device_start()
{
	m_ram.resize(TC0150ROD_RAM_SIZE / 2);
	memset(&m_ram[0], 0, TC0150ROD_RAM_SIZE);
	save_item(NAME(m_ram));

	m_roadgfx = (UINT16 *)region()->base();
	assert(m_roadgfx);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ16_MEMBER( tc0150rod_device::word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( tc0150rod_device::word_w )
{
	COMBINE_DATA(&m_ram[offset]);
}

/******************************************************************************

    Memory map for TC0150ROD
    ------------------------

    0000-07ff  Road A, bank 0   [all are 256 lines]
    0800-0fff  Road A, bank 1
    1000-17ff  Road B, bank 0
    1800-1fff  Road B, bank 1

    1ffe-1fff  Control word
               ........ xxxxxxxx    Screen line where priority changes
               ......xx ........    Road A RAM page
               ....xx.. ........    Road B RAM page
               0000.... ........    Not used?



    Road ram line layout (thanks to Raine for original table)
    --------------------

    -----+-----------------+----------------------------------------
    Word | Bit(s)          |  Info
    -----+-----------------+----------------------------------------
      0  |x....... ........|  Draw background behind left road edge
      0  |.x...... ........|  Left road edge from road A has priority over road B ?? (+)
      0  |..x..... ........|  Left road edge from road A has priority over road B ?? (*)
      0  |...xx... ........|  Left edge/background palette entry offset
      0  |......xx xxxxxxxx|  Left edge   [pixels from road center] (@)
         |                 |
      1  |x....... ........|  Draw background behind right road edge
      1  |.x...... ........|  Right road edge from road A has priority over road B ??
      1  |..x..... ........|  Right road edge from road A has priority over road B ?? (*)
      1  |...xx... ........|  Right edge/background palette entry offset
      1  |......xx xxxxxxxx|  Right edge   [pixels from road center] (@)
         |                 |
      2  |x....... ........|  Draw background behind road body
      2  |.x...... ........|  Road line body from Road A has higher priority than Road B ??
      2  |..x..... ........|  Road line body from Road A has higher priority than Road B ??
      2  |...xx... ........|  Body/background palette entry offset
      2  |.....xxx xxxxxxxx|  X Offset   [offset is inverted] (^)
         |                 |
      3  |xxxx.... ........|  Color Bank  (selects group of 4 palette entries used for line)
      3  |....xxxx xxxxxxxx|  Road Gfx Tile number (top 2 bits not used by any game)
    -----+-----------------+-----------------------------------------

    @ size of bitmask suggested by Nightstr stage C when boss appears
    ^ bitmask confirmed in ChaseHQ code

    * see Nightstr "stage choice tunnel"
    + see Contcirc track at race start

    These priority bits have a different meaning in road B ram. They appear to mean
    that the relevant part of road B slips under road A. I.e. in road A they raise
    priority, in road B they lower it.

    We need a screenshot of Nightstr "stage choice tunnel" showing exactly what effect
    happens at top and bottom of screen while tunnel bifurcates.


Priority Levels - used by this code to represent the way the TC0150ROD appears to work
---------------

To speed up the code, three bits in the existing pixel-color map are used to store
priority information:

x....... ........ = transparency
.xxx.... ........ = pixel priority
....xxxx xxxxxxxx = pixel pen

There's a problem if any TaitoZ games using twice the palette space turn
up that also use TC0150ROD. This seems unlikely.

Pixel priority levels
---------------------
0 = bottom (used for off-edge i.e. background)
1,2,3,4 = ascending priority levels


Priority bits refer to: (edge a, body a, edge b, body b)

Standard:  bits=(0,0,0,0)
     edge a, body a, edge b, body b
     1       2       3       4

Contcirc:  bits=(0,1,0,0) (body a up by 2)
     edge a, edge b, body b, body a
     1       3       4       4

Nightstr bottom half:  bits=(0,1,1,0) (Contcirc PLUS edge b down by 2)
     edge b, edge a, body b, body a
     1       1       4       4

Nightstr top half:  bits=(1,0,0,1) (edge b down by 1, body a up by 1)
     edge a, edge b, body a, body b
     1       2       3       4

When numbers are the same, A goes on top...

These may need revising once Nightstr / Contcirc screenshots showing road
intersections are obtained.



Road info
---------

Road gfx is 2bpp, each word holds 8 pixels in this format:
xxxxxxxx ........  lo 8 bits
........ xxxxxxxx  hi 8 bits

The line gfx is back to front: this is why we call 'left' 'right' and vice versa in
this code: when the pixels are poked in they are done in reverse order, restoring
the orientation.

Each road gfx tile is 0x200 long in the rom. This comprises TWO road lines each
of 1024x1 pixels.

The first is the "edge" graphic. The second is the road body graphic. This means
separate sets of colors can be used for road edge and road body, giving greater
color variety.

The edge graphic is stored with the edges touching each other. So we must pull LHS
and RHS out separately to draw them.

Gfx lines: generally 0-0x1ff are the standard group (higher tile number indexes
wider lines). However this is just the way the games are: NOT a function of the
TC0150ROD.

Proof of background palette entry offset is in Contcirc in the tunnel on
Monaco level, the flyer screenshot shows different background colors on left
and right.

To investigate the weird road A/B priority system look at Nightstr and also
Contcirc. Contcirc: the "pit entry lane" in road B looks completely wrong if it is
allowed on top of road A body.

Aquajack road requires correct bank selection, or it goes crazy.

Should pen0 in Road A body be transparent? It seems necessary for Bshark round 6
and it makes Aquajack roads look much better. However, in Nightstr stage C
this results in a black band in the middle of the water. Also, it leaves
transparent areas in Dblaxle road which look obviously wrong. For time being a
parameter has been added to select which games use the transparency.

TODO
----

Contcirc: is road really meant to be so ugly when going uphill?

ChaseHQ: take right fork on level 1, at either edge of road you can see black
edge. Not obvious what is going wrong here. What color is it meant to be?

Nightstr: is the choice tunnel split correct? Need verification from machine.

Dblaxle: some stray background lines at top of road on occasional frames.

Sprite/road Y positions sometimes don't match. Maybe we need to use a zoom
lookup table from rom for the TaitoZ sprites.


******************************************************************************/

void tc0150rod_device::draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, UINT32 low_priority, UINT32 high_priority )
{
	#ifdef MAME_DEBUG
	static int dislayer[6]; /* Road Layer toggles to help get road correct */
	#endif

	int x_offs = 0xa7;  /* Increasing this shifts road to right */
	UINT16 scanline[512];
	UINT16 roada_line[512], roadb_line[512];
	UINT16 *dst16;
	UINT16 *roada, *roadb;

	UINT16 pixel, color, gfx_word;
	UINT16 roada_clipl, roada_clipr, roada_bodyctrl;
	UINT16 roadb_clipl, roadb_clipr, roadb_bodyctrl;
	UINT16 pri, pixpri;
	UINT8 priorities[6];
	int x_index, roadram_index, roadram2_index, i;
	int xoffset, paloffs, palloffs, palroffs;
	int road_gfx_tilenum, colbank, road_center;
	int road_ctrl = m_ram[0xfff];
	int left_edge, right_edge, begin, end, right_over, left_over;
	int line_needs_drawing, draw_top_road_line, background_only;

	int min_x = cliprect.min_x;
	int max_x = cliprect.max_x;
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	int screen_width = max_x - min_x + 1;

	int y = min_y;
#if 0
	int twin_road = 0;
#endif

	int road_A_address = y_offs * 4 + ((road_ctrl & 0x0300) << 2);  /* Index into roadram for road A */
	int road_B_address = y_offs * 4 + ((road_ctrl & 0x0c00) << 0);  /* Index into roadram for road B */

	int priority_switch_line = (road_ctrl & 0x00ff) - y_offs;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_X))
	{
		dislayer[0] ^= 1;
		popmessage("RoadA body: %01x",dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		dislayer[1] ^= 1;
		popmessage("RoadA l-edge: %01x",dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		dislayer[2] ^= 1;
		popmessage("RoadA r-edge: %01x",dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		dislayer[3] ^= 1;
		popmessage("RoadB body: %01x",dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		dislayer[4] ^= 1;
		popmessage("RoadB l-edge: %01x",dislayer[4]);
	}
	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		dislayer[5] ^= 1;
		popmessage("RoadB r-edge: %01x",dislayer[5]);
	}
#endif

#if 0
	if (1)
	{
		char buf3[80];
		sprintf(buf3,"road control: %04x",road_ctrl);
		popmessage(buf3);
	}
#endif

	do
	{
		line_needs_drawing = 0;

		roadram_index  = road_A_address + y * 4;    /* in case there is some switching mechanism (unlikely) */
		roadram2_index = road_B_address + y * 4;

		roada = roada_line;
		roadb = roadb_line;

		for (i = 0; i < screen_width; i++)  /* Default transparency fill */
		{
			*roada++ = 0x8000;
			*roadb++ = 0x8000;
		}

		/* l-edge a, r-edge a, body a, l-edge b, r-edge-b, body b */
		priorities[0] = 1;
		priorities[1] = 1;
		priorities[2] = 2;
		priorities[3] = 3;
		priorities[4] = 3;
		priorities[5] = 4;

		roada_clipr    = m_ram[roadram_index];
		roada_clipl    = m_ram[roadram_index + 1];
		roada_bodyctrl = m_ram[roadram_index + 2];
		roadb_clipr    = m_ram[roadram2_index];
		roadb_clipl    = m_ram[roadram2_index + 1];
		roadb_bodyctrl = m_ram[roadram2_index + 2];

		/* Not very logical, but seems to work */
		if (roada_bodyctrl & 0x2000)    priorities[2] += 2;
		if (roadb_bodyctrl & 0x2000)    priorities[2] += 1;
		if (roada_clipl    & 0x2000)    priorities[3] -= 1;
		if (roadb_clipl    & 0x2000)    priorities[3] -= 2;
		if (roada_clipr    & 0x2000)    priorities[4] -= 1;
		if (roadb_clipr    & 0x2000)    priorities[4] -= 2;

		if (priorities[4] == 0) priorities[4]++;    /* Fixes Aquajack LH edge dropping below background */

#if 0
		if ((roada_bodyctrl & 0x8000) || (roadb_bodyctrl & 0x8000))
			twin_road++;
#endif

		/********************************************************/
		/*                        ROAD A                        */
			/********************************************************/

		palroffs =(roada_clipr & 0x1000) >> 11;
		palloffs =(roada_clipl & 0x1000) >> 11;
		xoffset  = roada_bodyctrl & 0x7ff;
		paloffs  =(roada_bodyctrl & 0x1800) >> 11;
		colbank  =(m_ram[roadram_index + 3] & 0xf000) >> 10;
		road_gfx_tilenum = m_ram[roadram_index + 3] & 0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) & 0x7ff);
		left_edge = road_center - (roada_clipl & 0x3ff);        /* start pixel for left edge */
		right_edge = road_center + 1 + (roada_clipr & 0x3ff);   /* start pixel for right edge */

		if ((roada_clipl) || (roada_clipr)) line_needs_drawing = 1;

		/* Main road line is drawn from 'begin' to 'end'-1 */

		begin = left_edge + 1;
		if (begin < 0)
		{
			begin = 0;  /* can't begin off edge of screen */
		}

		end = right_edge;
		if (end > screen_width)
		{
			end = screen_width; /* can't end off edge of screen */
		}

		/* We need to offset start pixel we draw for road edge when edge of
		   road is partially or wholly offscreen on the opposite side
		   e.g. Contcirc attract */

		if (right_edge < 0)
		{
			right_over = -right_edge;
			right_edge = 0;
		}
		if (left_edge >= screen_width)
		{
			left_over = left_edge - screen_width + 1;
			left_edge = screen_width - 1;
		}

		/* If road is way off to right we only need to plot background */
		background_only = (road_center > (screen_width - 2 + 1024/2)) ? 1 : 0;


		/********* Draw main part of road *********/

		color = ((palette_offs + colbank + paloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[2] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[0])
#endif
		{
		/* Is this calculation imperfect ?  (0xa0 = screen width/2) */
		x_index = (-xoffset + x_offs + begin) & 0x7ff;

		roada = roada_line + screen_width - 1 - begin;

		if ((line_needs_drawing) && (begin < end))
		{
			for (i = begin; i < end; i++)
			{
				if (road_gfx_tilenum)   /* fixes Nightstr round C */
				{
					gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)   pixel = (pixel - 1) & 3;
						*roada-- = (color + pixel) | pri;
					}
					else    *roada-- = 0xf000;  /* priority transparency, fixes Bshark round 6 + Aquajack */
				}
				else roada--;

				x_index++;
				x_index &= 0x7ff;
			}
		}
		}


		/********* Draw 'left' road edge *********/

		color = ((palette_offs + colbank + palloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[0] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[2])
#endif
		{
		if (background_only)    /* The "road edge" line is entirely off screen so can't be drawn */
		{
			if (roada_clipl & 0x8000)   /* but we may need to fill in the background color */
			{
				roada = roada_line;
				for (i = 0; i < screen_width; i++)
				{
					*roada++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024 / 2 - 1 - left_over) & 0x7ff;

				roada = roada_line + screen_width - 1 - left_edge;

				if (line_needs_drawing)
				{
					for (i = left_edge; i >= 0; i--)
					{
						gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

						pixpri = (pixel == 0) ? (0) : (pri);    /* off edge has low priority */

						if ((pixel == 0) && !(roada_clipl & 0x8000))
						{
							roada++;
						}
						else
						{
							if (type)   pixel = (pixel - 1)&3;
							*roada++ = (color + pixel) | pixpri;
						}

						x_index--;
						x_index &= 0x7ff;
					}
				}
			}
		}
		}


		/********* Draw 'right' road edge *********/

		color = ((palette_offs + colbank + palroffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[1] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[1])
#endif
		{
		if ((right_edge < screen_width) && (right_edge >= 0))
		{
			x_index = (1024 / 2 + right_over) & 0x7ff;

			roada = roada_line + screen_width - 1 - right_edge;

			if (line_needs_drawing)
			{
				for (i = right_edge; i < screen_width; i++)
				{
					gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					pixpri = (pixel == 0) ? (0) : (pri);    /* off edge has low priority */

					if ((pixel == 0) && !(roada_clipr & 0x8000))
					{
						roada--;
					}
					else
					{
						if (type)   pixel = (pixel - 1) & 3;
						*roada-- = (color + pixel) | pixpri;
					}

					x_index++;
					x_index &= 0x7ff;
				}
			}
		}
		}


		/********************************************************/
		/*                        ROAD B                        */
			/********************************************************/

		palroffs = (roadb_clipr & 0x1000) >> 11;
		palloffs = (roadb_clipl & 0x1000) >> 11;
		xoffset  =  roadb_bodyctrl & 0x7ff;
		paloffs  = (roadb_bodyctrl & 0x1800) >> 11;
		colbank  = (m_ram[roadram2_index + 3] & 0xf000) >> 10;
		road_gfx_tilenum = m_ram[roadram2_index + 3] & 0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) & 0x7ff);

// ChaseHQ glitches on right when road rejoins:
// de7, de8 causes problems => 5e7/e8
// 5ff - (a7 - 5e7)
// 5ff - 2c0 = 33f / 340 which is not quite > screenwidth + 1024/2: so we subtract 2 more, fixed


// ChaseHQ glitches on right when road rejoins:
// 0a6 and lower => 0x5ff 5fe etc.
// 35c => 575 right road edge wraps back onto other side of screen
// 5ff-54a       through    5ff-331
// b6            through    2ce
// 2a6 through 0 through    5a7 ??

		left_edge = road_center - (roadb_clipl & 0x3ff);        /* start pixel for left edge */
		right_edge = road_center + 1 + (roadb_clipr & 0x3ff);   /* start pixel for right edge */

		if (((roadb_clipl) || (roadb_clipr)) && ((road_ctrl & 0x800) || (type == 2)))
		{
			draw_top_road_line = 1;
			line_needs_drawing = 1;
		}
		else    draw_top_road_line = 0;

		/* Main road line is drawn from 'begin' to 'end'-1 */

		begin = left_edge + 1;
		if (begin < 0)
		{
			begin = 0;  /* can't begin off edge of screen */
		}

		end = right_edge;
		if (end > screen_width)
		{
			end = screen_width; /* can't end off edge of screen */
		}

		/* We need to offset start pixel we draw for road edge when edge of
		   road is partially or wholly offscreen on the opposite side
		   e.g. Contcirc attract */

		if (right_edge < 0)
		{
			right_over = -right_edge;
			right_edge = 0;
		}
		if (left_edge >= screen_width)
		{
			left_over = left_edge - screen_width + 1;
			left_edge = screen_width - 1;
		}

		/* If road is way off to right we only need to plot background */
		background_only = (road_center > (screen_width - 2 + 1024/2)) ? 1 : 0;


		/********* Draw main part of road *********/

		color = ((palette_offs + colbank + paloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[5] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[3])
#endif
		{
		/* Is this calculation imperfect ?  (0xa0 = screen width/2) */
		x_index = (-xoffset + x_offs + begin) & 0x7ff;

		if (x_index > 0x3ff)    /* Second half of gfx contains the road body line */
		{
			roadb = roadb_line + screen_width - 1 - begin;

			if (draw_top_road_line && road_gfx_tilenum && (begin < end))
			{
				for (i = begin; i < end; i++)
				{
					gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)   pixel = (pixel - 1) & 3;
						*roadb-- = (color + pixel) | pri;
					}
					else    *roadb-- = 0xf000;  /* high priority transparency, fixes Aquajack */

					x_index++;
					x_index &= 0x7ff;
				}
			}
		}
		}


		/********* Draw 'left' road edge *********/

		color = ((palette_offs + colbank + palloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[3] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[5])
#endif
		{
		if (background_only)    /* The "road edge" line is entirely off screen so can't be drawn */
		{
			if ((roadb_clipl & 0x8000) && draw_top_road_line)   /* but we may need to fill in the background color */
			{
				roadb = roadb_line;
				for (i = 0; i < screen_width; i++)
				{
					*roadb++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024 / 2 - 1 - left_over) & 0x7ff;

				roadb = roadb_line + screen_width - 1 - left_edge;

				if (draw_top_road_line)     // rename to draw_roadb_line !?
				{
					for (i = left_edge; i >= 0; i--)
					{
						gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

						pixpri = (pixel == 0) ? (0) : (pri);    /* off edge has low priority */

						if ((pixel == 0) && !(roadb_clipl & 0x8000))    /* test for background disabled */
						{
							roadb++;
						}
						else
						{
							if (type)   pixel = (pixel - 1) & 3;
							*roadb++ = (color + pixel) | pixpri;
						}

						x_index--;
						if (x_index < 0)    break;
					}
				}
			}
		}
		}


		/********* Draw 'right' road edge *********/

		color = ((palette_offs + colbank + palroffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[4] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[4])
#endif
		{
		if ((right_edge < screen_width) && (right_edge >= 0))
		{
			x_index = (1024 / 2 + right_over) & 0x7ff;

			roadb = roadb_line + screen_width - 1 - right_edge;

			if (draw_top_road_line)
			{
				for (i = right_edge; i < screen_width; i++)
				{
					gfx_word = m_roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					pixpri = (pixel == 0) ? (0) : (pri);    /* off edge has low priority */

					if ((pixel == 0) && !(roadb_clipr & 0x8000))    /* test for background disabled */
					{
						roadb--;
					}
					else
					{
						if (type)   pixel = (pixel - 1) & 3;
						*roadb-- =  (color + pixel) | pixpri;
					}

					x_index++;
					if (x_index > 0x3ff)    break;
				}
			}
		}
		}


		/******** Combine the two lines according to pixel priorities ********/

		if (line_needs_drawing)
		{
			dst16 = scanline;

			for (i = 0; i < screen_width; i++)
			{
				if (roada_line[i] == 0x8000)    /* road A pixel transparent */
				{
					*dst16++ = roadb_line[i] & 0x8fff;
				}
				else if (roadb_line[i] == 0x8000)   /* road B pixel transparent */
				{
					*dst16++ = roada_line[i] & 0x8fff;
				}
				else    /* two competing pixels, which has highest priority... */
				{
					if ((roadb_line[i] & 0x7000) > (roada_line[i] & 0x7000))
					{
						*dst16++ = roadb_line[i] & 0x8fff;
					}
					else
					{
						*dst16++ = roada_line[i] & 0x8fff;
					}
				}
			}

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, 1, ROT0, priority_bitmap, (y > priority_switch_line) ? high_priority : low_priority);
		}

		y++;
	}
	while (y <= max_y);

#if 0
	if (twin_road)  // I don't know what this means, actually...
	{
		char buf2[80];
		sprintf(buf2, "Road twinned for %04x lines", twin_road);
		popmessage(buf2);
	}
#endif
}
