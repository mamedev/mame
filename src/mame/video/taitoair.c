// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/***************************************************************************

Functions to emulate the video hardware of the machine.

TODO
====

Harmonise draw_sprites code and offload it into taitoic.c, leaving this
as short as possible (TC0080VCO has sprites as well as tilemaps in its
address space).

Maybe wait until the Taito Air system is done - also uses TC0080VCO.

Why does syvalion draw_sprites ignore the zoomy value
(using zoomx instead) ???


Sprite ram notes
----------------

BG / chain RAM
-----------------------------------------
[0]  +0         +1
     -xxx xxxx  xxxx xxxx = tile number

[1]  +0         +1
     ---- ----  x--- ---- = flip Y
     ---- ----  -x-- ---- = flip X
     ---- ----  --xx xxxx = color


sprite RAM
--------------------------------------------------------------
 +0         +1         +2         +3
 ---x ----  ---- ----  ---- ----  ---- ---- = unknown
 ---- xx--  ---- ----  ---- ----  ---- ---- = chain y size
 ---- --xx  xxxx xxxx  ---- ----  ---- ---- = sprite x coords
 ---- ----  ---- ----  ---- --xx  xxxx xxxx = sprite y coords

 +4         +5         +6         +7
 --xx xxxx  ---- ----  ---- ----  ---- ---- = zoom x
 ---- ----  --xx xxxx  ---- ----  ---- ---- = zoom y
 ---- ----  ---- ----  ---x xxxx  xxxx xxxx = tile information offset


***************************************************************************/

#include "emu.h"
#include "includes/taitoair.h"


/* These are hand-tuned values */
static const int zoomy_conv_table[] =
{
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,   /* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,   /* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};

/***************************************************************************
  Screen refresh
***************************************************************************/

void taitoair_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	/* Y chain size is 16/32?/64/64? pixels. X chain size
	   is always 64 pixels. */

	address_space &space = machine().driver_data()->generic_space();
	static const int size[] = { 1, 2, 4, 4 };
	int x0, y0, x, y, dx, dy, ex, ey, zx, zy;
	int ysize;
	int j, k;
	int offs;                   /* sprite RAM offset */
	int tile_offs;              /* sprite chain offset */
	int zoomx, zoomy;           /* zoom value */

	for (offs = 0x03f8 / 2; offs >= 0; offs -= 0x008 / 2)
	{
		/* TODO: remove this aberration of nature */
		if (offs <  0x01b0 && priority == 0)    continue;
		if (offs >= 0x01b0 && priority == 1)    continue;

		x0        =  m_tc0080vco->sprram_r(space, offs + 1, 0xffff) & 0x3ff;
		y0        =  m_tc0080vco->sprram_r(space, offs + 0, 0xffff) & 0x3ff;
		zoomx     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x7f00) >> 8;
		zoomy     = (m_tc0080vco->sprram_r(space, offs + 2, 0xffff) & 0x007f);
		tile_offs = (m_tc0080vco->sprram_r(space, offs + 3, 0xffff) & 0x1fff) << 2;
		ysize     = size[(m_tc0080vco->sprram_r(space, offs, 0xffff) & 0x0c00) >> 10];

		if (tile_offs)
		{
			/* Convert zoomy value to real value as zoomx */
			zoomy = zoomy_conv_table[zoomy];

			if (zoomx < 63)
			{
				dx = 8 + (zoomx + 2) / 8;
				ex = (zoomx + 2) % 8;
				zx = ((dx << 1) + ex) << 11;
			}
			else
			{
				dx = 16 + (zoomx - 63) / 4;
				ex = (zoomx - 63) % 4;
				zx = (dx + ex) << 12;
			}

			if (zoomy < 63)
			{
				dy = 8 + (zoomy + 2) / 8;
				ey = (zoomy + 2) % 8;
				zy = ((dy << 1) + ey) << 11;
			}
			else
			{
				dy = 16 + (zoomy - 63) / 4;
				ey = (zoomy - 63) % 4;
				zy = (dy + ey) << 12;
			}

			if (x0 >= 0x200) x0 -= 0x400;
			if (y0 >= 0x200) y0 -= 0x400;

			if (m_tc0080vco->flipscreen_r())
			{
				x0 = 497 - x0;
				y0 = 498 - y0;
				dx = -dx;
				dy = -dy;
			}
			else
			{
				x0 += 1;
				y0 += 2;
			}

			y = y0;
			for (j = 0; j < ysize; j ++)
			{
				x = x0;
				for (k = 0; k < 4; k ++)
				{
					if (tile_offs >= 0x1000)
					{
						int tile, color, flipx, flipy;

						tile  = m_tc0080vco->cram_0_r(space, tile_offs, 0xffff) & 0x7fff;
						color = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x001f;
						flipx = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0040;
						flipy = m_tc0080vco->cram_1_r(space, tile_offs, 0xffff) & 0x0080;

						if (m_tc0080vco->flipscreen_r())
						{
							flipx ^= 0x0040;
							flipy ^= 0x0080;
						}


									m_gfxdecode->gfx(0)->zoom_transpen(bitmap,cliprect,
									tile,
									color,
									flipx, flipy,
									x, y,
									zx, zy, 0
						);
					}
					tile_offs ++;
					x += dx;
				}
				y += dy;
			}
		}
	}
}

void taitoair_state::fill_slope( bitmap_ind16 &bitmap, const rectangle &cliprect, int color, INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 y1, INT32 y2, INT32 *nx1, INT32 *nx2 )
{
	if (y1 > cliprect.max_y)
		return;

	if (y2 <= cliprect.min_y)
	{
		int delta = y2 - y1;
		*nx1 = x1 + delta * sl1;
		*nx2 = x2 + delta * sl2;
		return;
	}

	if (y1 < -1000000 || y1 > 1000000)
		return;

	if (y2 > cliprect.max_y)
		y2 = cliprect.max_y + 1;

	if (y1 < cliprect.min_y)
	{
		int delta = cliprect.min_y - y1;
		x1 += delta * sl1;
		x2 += delta * sl2;
		y1 = cliprect.min_y;
	}

	if (x1 > x2 || (x1==x2 && sl1 > sl2))
	{
		INT32 t, *tp;
		t = x1;
		x1 = x2;
		x2 = t;
		t = sl1;
		sl1 = sl2;
		sl2 = t;
		tp = nx1;
		nx1 = nx2;
		nx2 = tp;
	}

	while (y1 < y2)
	{
		if (y1 >= cliprect.min_y)
		{
			int xx1 = x1 >> TAITOAIR_FRAC_SHIFT;
			int xx2 = x2 >> TAITOAIR_FRAC_SHIFT;
			int grad_col;
			int base_color;

			if (xx1 <= cliprect.max_x || xx2 >= cliprect.min_x)
			{
				if (xx1 < cliprect.min_x)
					xx1 = cliprect.min_x;
				if (xx2 > cliprect.max_x)
					xx2 = cliprect.max_x;

				if(color & 0x40)
				{
					/* Non-terrain elements are colored with this. */
					base_color = (color & 0x3f) + 0x340;
					grad_col = 0;
				}
				else
				{
					/* Terrain elements, with a gradient applied. */
					/* TODO: it's unknown if gradient color applies by global screen Y coordinate or there's a calculation to somewhere ... */
					base_color = ((color & 0x3f) * 0x80) + 0x2040;
					grad_col = (y1 >> 3) & 0x3f;
				}

				while (xx1 <= xx2)
				{
					bitmap.pix16(y1, xx1) = base_color + grad_col;
					xx1++;
				}
			}
		}

		x1 += sl1;
		x2 += sl2;
		y1++;
	}
	*nx1 = x1;
	*nx2 = x2;
}

void taitoair_state::fill_poly( bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q )
{
	INT32 sl1, sl2, cury, limy, x1, x2;
	int pmin, pmax, i, ps1, ps2;
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT * 2];
	int color = q->col;
	int pcount = q->pcount;

	for (i = 0; i < pcount; i++)
	{
		p[i].x = p[i + pcount].x = q->p[i].x << TAITOAIR_FRAC_SHIFT;
		p[i].y = p[i + pcount].y = q->p[i].y;
	}

	pmin = pmax = 0;
	for (i = 1; i < pcount; i++)
	{
		if (p[i].y < p[pmin].y)
			pmin = i;
		if (p[i].y > p[pmax].y)
			pmax = i;
	}

	cury = p[pmin].y;
	limy = p[pmax].y;

	if (cury == limy)
		return;

	if (cury > cliprect.max_y)
		return;
	if (limy <= cliprect.min_y)
		return;

	if (limy > cliprect.max_y)
		limy = cliprect.max_y;

	ps1 = pmin + pcount;
	ps2 = pmin;

	goto startup;

	for (;;)
	{
		if (p[ps1 - 1].y == p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			ps2++;

		startup:
			while (p[ps1 - 1].y == cury)
				ps1--;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x1 = p[ps1].x;
			x2 = p[ps2].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
		else if (p[ps1 - 1].y < p[ps2 + 1].y)
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if (cury >= limy)
				break;
			ps1--;
			while (p[ps1 - 1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
		}
		else
		{
			fill_slope(bitmap, cliprect, color, x1, x2, sl1, sl2, cury, p[ps2 + 1].y, &x1, &x2);
			cury = p[ps2 + 1].y;
			if (cury >= limy)
				break;
			ps2++;
			while (p[ps2 + 1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
	}
}

/***************************************************************************
  dsp handlers
***************************************************************************/

/*
    TODO: still don't know how this works. It calls three values (0x1fff-0x5fff-0xdfff), for two or three offsets.
    In theory this should fit into framebuffer draw, display, clear and swap in some way.
*/
WRITE16_MEMBER(taitoair_state::dsp_flags_w)
{
	rectangle cliprect;

	/* printf("%04x -> %d\n",data,offset); */

	cliprect.min_x = 0;
	cliprect.min_y = 3*16;
	cliprect.max_x = m_screen->width() - 1;
	cliprect.max_y = m_screen->height() - 1;

	{
		/* clear and copy operation if offset is 0x3001 */
		if(offset == 1)
		{
			/* clear screen fb */
			m_framebuffer[1]->fill(0, cliprect);
			/* copy buffer fb into screen fb (at this stage we are ready to draw) */
			copybitmap_trans(*m_framebuffer[1], *m_framebuffer[0], 0, 0, 0, 0, cliprect, 0);
			/* now clear buffer fb */
			m_framebuffer[0]->fill(0, cliprect);
		}

		/* if offset 0x3001 OR 0x3002 we put data in the buffer fb */
		if(offset)
		{
			if (m_line_ram[0x3fff])
			{
				int adr = 0x3fff;
//              struct taitoair_poly q;

				while (adr >= 0 && m_line_ram[adr] && m_line_ram[adr] != 0x4000)
				{
					int pcount;
					if (!(m_line_ram[adr] & 0x8000) || adr < 10)
					{
						logerror("quad: unknown value %04x at %04x\n", m_line_ram[adr], adr);
						break;
					}
					m_q.col = m_line_ram[adr] & 0x7f;//((m_line_ram[adr] & 0x007f) * 0x80) + 0x2040;

					adr--;
					pcount = 0;
					while (pcount < TAITOAIR_POLY_MAX_PT && adr >= 1 && !(m_line_ram[adr] & 0xc000))
					{
						m_q.p[pcount].y = m_line_ram[adr] + 3 * 16;
						m_q.p[pcount].x = m_line_ram[adr - 1];
						pcount++;
						adr -= 2;
					}
					adr--;
					m_q.pcount = pcount;
					fill_poly(*m_framebuffer[0], cliprect, &m_q);
				}
			}
		}
	}
}

WRITE16_MEMBER(taitoair_state::dsp_x_eyecoord_w)
{
	m_eyecoordBuffer[0] = data;
}

WRITE16_MEMBER(taitoair_state::dsp_y_eyecoord_w)
{
	m_eyecoordBuffer[1] = data;
}

WRITE16_MEMBER(taitoair_state::dsp_z_eyecoord_w)
{
	m_eyecoordBuffer[2] = data;
}

WRITE16_MEMBER(taitoair_state::dsp_frustum_left_w)
{
	/* Strange.  It comes in as it it were the right side of the screen */
	m_frustumLeft = -data;
}

WRITE16_MEMBER(taitoair_state::dsp_frustum_bottom_w)
{
	m_frustumBottom = data;
}


void taitoair_state::multVecMtx(const INT16* vec4, const float* m, float* result)
{
#define M(row,col)  m[col*4+row]
	result[0] = vec4[0]*M(0,0) + vec4[1]*M(1,0) + vec4[2]*M(2,0) + vec4[3]*M(3,0);
	result[1] = vec4[0]*M(0,1) + vec4[1]*M(1,1) + vec4[2]*M(2,1) + vec4[3]*M(3,1);
	result[2] = vec4[0]*M(0,2) + vec4[1]*M(1,2) + vec4[2]*M(2,2) + vec4[3]*M(3,2);

	float w = vec4[0]*M(0,3) + vec4[1]*M(1,3) + vec4[2]*M(2,3) + vec4[3]*M(3,3);
	result[0] /= w;
	result[1] /= w;
	result[2] /= w;
#undef M
}

int taitoair_state::projectEyeCoordToScreen(float* projectionMatrix,const int Res,INT16* eyePoint3d,int type)
{
	/* Return (-1, -1) if the eye point is behind camera */
	int res = -10000;
	if (eyePoint3d[2] <= 0.0 && eyePoint3d[0] <= 0.0)
		return -10000;
	if (eyePoint3d[2] <= 0.0 && eyePoint3d[0] >= 0.0)
		return 10000;

	/* Coordinate system flip */
	eyePoint3d[0] *= -1;

	/* Nothing fancy about this homogeneous worldspace coordinate */
	eyePoint3d[3] = 1;

	float deviceCoordinates[3];
	multVecMtx(eyePoint3d, projectionMatrix, deviceCoordinates);

	/* We're only interested if it projects within the device */
	// if ( ( deviceCoordinates[type] >= -1.0) && ( deviceCoordinates[type] <= 1.0))
	res = (int)( deviceCoordinates[type] * (Res-1) );

	return res;
}

void taitoair_state::airInfernoFrustum(const INT16 leftExtent, const INT16 bottomExtent, float* m)
{
	/* Hard-coded near and far clipping planes :( */
	float nearZ = 1.0f;
	float farZ = 10000.0f;
	float left = 1.0f;
	float right = -1.0f;
	float bottom = (float)(-bottomExtent) / leftExtent;
	float top = (float)(bottomExtent) / leftExtent;

	float x = (2.0f*nearZ) / (right-left);
	float y = (2.0f*nearZ) / (top-bottom);
	float a = (right+left) / (right-left);
	float b = (top+bottom) / (top-bottom);
	float c = -(farZ+nearZ) / ( farZ-nearZ);
	float d = -(2.0f*farZ*nearZ) / (farZ-nearZ);

#define M(row,col)  m[col*4+row]
	M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
	M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
	M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
	M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
}

WRITE16_MEMBER(taitoair_state::dsp_rasterize_w)
{
}

READ16_MEMBER(taitoair_state::dsp_x_return_r)
{
	/* Construct a frustum from the system's most recently set left and bottom extents */
	float m[16];
	airInfernoFrustum(m_frustumLeft, m_frustumBottom, m);
	int res;

	res = projectEyeCoordToScreen(m,
									32*16, /* x max screen size */
									m_eyecoordBuffer,0);

	// Extremely poor man's clipping :-P
	if (res == -10000) return -32*8;
	if (res ==  10000) return 32*8-1;
	if (res > 32*8-1) res = 32*8-1;
	if (res < -32*8) res = -32*8;
	return res;
}

READ16_MEMBER(taitoair_state::dsp_y_return_r)
{
	/* Construct a frustum from the system's most recently set left and bottom extents */
	float m[16];
	airInfernoFrustum(m_frustumLeft, m_frustumBottom, m);

	int res;
	res = projectEyeCoordToScreen(m,
									28*16, /* y max screen size */
									m_eyecoordBuffer, 1);

	// Extremely poor man's clipping :-P
	if (res == -10000) return 28*7;
	if (res == 10000) return 28*7;
	if (res > 28*7) res = 28*7;
	if (res < -28*7) res = -28*7;
	return res;
}

void taitoair_state::video_start()
{
	int width, height;

	width = m_screen->width();
	height = m_screen->height();
	m_framebuffer[0] = auto_bitmap_ind16_alloc(machine(), width, height);
	m_framebuffer[1] = auto_bitmap_ind16_alloc(machine(), width, height);
	//m_buffer3d = auto_bitmap_ind16_alloc(machine(), width, height);
}

UINT32 taitoair_state::screen_update_taitoair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tc0080vco->tilemap_update();

	UINT32 counter1 = (m_tc0430grw[0] << 16) | m_tc0430grw[1];
	UINT32 inc1x    = INT16(m_tc0430grw[2]);
	UINT32 inc1y    = INT16(m_tc0430grw[3]);
	UINT32 counter2 = (m_tc0430grw[4] << 16) | m_tc0430grw[5];
	UINT32 inc2x    = INT16(m_tc0430grw[6]);
	UINT32 inc2y    = INT16(m_tc0430grw[7]);

	// Deltas are 118/31
	int dx = cliprect.min_x      + 118;
	int dy = cliprect.min_y - 48 + 31;

	counter1 += dx*inc1x + dy*inc1y;
	counter2 += dx*inc2x + dy*inc2y;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		UINT32 c1b = counter1;
		UINT32 c2b = counter2;
		UINT16 *dest = &bitmap.pix(y, cliprect.min_x);
		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			UINT16 base = 0;
			UINT32 cntr = 0;
			if(c2b & 0x800000) {
				base = 0x2040;
				cntr = c2b;
			} else if(c1b & 0x800000) {
				base = 0x2000;
				cntr = c1b;
			}
			*dest++ = base | (cntr >= 0x83f000 ? 0x3f : (cntr >> 12) & 0x3f);
				
			c1b += inc1x;
			c2b += inc2x;
		}
		counter1 += inc1y;
		counter2 += inc2y;
	}
		

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	draw_sprites(bitmap, cliprect, 0);

	copybitmap_trans(bitmap, *m_framebuffer[1], 0, 0, 0, 0, cliprect, 0);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 1, 0, 0);

	draw_sprites(bitmap, cliprect, 1);

	m_tc0080vco->tilemap_draw(screen, bitmap, cliprect, 2, 0, 0);

	/* Hacky 3d bitmap */
	//copybitmap_trans(bitmap, m_buffer3d, 0, 0, 0, 0, cliprect, 0);
	//m_buffer3d->fill(0, cliprect);

	return 0;
}
