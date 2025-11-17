// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese

// Taito TC0780FPA Polygon Renderer

#include "emu.h"
#include "tc0780fpa.h"
#include "screen.h"


static constexpr unsigned POLY_FIFO_SIZE = 32;


tc0780fpa_renderer::tc0780fpa_renderer(device_t &parent, screen_device &screen, const uint8_t *texture_ram)
	: poly_manager<float, tc0780fpa_polydata, 6>(screen.machine())
{
	int const width = screen.width();
	int const height = screen.height();

	m_fb[0].allocate(width, height);
	m_fb[1].allocate(width, height);
	m_zb.allocate(width, height);

	m_texture = texture_ram;

	m_cliprect = screen.cliprect();

	m_current_fb = 0;

	// save state
	parent.save_item(NAME(m_fb[0]));
	parent.save_item(NAME(m_fb[1]));
	parent.save_item(NAME(m_zb));
}

void tc0780fpa_renderer::swap_buffers()
{
	wait("Finished render");

	m_current_fb ^= 1;

	m_fb[m_current_fb].fill(0, m_cliprect);
	m_zb.fill(0xffff, m_cliprect);
}

void tc0780fpa_renderer::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap_trans(bitmap, m_fb[m_current_fb ^ 1], 0, 0, 0, 0, cliprect, 0);
}

void tc0780fpa_renderer::render_solid_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	int const color = extent.param[1].start;
	float const dz = extent.param[0].dpdx;
	uint16_t *const fb = &m_fb[m_current_fb].pix(scanline);
	uint16_t *const zb = &m_zb.pix(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int const iz = (int)z & 0xffff;

		if (iz <= zb[x])
		{
			fb[x] = color;
			zb[x] = iz;
		}

		z += dz;
	}
}

void tc0780fpa_renderer::render_shade_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	float color = extent.param[1].start;
	float const dz = extent.param[0].dpdx;
	float const dcolor = extent.param[1].dpdx;
	uint16_t *const fb = &m_fb[m_current_fb].pix(scanline);
	uint16_t *const zb = &m_zb.pix(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int const ic = (int)color & 0xffff;
		int const iz = (int)z & 0xffff;

		if (iz <= zb[x])
		{
			fb[x] = ic;
			zb[x] = iz;
		}

		color += dcolor;
		z += dz;
	}
}

void tc0780fpa_renderer::render_texture_scan(int32_t scanline, const extent_t &extent, const tc0780fpa_polydata &extradata, int threadid)
{
	float z = extent.param[0].start;
	float u = extent.param[1].start;
	float v = extent.param[2].start;
	float color = extent.param[3].start;
	float const dz = extent.param[0].dpdx;
	float const du = extent.param[1].dpdx;
	float const dv = extent.param[2].dpdx;
	float const dcolor = extent.param[3].dpdx;
	uint16_t *const fb = &m_fb[m_current_fb].pix(scanline);
	uint16_t *const zb = &m_zb.pix(scanline);
	int const tex_wrap_x = extradata.tex_wrap_x;
	int const tex_wrap_y = extradata.tex_wrap_y;
	int const tex_base_x = extradata.tex_base_x;
	int const tex_base_y = extradata.tex_base_y;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu, iv;
		int const palette = ((int)color & 0x7f) << 8;
		int const iz = (int)z & 0xffff;

		if (!tex_wrap_x)
		{
			iu = ((int)u >> 4) & 0x7ff;
		}
		else
		{
			iu = (tex_base_x + (((int)u >> 4) & ((0x08 << tex_wrap_x) - 1))) & 0x7ff;
		}

		if (!tex_wrap_y)
		{
			iv = ((int)v >> 4) & 0x7ff;
		}
		else
		{
			iv = (tex_base_y + (((int)v >> 4) & ((0x08 << tex_wrap_y) - 1))) & 0x7ff;
		}

		uint8_t const texel = m_texture[(iv << 11) + iu];

		if (iz <= zb[x] && texel != 0)
		{
			fb[x] = palette | texel;
			zb[x] = iz;
		}

		u += du;
		v += dv;
		color += dcolor;
		z += dz;
	}
}


void tc0780fpa_renderer::render(uint16_t *polygon_fifo, int length)
{
	vertex_t vert[4];

	uint16_t const cmd = polygon_fifo[0];

	int ptr = 1;
	switch (cmd & 0x7)
	{
		// screen global clipping for 3d(?)
		case 0x00:
		{
			uint16_t const min_x = polygon_fifo[ptr+1];
			uint16_t const min_y = polygon_fifo[ptr+0];
			uint16_t const min_z = polygon_fifo[ptr+2];
			uint16_t const max_x = polygon_fifo[ptr+4];
			uint16_t const max_y = polygon_fifo[ptr+3];
			uint16_t const max_z = polygon_fifo[ptr+5];

			if (min_x != 0 || min_y != 0 || min_z != 0 || max_x != 512 || max_y != 400 || max_z != 0x7fff)
			{
				printf("CMD %04x\n", cmd);
				printf("MIN Y %04x\n", polygon_fifo[ptr+0]);
				printf("MIN X %04x\n", polygon_fifo[ptr+1]);
				printf("MIN Z %04x\n", polygon_fifo[ptr+2]);
				printf("MAX Y %04x\n", polygon_fifo[ptr+3]);
				printf("MAX X %04x\n", polygon_fifo[ptr+4]);
				printf("MAX Z %04x\n", polygon_fifo[ptr+5]);
			}

			swap_buffers();
			break;
		}

		// Gouraud Shaded Triangle (Landing Gear)
		case 0x01:
		{
			// 0x00: Command ID (0x0001)
			// 0x01: Vertex 1 color
			// 0x02: Vertex 1 Y
			// 0x03: Vertex 1 X
			// 0x04: Vertex 1 Z
			// 0x05: Vertex 2 color
			// 0x06: Vertex 2 Y
			// 0x07: Vertex 2 X
			// 0x08: Vertex 2 Z
			// 0x09: Vertex 3 color
			// 0x0a: Vertex 3 Y
			// 0x0b: Vertex 3 X
			// 0x0c: Vertex 3 Z

			for (int i = 0; i < 3; i++)
			{
				vert[i].p[1] = polygon_fifo[ptr++];
				vert[i].y =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].x =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[0] = (uint16_t)(polygon_fifo[ptr++]);
			}

			if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000)
			{
				if (vert[0].p[1] == vert[1].p[1] &&
					vert[1].p[1] == vert[2].p[1])
				{
					// optimization: all colours the same -> render solid
					render_triangle<2>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_solid_scan, this), vert[0], vert[1], vert[2]);
				}
				else
				{
					render_triangle<2>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_shade_scan, this), vert[0], vert[1], vert[2]);
				}
			}
			break;
		}

		// Textured Triangle
		case 0x03:
		{
			// 0x00: Command ID (0x0003)
			// 0x01: Texture base
			// 0x02: Vertex 1 Palette
			// 0x03: Vertex 1 V
			// 0x04: Vertex 1 U
			// 0x05: Vertex 1 Y
			// 0x06: Vertex 1 X
			// 0x07: Vertex 1 Z
			// 0x08: Vertex 2 Palette
			// 0x09: Vertex 2 V
			// 0x0a: Vertex 2 U
			// 0x0b: Vertex 2 Y
			// 0x0c: Vertex 2 X
			// 0x0d: Vertex 2 Z
			// 0x0e: Vertex 3 Palette
			// 0x0f: Vertex 3 V
			// 0x10: Vertex 3 U
			// 0x11: Vertex 3 Y
			// 0x12: Vertex 3 X
			// 0x13: Vertex 3 Z

			tc0780fpa_polydata &extra = object_data().next();
			uint16_t const texbase = polygon_fifo[ptr++];

			extra.tex_base_x = ((texbase >> 0) & 0xff) << 4;
			extra.tex_base_y = ((texbase >> 8) & 0xff) << 4;

			extra.tex_wrap_x = (cmd >> 6) & 3;
			extra.tex_wrap_y = (cmd >> 4) & 3;

			for (int i = 0; i < 3; i++)
			{
				vert[i].p[3] = polygon_fifo[ptr++] + 0.5; // palette
				vert[i].p[2] = (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[1] = (int16_t)(polygon_fifo[ptr++]);
				vert[i].y =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].x =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[0] = (uint16_t)(polygon_fifo[ptr++]);
			}

			if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000)
			{
				render_triangle<4>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_texture_scan, this), vert[0], vert[1], vert[2]);
			}
			break;
		}

		// Gouraud shaded Quad
		case 0x04:
		{
			// 0x00: Command ID (0x0004)
			// 0x01: Vertex 1 color
			// 0x02: Vertex 1 Y
			// 0x03: Vertex 1 X
			// 0x04: Vertex 1 Z
			// 0x05: Vertex 2 color
			// 0x06: Vertex 2 Y
			// 0x07: Vertex 2 X
			// 0x08: Vertex 2 Z
			// 0x09: Vertex 3 color
			// 0x0a: Vertex 3 Y
			// 0x0b: Vertex 3 X
			// 0x0c: Vertex 3 Z
			// 0x0d: Vertex 4 color
			// 0x0e: Vertex 4 Y
			// 0x0f: Vertex 4 X
			// 0x10: Vertex 4 Z

			for (int i = 0; i < 4; i++)
			{
				vert[i].p[1] = polygon_fifo[ptr++];
				vert[i].y =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].x =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[0] = (uint16_t)(polygon_fifo[ptr++]);
			}

			if (vert[0].p[0] < 0x8000 && vert[1].p[0] < 0x8000 && vert[2].p[0] < 0x8000 && vert[3].p[0] < 0x8000)
			{
				if (vert[0].p[1] == vert[1].p[1] &&
					vert[1].p[1] == vert[2].p[1] &&
					vert[2].p[1] == vert[3].p[1])
				{
					// optimization: all colours the same -> render solid
					render_polygon<4, 2>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_solid_scan, this), vert);
				}
				else
				{
					render_polygon<4, 2>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_shade_scan, this), vert);
				}
			}
			break;
		}

		// Textured Quad
		case 0x06:
		{
			// 0x00: Command ID (0x0006)
			// 0x01: Texture base
			// 0x02: Vertex 1 Palette
			// 0x03: Vertex 1 V
			// 0x04: Vertex 1 U
			// 0x05: Vertex 1 Y
			// 0x06: Vertex 1 X
			// 0x07: Vertex 1 Z
			// 0x08: Vertex 2 Palette
			// 0x09: Vertex 2 V
			// 0x0a: Vertex 2 U
			// 0x0b: Vertex 2 Y
			// 0x0c: Vertex 2 X
			// 0x0d: Vertex 2 Z
			// 0x0e: Vertex 3 Palette
			// 0x0f: Vertex 3 V
			// 0x10: Vertex 3 U
			// 0x11: Vertex 3 Y
			// 0x12: Vertex 3 X
			// 0x13: Vertex 3 Z
			// 0x14: Vertex 4 Palette
			// 0x15: Vertex 4 V
			// 0x16: Vertex 4 U
			// 0x17: Vertex 4 Y
			// 0x18: Vertex 4 X
			// 0x19: Vertex 4 Z

			tc0780fpa_polydata &extra = object_data().next();
			uint16_t const texbase = polygon_fifo[ptr++];

			extra.tex_base_x = ((texbase >> 0) & 0xff) << 4;
			extra.tex_base_y = ((texbase >> 8) & 0xff) << 4;

			extra.tex_wrap_x = (cmd >> 6) & 3;
			extra.tex_wrap_y = (cmd >> 4) & 3;

			for (int i = 0; i < 4; i++)
			{
				vert[i].p[3] = polygon_fifo[ptr++] + 0.5; // palette
				vert[i].p[2] = (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[1] = (int16_t)(polygon_fifo[ptr++]);
				vert[i].y =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].x =  (int16_t)(polygon_fifo[ptr++]);
				vert[i].p[0] = (uint16_t)(polygon_fifo[ptr++]);
			}

			if (vert[0].p[0] < 0xc000 && vert[1].p[0] < 0xc000 && vert[2].p[0] < 0xc000 && vert[3].p[0] < 0xc000)
			{
				render_polygon<4, 4>(m_cliprect, render_delegate(&tc0780fpa_renderer::render_texture_scan, this), vert);
			}
			break;
		}

		default:
		{
			printf("tc0780fpa::render(): unknown command %04X %d, %d\n", cmd, ptr, length);
			break;
		}
	}
}

DEFINE_DEVICE_TYPE(TC0780FPA, tc0780fpa_device, "tc0780fpa", "TC0780FPA Polygon Renderer")

tc0780fpa_device::tc0780fpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0780FPA, tag, owner, clock)
	, device_video_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0780fpa_device::device_start()
{
	m_texture = std::make_unique<uint8_t[]>(0x400000);
	m_poly_fifo = std::make_unique<uint16_t[]>(POLY_FIFO_SIZE);

	m_renderer = std::make_unique<tc0780fpa_renderer>(*this, screen(), m_texture.get());

	save_pointer(NAME(m_texture), 0x400000);
	save_pointer(NAME(m_poly_fifo), POLY_FIFO_SIZE);
	save_item(NAME(m_poly_fifo_ptr));
	save_item(NAME(m_tex_address));
	save_item(NAME(m_tex_offset));
	save_item(NAME(m_texbase_x));
	save_item(NAME(m_texbase_y));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0780fpa_device::device_reset()
{
	m_poly_fifo_ptr = 0;
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void tc0780fpa_device::device_stop()
{
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

uint16_t tc0780fpa_device::tex_addr_r()
{
	return m_tex_address;
}

void tc0780fpa_device::tex_addr_w(uint16_t data)
{
	m_tex_address = data;

	m_texbase_x = (((data >> 0) & 0x1f) << 1) | ((data >> 12) & 0x1);
	m_texbase_y = (((data >> 5) & 0x1f) << 1) | ((data >> 13) & 0x1);

	m_tex_offset = 0;
}

void tc0780fpa_device::tex_w(uint16_t data)
{
	int const x = ((m_tex_offset >> 0) & 0x1f) | ((m_tex_offset >> 5) & 0x20);
	int const y = ((m_tex_offset >> 5) & 0x1f) | ((m_tex_offset >> 6) & 0x20);

	int const index = (((m_texbase_y << 5) + y) << 11) + ((m_texbase_x << 5) + x);
	m_texture[index] = data & 0xff;

	m_tex_offset++;
}

void tc0780fpa_device::poly_fifo_w(uint16_t data)
{
	assert (m_poly_fifo_ptr < POLY_FIFO_SIZE); // never happens
	m_poly_fifo[m_poly_fifo_ptr++] = data;

	static const int cmd_length[8] = { 7, 13, -1, 20, 17, -1, 26, -1 };
	uint16_t const cmd = m_poly_fifo[0] & 0x7;

	if (m_poly_fifo_ptr >= cmd_length[cmd])
	{
		m_renderer->render(m_poly_fifo.get(), m_poly_fifo_ptr);
		m_poly_fifo_ptr = 0;
	}

}

void tc0780fpa_device::render_w(uint16_t data)
{
}

void tc0780fpa_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_renderer->draw(bitmap, cliprect);
}
