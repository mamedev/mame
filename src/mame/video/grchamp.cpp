// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Phil Stroffolino,Hans Andersson
/*  Video hardware for Taito Grand Champion */

/* updated by Hans Andersson, dec 2005     */
#include "emu.h"
#include "video/resnet.h"
#include "includes/grchamp.h"

#define FOG_SIZE 70



#define RGB_MAX     191

PALETTE_INIT_MEMBER(grchamp_state, grchamp)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[3] = { 100, 270, 470 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, RGB_MAX, -1.0,
			3,  &resistances[0], rweights, 0, 100,
			3,  &resistances[0], gweights, 0, 100,
			2,  &resistances[0], bweights, 0, 100);

	/* initialize the palette with these colors */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 1;
		bit1 = (color_prom[i] >> 1) & 1;
		bit2 = (color_prom[i] >> 2) & 1;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 1;
		bit1 = (color_prom[i] >> 4) & 1;
		bit2 = (color_prom[i] >> 5) & 1;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 1;
		bit1 = (color_prom[i] >> 7) & 1;
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE8_MEMBER(grchamp_state::left_w)
{
	m_leftram[offset] = data;
	m_left_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(grchamp_state::center_w)
{
	m_centerram[offset] = data;
	m_center_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(grchamp_state::right_w)
{
	m_rightram[offset] = data;
	m_right_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(grchamp_state::get_text_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(grchamp_state::get_left_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_leftram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(grchamp_state::get_right_tile_info)
{
	SET_TILE_INFO_MEMBER(2, m_rightram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(grchamp_state::get_center_tile_info)
{
	SET_TILE_INFO_MEMBER(3, m_centerram[tile_index], 0, 0);
}

TILEMAP_MAPPER_MEMBER(grchamp_state::get_memory_offset)
{
	return (col % 32) + row * 32 + (col / 32) * 32*32;
}


void grchamp_state::video_start()
{
	m_work_bitmap.allocate(32,32);

	/* allocate tilemaps for each of the three sections */
	m_text_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(grchamp_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS,  8,8, 32,32);
	m_left_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(grchamp_state::get_left_tile_info),this), tilemap_mapper_delegate(FUNC(grchamp_state::get_memory_offset),this),  8,8, 64,32);
	m_right_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(grchamp_state::get_right_tile_info),this), tilemap_mapper_delegate(FUNC(grchamp_state::get_memory_offset),this),  8,8, 64,32);
	m_center_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(grchamp_state::get_center_tile_info),this), tilemap_mapper_delegate(FUNC(grchamp_state::get_memory_offset),this),  8,8, 64,32);
}

#if 0
int grchamp_state::collision_check(bitmap_ind16 &bitmap, int which )
{
	int bgcolor = m_palette->pen(0);
	int sprite_transp = m_palette->pen(0x24);
	const rectangle &visarea = m_screen->visible_area();
	int y0 = 240 - m_cpu0_out[3];
	int x0 = 256 - m_cpu0_out[2];
	int x,y,sx,sy;
	int pixel;
	int result = 0;

	if( which==0 )
	{
		/* draw the current player sprite into a work bitmap */

			m_gfxdecode->gfx(4)->opaque(m_work_bitmap,
			m_work_bitmap.cliprect(),
			m_cpu0_out[4]&0xf,
			1, /* color */
			0,0,
			0,0 );
	}

	for( y = 0; y <32; y++ )
	{
		for( x = 0; x<32; x++ )
		{
			pixel = m_work_bitmap.pix16(y, x);
			if( pixel != sprite_transp ){
				sx = x+x0;
				sy = y+y0;
				if(visarea->contains(sx, sy))
				{
					// Collision check uses only 16 pens!
					pixel = bitmap.pix16(sy, sx) % 16;
					if( pixel != bgcolor )
					{
						result = 1; /* flag collision */
						/*  wipe this pixel, so collision checks with the
						**  next layer work */
						bitmap.pix16(sy, sx) = bgcolor;
					}
				}
			}

		}
	}
	return result?(1<<which):0;
}

void grchamp_state::draw_fog(bitmap_ind16 &bitmap, const rectangle &cliprect, int fog)
{
	int x,y,offs;

	// Emulation of analog fog effect
	for (x = 0; x < 100; x++)
	{
		offs = 0x40;
		if(x > (100-FOG_SIZE-1))
			offs = 0x40*(x-(100-FOG_SIZE-1));
		for(y=16;y<240;y++)
		{
			bitmap.pix16(y, x) = bitmap.pix16(y, x) + offs;
		}
	}
}

void grchamp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(5);
	int bank = (m_cpu0_out[0] & 0x20) ? 0x40 : 0x00;
	const UINT8 *source = m_spriteram + 0x40;
	const UINT8 *finish = source + 0x40;

	while (source < finish)
	{
		int sx = source[3];
		int sy = 240-source[0];
		int color = source[2];
		int code = source[1);

			gfx->transpen(bitmap,cliprect,
			bank + (code & 0x3f),
			color,
			code & 0x40,
			code & 0x80,
			sx,sy, 0);
		source += 4;
	}
}
#endif


void grchamp_state::draw_objects(int y, UINT8 *objdata)
{
/*
    CPU 5/7:

        TOP-LEFT:
            2xLS163 counters
                /CNTRLD loads HPOSI0-7
                /CNTRCLR | /256H clears (i.e., cleared on /CNTRCLR only during HBLANK)
                clocks on 6MHz

            Output from counters is inverted if /256H = 1 (during visible area)

            Becomes address in line buffer

        TOP-CENTER: (POINT "A" == output from 74LS02 pin 13)
            0 during visible area (always)
            0 during HBLANK if:
                2xLS163 counters & 0xf8 == 0xf8 or
                2xLS163 counters & 0xf8 == 0x00
            When 0, forces /RC0, /RC1, /RC2, /RV0, /RV1 to 1

        When drawing (during HBLANK):
            /RC0 = 0 iff (RAW0 | RAW1) & (prev /RC11) & (prev /RC22) & HPOSI0
            /RC1 = 0 iff (RAW0 | RAW1) & (prev /RC00) & (prev /RC22) & HPOSI1
            /RC2 = 0 iff (RAW0 | RAW1) & (prev /RC00) & (prev /RC11) & HPOSI2
            /RV0 = RAW0 | (prev bit value & !(RAW0 | RAW1))
            /RV1 = RAW1 | (prev bit value & !(RAW0 | RAW1))



        /RC00 = (RAW0|RAW1) | DOUT(2)
        /RC11 = (RAW0|RAW1) | DOUT(3)
        /RC22 = (RAW0|RAW1) | DOUT(4)


*/
	const UINT8 *prom = memregion("proms")->base() + 0x20;
	gfx_element *gfx;
	int change = (m_cpu0_out[0] & 0x20) << 3;
	int num;

	/* first clear to 0; this is done as the previous scanline was scanned */
	memset(objdata, 0, 256);

	/* now draw the sprites; this is done during HBLANK */
	gfx = m_gfxdecode->gfx(4);
	for (num = 0; num < 16; num++)
	{
		/*
		    Each sprite is 4 bytes. The logic reads one byte every 2H:
		        5C,7D,5E,7F, 5C,5D,5E,5F, 58,79,5A,7B, 58,59,5A,5B,
		        54,75,56,77, 54,55,56,57, 50,71,52,73, 50,51,52,53,
		        4C,6D,4E,6F, 4C,4D,4E,4F, 48,69,4A,6B, 48,49,4A,4B,
		        44,65,46,67, 44,45,46,47, 40,61,42,63, 40,41,42,43,
		*/
		int dataoffs = ((~num & 0x0e) << 1) | ((~num & 0x01) << 5);

		/* the first of the 4 bytes is the Y position; this is used to match the scanline */
		/* we match this scanline if the sum & 0xf0 == 0 */
		int sy = m_spriteram[0x40 + (dataoffs & ~0x20)];
		int dy = sy + ~y;
		if ((dy & 0xf0) == 0)
		{
			/* the second byte is: code is in bits 0-5, xflip in bit 6, yflip in bit 7 */
			/* note that X flip is reversed (on purpose) */
			int codeflip = m_spriteram[0x41 + dataoffs];
			int code = (codeflip & 0x3f) + (change >> 2);
			int yflip = (codeflip & 0x80) ? 0x0f : 0x00;
			int xflip = (codeflip & 0x40) ? 0x0f : 0x00;
			const UINT8 *src = gfx->get_data(code) + ((dy ^ yflip) & 15) * gfx->rowbytes();

			/* the third byte is: color in bits 0-2 */
			int color = (m_spriteram[0x42 + (dataoffs & ~0x20)] & 0x07) << 2;

			/* the fourth byte is the X position */
			int sx = m_spriteram[0x43 + dataoffs];
			int x;

			/* draw 16 pixels */
			for (x = 0; x < 16; x++)
			{
				int dx = ~(x + sx) & 0xff;

				/* the line buffer circuit clips between $08 and $F8 */
				if (dx >= 0x08 && dx < 0xf8)
				{
					int pix = src[x ^ xflip];

					/* only non-zero pixels are written */
					if (pix != 0)
						objdata[dx] = pix | color;
				}
			}
		}
	}

	/* finally draw the text characters; this is done as we read out the object buffers */
	gfx = m_gfxdecode->gfx(0);
	for (num = 0; num < 32; num++)
	{
		/*
		    The logic reads one byte every 4H, 64 bytes total:
		        3E,3F,3C,3D, 3A,3B,38,39, 36,37,34,35, 32,33,30,31,
		        2E,2F,2C,2D, 2A,2B,28,29, 26,27,24,25, 22,23,20,21,
		        1E,1F,1C,1D, 1A,1B,18,19, 16,17,14,15, 12,13,10,11,
		        0E,0F,0C,0D, 0A,0B,08,09, 06,07,04,05, 02,03,00,01
		*/
		int hprime = num ^ 0x1f;
		int dataoffs = hprime << 1;
		int sy = m_spriteram[0x00 + dataoffs];
		int dy = sy + ~y;
		int color = (m_spriteram[0x01 + dataoffs] & 0x07) << 2;
		int code = m_videoram[hprime | ((dy & 0xf8) << 2)] + change;
		const UINT8 *src = gfx->get_data(code) + (dy & 7) * gfx->rowbytes();
		int x;

		/* draw 8 pixels */
		for (x = 0; x < 8; x++)
		{
			int pix = src[x ^ 7];

			/* look up the final result in the PROM */
			/*    bit 4 = CHARAC (we add this in) */
			/*    bit 3 = /OBJECT */
			/*    bit 2 = /RADA */
			/*    bit 1 = /GREENA */
			/*    bit 0 = /BLUEA */

			/* if non-zero pixels, just OR in the color */
			if (pix != 0)
				objdata[num * 8 + x] = (prom[pix | color] ^ 0x0f) | 0x10;

			/* otherwise, fetch the sprite data */
			else
				objdata[num * 8 + x] = prom[objdata[num * 8 + x]] ^ 0x0f;
		}
	}
}


UINT32 grchamp_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const rgb_t objpix_lookup[8] =
	{
		rgb_t(0,0,0),
		rgb_t(0,0,RGB_MAX),
		rgb_t(0,RGB_MAX,0),
		rgb_t(0,RGB_MAX,RGB_MAX),
		rgb_t(RGB_MAX,0,0),
		rgb_t(RGB_MAX,0,RGB_MAX),
		rgb_t(RGB_MAX,RGB_MAX,0),
		rgb_t(RGB_MAX,RGB_MAX,RGB_MAX)
	};

	const pen_t *bgpen = m_palette->pens();
	const UINT8 *amedata = memregion("gfx5")->base();
	const UINT8 *headdata = memregion("gfx6")->base();
	const UINT8 *pldata = memregion("gfx7")->base();
	bitmap_ind16 &lpixmap = m_left_tilemap->pixmap();
	bitmap_ind16 &rpixmap = m_right_tilemap->pixmap();
	bitmap_ind16 &cpixmap = m_center_tilemap->pixmap();
	int lrxscroll, cxscroll, lyscroll, ryscroll, cyscroll;
	int bgcolor = m_cpu1_out[3] & 0x10;
	int amebase = m_cpu0_out[4] >> 4;
	int plbase = m_cpu0_out[4] & 0x0f;
	int cxmask;
	int x, y;

	/* ensure that the tilemaps are the same size */
	assert(lpixmap.width() == rpixmap.width() && lpixmap.width() == cpixmap.width());
	assert(lpixmap.height() == rpixmap.height() && lpixmap.height() == cpixmap.height());

	/* extract background scroll values; left and right share the same X scroll */
	lrxscroll = m_cpu1_out[0] + (m_cpu1_out[1] & 1) * 256;
	lyscroll = m_cpu1_out[2];
	ryscroll = m_cpu1_out[7];
	cxscroll = m_cpu1_out[9] + (m_cpu1_out[10] & 1) * 256;
	cyscroll = m_cpu1_out[11];

	/* determine the center background mask, controlled by attribute bit 0x20 */
	cxmask = (m_cpu1_out[3] & 0x20) ? 0xff : 0x1ff;

	/* iterate over scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		/* select either left or right tilemaps based on Y */
		bitmap_ind16 &lrpixmap = (y < 128) ? lpixmap : rpixmap;
		int lryscroll = (y < 128) ? lyscroll : ryscroll;

		/* get source/dest pointers */
		/* the Y counter starts counting when VBLANK goes to 0, which is at Y=16 */
		UINT16 *lrsrc = &lrpixmap.pix16((lryscroll + y - 16) & 0xff);
		UINT16 *csrc = &cpixmap.pix16((cyscroll + y - 16) & 0xff);
		UINT32 *dest = &bitmap.pix32(y);
		UINT8 objdata[256];

		/* draw the objects for this scanline */
		draw_objects(y, objdata);

		/* iterate over columns */
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			rgb_t finalpix;
			int headbit = 0;
			int kill = 0;
			int mydh, mydv;
			int objpix;
			int mvid;

			/* the X counter starts counting when HSYNC goes to 0 */
			/* HYSYNC is high from X=304 through X=336; this means it has */
			/* been counting from 336 through 384 before HBLANK is low */
			mvid = csrc[(cxscroll + x + (384-336)) & cxmask];
			if ((mvid & 0x0f) == 0)
				mvid = lrsrc[(lrxscroll + x + (384-336)) & 0x1ff];

			/* objdata contains the REDA/GREENA/BLUEA states */
			objpix = objdata[x];

			/* if the headlamp is visible, determine that now */
			mydh = (m_cpu0_out[2] - x) & 0xff;
			mydv = (m_cpu0_out[3] - (y - 16)) & 0xff;
			if ((m_cpu0_out[0] & 0x10) && (mydh & 0xc0) == 0xc0 && ((mydv ^ (mydv >> 1)) & 0x40) == 0)
			{
				int bits = headdata[((mydh & 0x38) >> 3) |
									((mydv & 0x3f) << 3) |
									((~mydv & 0x40) << 3) |
									((m_cpu0_out[0] & 0x10) << 6)];
				headbit = (bits >> (~mydh & 0x07)) & 0x01;
			}

			/* if the headlamp is on and we're not in the headlamp area, */
			/* and this isn't a character pixel, the /KILL switch is set */
			if ((m_cpu0_out[0] & 0x10) && !headbit && !(objpix & 0x10))
			{
				kill = 1;
				objpix &= ~7;
			}

			/* if the player car is visible, compute its pixels */
			/* the H and V counters work like the tilemaps, offset by the same amount */
			if ((mydv & 0xe0) == 0 && (mydh & 0xe0) == 0)
			{
				int bits = pldata[(mydh >> 2) | (mydv << 3) | (plbase << 8)] >> (~mydh & 0x03);
				if (bits & 0x01)
				{
					objpix |= 4;    /* MYCAR(A) */

					/* handle collision detection between MYCARRED and MVID/OBJECT */

					/* skip if the state is being held clear, or if we already have a collision */
					if ((m_cpu0_out[0] & 0x02) && !(m_collide & 0x1000))
					{
						if (objpix & 0x08)
						{
							osd_printf_debug("Collide car/object @ (%d,%d)\n", x, y);
							m_collide = 0x1000 | 0x2000/* guess */ | ((~y & 0x80) << 3) | ((~y & 0xf8) << 2) | ((~x & 0xf8) >> 3);
						}
						else if ((mvid & 0x0f) != 0)
						{
							osd_printf_debug("Collide car/bg @ (%d,%d)\n", x, y);
							m_collide = 0x1000 | 0x4000/* guess */ | ((~y & 0x80) << 3) | ((~y & 0xf8) << 2) | ((~x & 0xf8) >> 3);
						}
					}
				}
				if (bits & 0x10)
					objpix |= 3;    /* MYCAR(B) */
			}

			/* if rain is enabled, it ORs against the bits */
			if (amebase != 0)
			{
				int effx = (m_cpu0_out[8] + x) & 0x0f;
				int effy = (m_cpu0_out[7] - y) & 0x0f;
				if ((amedata[(amebase << 5) | (effy << 1) | (effx >> 3)] >> (effx & 0x07)) & 0x01)
					objpix |= 7;
			}

			/* if the radar is on, it ORs against the bits */
			if (y >= 192 && (m_cpu0_out[0] & 0x80))
			{
				if ((m_radarram[((~y & 0x3e) << 4) | ((~x & 0xf8) >> 3)] >> (x & 0x07)) & 0x01)
					objpix |= 7;
			}

			/* handle collision detection between MVID and OBJECT */
			if (!(m_collide & 0x1000) && (objpix & 0x08) && (mvid & 0x0f) != 0)
			{
osd_printf_debug("Collide bg/object @ (%d,%d)\n", x, y);
				m_collide = 0x1000 | 0x8000 | ((~y & 0x80) << 3) | ((~y & 0xf8) << 2) | ((~x & 0xf8) >> 3);
			}

	/*

	OBJECT LAYER:

	    R = REDA | MYRADAR | MYCAR(A) | RADARVID | AMEOUT
	    G = GREENA | MYCAR(B) | RADARVID | AMEOUT
	    B = BLUEA | MYCAR(B) | RADARVID | AMEOUT

	    if (R | G | B) display object,
	    else display background
	*/


	/* still to do:
	    collision detection
	    myradar
	    fog
	    real headlamp effect
	*/

			/* if the object data is non-zero, it gets priority */
			if ((objpix & 7) != 0)
				finalpix = objpix_lookup[objpix & 7];

			/* otherwise, it's the background, unless it's been KILL'ed */
			else if (!kill)
				finalpix = bgpen[mvid | bgcolor];

			/* in which case it's black */
			else
				finalpix = rgb_t(0,0,0);

			/* if the headlamp is visible, adjust the brightness */
			if (headbit)
				finalpix += rgb_t(64,64,64);

			dest[x] = finalpix;
		}
	}


	return 0;
}
