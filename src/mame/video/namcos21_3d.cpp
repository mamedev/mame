// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood

#include "emu.h"
#include "namcos21_3d.h"

DEFINE_DEVICE_TYPE(NAMCOS21_3D, namcos21_3d_device, "namcos21_3d", "Namco System 21 3D Rasterizer")

namcos21_3d_device::namcos21_3d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOS21_3D, tag, owner, clock),
	m_fixed_palbase(-1),
	m_zz_shift(10),
	m_zzmult(0x100),
	m_depth_reverse(false),
	m_poly_frame_width(0),
	m_poly_frame_height(0),
	m_framebuffer_size_in_bytes(0)
{
}

void namcos21_3d_device::device_start()
{
	allocate_poly_framebuffer();
}

void namcos21_3d_device::device_reset()
{
}

void namcos21_3d_device::allocate_poly_framebuffer()
{
	if (m_framebuffer_size_in_bytes == 0)
		fatalerror("m_framebuffer_size_in_bytes == 0\n");

	m_mpPolyFrameBufferZ = std::make_unique<uint16_t[]>(m_framebuffer_size_in_bytes / 2);
	m_mpPolyFrameBufferPens = std::make_unique<uint16_t[]>(m_framebuffer_size_in_bytes / 2);

	m_mpPolyFrameBufferZ2 = std::make_unique<uint16_t[]>(m_framebuffer_size_in_bytes / 2);
	m_mpPolyFrameBufferPens2 = std::make_unique<uint16_t[]>(m_framebuffer_size_in_bytes / 2);

	swap_and_clear_poly_framebuffer();
	swap_and_clear_poly_framebuffer();

	save_pointer(NAME(m_mpPolyFrameBufferZ), m_framebuffer_size_in_bytes / 2);
	save_pointer(NAME(m_mpPolyFrameBufferPens), m_framebuffer_size_in_bytes / 2);

	save_pointer(NAME(m_mpPolyFrameBufferZ2), m_framebuffer_size_in_bytes / 2);
	save_pointer(NAME(m_mpPolyFrameBufferPens2), m_framebuffer_size_in_bytes / 2);
}

void namcos21_3d_device::swap_and_clear_poly_framebuffer()
{
	/* swap work and visible framebuffers */
	m_mpPolyFrameBufferZ.swap(m_mpPolyFrameBufferZ2);

	m_mpPolyFrameBufferPens.swap(m_mpPolyFrameBufferPens2);

	/* wipe work zbuffer */
	for (int i = 0; i < m_poly_frame_width*m_poly_frame_height; i++)
	{
		m_mpPolyFrameBufferZ[i] = 0x7fff;
	}
}

void namcos21_3d_device::copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &clip, int zlo, int zhi)
{
	/* blit the visible framebuffer */
	for (int sy = clip.top(); sy <= clip.bottom(); sy++)
	{
		uint16_t *const dest = &bitmap.pix(sy);
		uint16_t const *const pPen = m_mpPolyFrameBufferPens2.get() + m_poly_frame_width * sy;
		uint16_t const *const pZ = m_mpPolyFrameBufferZ2.get() + m_poly_frame_width * sy;
		for (int sx = clip.left(); sx <= clip.right(); sx++)
		{
			int z = pZ[sx];
			//if( pZ[sx]!=0x7fff )
			if (z >= zlo && z <= zhi)
			{
				dest[sx] = pPen[sx];
			}
		}
	}
}

/*********************************************************************************************/

#define SWAP(T,A,B) { const T *temp = A; A = B; B = temp; }

void namcos21_3d_device::renderscanline_flat(const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable)
{
	if (e1->x > e2->x)
	{
		SWAP(edge, e1, e2);
	}

	{
		uint16_t *pDest = m_mpPolyFrameBufferPens.get() + sy * m_poly_frame_width;
		uint16_t *pZBuf = m_mpPolyFrameBufferZ.get() + sy * m_poly_frame_width;
		int x0 = (int)e1->x;
		int x1 = (int)e2->x;
		int w = x1 - x0;
		if (w)
		{
			double z = e1->z;
			double dz = (e2->z - e1->z) / w;
			int x, crop;
			crop = -x0;
			if (crop > 0)
			{
				z += crop * dz;
				x0 = 0;
			}
			if (x1 > m_poly_frame_width - 1)
			{
				x1 = m_poly_frame_width - 1;
			}

			for (x = x0; x < x1; x++)
			{
				uint16_t zz = (uint16_t)z;
				if (zz < pZBuf[x])
				{
					int pen = color;
					if (depthcueenable && zz > 0)
					{
						int depth = 0;
						if (m_depth_reverse)
						{
							depth = (zz >> m_zz_shift)*m_zzmult;
							pen += depth;
						}
						else
						{
							depth = (zz >> m_zz_shift)*m_zzmult;
							pen -= depth;
						}
					}
					pDest[x] = pen;
					pZBuf[x] = zz;
				}
				z += dz;
			}
		}
	}
}

void namcos21_3d_device::rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, int depthcueenable)
{
	int dy, ystart, yend, crop;

	/* first, sort so that v0->y <= v1->y <= v2->y */
	for (;;)
	{
		if (v0->y > v1->y)
		{
			SWAP(n21_vertex, v0, v1);
		}
		else if (v1->y > v2->y)
		{
			SWAP(n21_vertex, v1, v2);
		}
		else
		{
			break;
		}
	}

	ystart = v0->y;
	yend = v2->y;
	dy = yend - ystart;
	if (dy)
	{
		int y;
		edge e1; /* short edge (top and bottom) */
		edge e2; /* long (common) edge */

		double dx2dy = (v2->x - v0->x) / dy;
		double dz2dy = (v2->z - v0->z) / dy;

		double dx1dy;
		double dz1dy;

		e2.x = v0->x;
		e2.z = v0->z;
		crop = -ystart;
		if (crop > 0)
		{
			e2.x += dx2dy * crop;
			e2.z += dz2dy * crop;
		}

		ystart = v0->y;
		yend = v1->y;
		dy = yend - ystart;
		if (dy)
		{
			e1.x = v0->x;
			e1.z = v0->z;

			dx1dy = (v1->x - v0->x) / dy;
			dz1dy = (v1->z - v0->z) / dy;

			crop = -ystart;
			if (crop > 0)
			{
				e1.x += dx1dy * crop;
				e1.z += dz1dy * crop;
				ystart = 0;
			}
			if (yend > m_poly_frame_height - 1) yend = m_poly_frame_height - 1;

			for (y = ystart; y < yend; y++)
			{
				renderscanline_flat(&e1, &e2, y, color, depthcueenable);

				e2.x += dx2dy;
				e2.z += dz2dy;

				e1.x += dx1dy;
				e1.z += dz1dy;
			}
		}

		ystart = v1->y;
		yend = v2->y;
		dy = yend - ystart;
		if (dy)
		{
			e1.x = v1->x;
			e1.z = v1->z;

			dx1dy = (v2->x - v1->x) / dy;
			dz1dy = (v2->z - v1->z) / dy;

			crop = -ystart;
			if (crop > 0)
			{
				e1.x += dx1dy * crop;
				e1.z += dz1dy * crop;
				ystart = 0;
			}
			if (yend > m_poly_frame_height - 1)
			{
				yend = m_poly_frame_height - 1;
			}
			for (y = ystart; y < yend; y++)
			{
				renderscanline_flat(&e1, &e2, y, color, depthcueenable);

				e2.x += dx2dy;
				e2.z += dz2dy;

				e1.x += dx1dy;
				e1.z += dz1dy;
			}
		}
	}
}

void namcos21_3d_device::draw_quad(int sx[4], int sy[4], int zcode[4], int color)
{
	n21_vertex a, b, c, d;
	int depthcueenable = 1;
	/*
	    0x0000..0x1fff  sprite palettes (0x20 sets of 0x100 colors)
	    0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	    0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	    0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	*/


	if (m_fixed_palbase != -1)
	{
		// Winning Run & Driver's Eyes use this logic
		color = m_fixed_palbase | (color & 0xff);
	}
	else
	{ /* map color code to hardware pen */
		int code = color >> 8;
		if (code & 0x80)
		{
			color = color & 0xff;
			// color = 0x3e00|color;
			color = 0x2100 | color;
			depthcueenable = 0;
		}
		else
		{
			color &= 0xff;
			color = 0x3e00 | color;
			if ((code & 0x02) == 0)
			{
				color |= 0x100;
			}
		}
	}
	a.x = sx[0];
	a.y = sy[0];
	a.z = zcode[0];

	b.x = sx[1];
	b.y = sy[1];
	b.z = zcode[1];

	c.x = sx[2];
	c.y = sy[2];
	c.z = zcode[2];

	d.x = sx[3];
	d.y = sy[3];
	d.z = zcode[3];

	rendertri(&a, &b, &c, color, depthcueenable);
	rendertri(&c, &d, &a, color, depthcueenable);
}
