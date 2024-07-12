// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood

#include "emu.h"
#include "namcos21_3d.h"

#include <algorithm>
#include <utility>

DEFINE_DEVICE_TYPE(NAMCOS21_3D, namcos21_3d_device, "namcos21_3d", "Namco System 21 3D Rasterizer")

namcos21_3d_device::namcos21_3d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCOS21_3D, tag, owner, clock),
	m_fixed_palbase(-1),
	m_zz_shift(10),
	m_zzmult(0x100),
	m_depth_reverse(false),
	m_poly_frame_width(0),
	m_poly_frame_height(0)
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
	unsigned framebuffer_size = m_poly_frame_width*m_poly_frame_height;

	if (framebuffer_size == 0)
		fatalerror("framebuffer_size == 0\n");

	m_mpPolyFrameBufferZ = std::make_unique<uint16_t[]>(framebuffer_size);
	m_mpPolyFrameBufferPens = std::make_unique<uint16_t[]>(framebuffer_size);

	m_mpPolyFrameBufferZ2 = std::make_unique<uint16_t[]>(framebuffer_size);
	m_mpPolyFrameBufferPens2 = std::make_unique<uint16_t[]>(framebuffer_size);

	swap_and_clear_poly_framebuffer();
	swap_and_clear_poly_framebuffer();

	save_pointer(NAME(m_mpPolyFrameBufferZ), framebuffer_size);
	save_pointer(NAME(m_mpPolyFrameBufferPens), framebuffer_size);

	save_pointer(NAME(m_mpPolyFrameBufferZ2), framebuffer_size);
	save_pointer(NAME(m_mpPolyFrameBufferPens2), framebuffer_size);
}

void namcos21_3d_device::swap_and_clear_poly_framebuffer()
{
	/* swap work and visible framebuffers */
	m_mpPolyFrameBufferZ.swap(m_mpPolyFrameBufferZ2);

	m_mpPolyFrameBufferPens.swap(m_mpPolyFrameBufferPens2);

	/* wipe work zbuffer */
	std::fill_n(m_mpPolyFrameBufferZ.get(), m_poly_frame_width*m_poly_frame_height, 0x7fff);
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

void namcos21_3d_device::renderscanline_flat(const edge *e1, const edge *e2, unsigned sy, unsigned color, bool depthcueenable)
{
	if (e1->x > e2->x)
		std::swap(e1, e2);

	int x0 = (int)e1->x;
	int x1 = (int)e2->x;
	int w = x1 - x0;

	if (!w)
		return;

	double z = e1->z;
	double dz = (e2->z - e1->z) / w;

	if (x0 < 0)
	{
		z += -x0 * dz;
		x0 = 0;
	}

	if (x1 > m_poly_frame_width - 1)
		x1 = m_poly_frame_width - 1;

	uint16_t *pDest = m_mpPolyFrameBufferPens.get() + sy * m_poly_frame_width;
	uint16_t *pZBuf = m_mpPolyFrameBufferZ.get() + sy * m_poly_frame_width;

	for (int x = x0; x < x1; x++)
	{
		uint16_t zz = (uint16_t)z;

		if (zz < pZBuf[x])
		{
			unsigned pen = color;

			if (depthcueenable && zz > 0)
			{
				const unsigned depth = (zz >> m_zz_shift)*m_zzmult;

				if (m_depth_reverse)
					pen += depth;
				else
					pen -= depth;
			}

			pDest[x] = pen;
			pZBuf[x] = zz;
		}

		z += dz;
	}
}

void namcos21_3d_device::rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, bool depthcueenable)
{
	/* first, sort so that v0->y <= v1->y <= v2->y */
	{
		if (v0->y > v1->y)
			std::swap(v0, v1);

		if (v1->y > v2->y)
			std::swap(v1, v2);

		if (v0->y > v1->y)
			std::swap(v0, v1);
	}

	int ystart = v0->y;
	int yend = v2->y;
	int dy = yend - ystart;

	if (dy)
	{
		edge e1; /* short edge (top and bottom) */
		edge e2; /* long (common) edge */

		double dx2dy = (v2->x - v0->x) / dy;
		double dz2dy = (v2->z - v0->z) / dy;

		double dx1dy;
		double dz1dy;

		e2.x = v0->x;
		e2.z = v0->z;

		if (ystart < 0)
		{
			e2.x += dx2dy * -ystart;
			e2.z += dz2dy * -ystart;
		}

		for (const auto& v_pair : {std::make_pair(v0, v1), std::make_pair(v1, v2)})
		{

			e1.x = v_pair.first->x;
			e1.z = v_pair.first->z;

			ystart = v_pair.first->y;
			yend = v_pair.second->y;
			dy = yend - ystart;

			if (!dy)
				continue;

			dx1dy = (v_pair.second->x - v_pair.first->x) / dy;
			dz1dy = (v_pair.second->z - v_pair.first->z) / dy;

			if (ystart < 0)
			{
				e1.x += dx1dy * -ystart;
				e1.z += dz1dy * -ystart;
				ystart = 0;
			}

			if (yend > m_poly_frame_height - 1)
				yend = m_poly_frame_height - 1;

			for (int y = ystart; y < yend; y++)
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

void namcos21_3d_device::draw_quad(int sx[4], int sy[4], int zcode[4], unsigned color)
{
	bool depthcueenable = true;
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
		unsigned code = color >> 8;
		if (code & 0x80)
		{
			color = 0x2100 | (color & 0xff);
			// color = 0x3e00|color;
			depthcueenable = false;
		}
		else
		{
			color = 0x3e00 | (color & 0xff);

			if ((code & 0x02) == 0)
				color |= 0x100;
		}
	}

	n21_vertex a = {
		.x = (double)sx[0],
		.y = (double)sy[0],
		.z = (double)zcode[0]
	};

	n21_vertex b = {
		.x = (double)sx[1],
		.y = (double)sy[1],
		.z = (double)zcode[1]
	};

	n21_vertex c = {
		.x = (double)sx[2],
		.y = (double)sy[2],
		.z = (double)zcode[2]
	};

	n21_vertex d = {
		.x = (double)sx[3],
		.y = (double)sy[3],
		.z = (double)zcode[3]
	};

	rendertri(&a, &b, &c, color, depthcueenable);
	rendertri(&c, &d, &a, color, depthcueenable);
}
