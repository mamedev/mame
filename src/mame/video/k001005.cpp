// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "k001005.h"


/*****************************************************************************/
/* Konami K001005 Polygon Renderer (KS10071) */

/***************************************************************************/
/*                                                                         */
/*                                  001005                                 */
/*                                                                         */
/***************************************************************************/

/*
    TODO:
        - Fog equation and parameters are probably not accurate.
        - Winding Heat (and maybe others) have slight Z-fighting problems.
        - 3D in Solar Assault title isn't properly turned off (the SHARC keeps rendering 3D).

*/

#define LOG_POLY_FIFO   0


k001005_renderer::k001005_renderer(device_t &parent, screen_device &screen, device_t *k001006)
	: poly_manager<float, k001005_polydata, 8, 50000>(screen)
{
	m_k001006 = k001006;

	int width = screen.width();
	int height = screen.height();

	m_fb[0] = std::make_unique<bitmap_rgb32>( width, height);
	m_fb[1] = std::make_unique<bitmap_rgb32>( width, height);

	m_zb = std::make_unique<bitmap_ind32>(width, height);

	m_3dfifo = std::make_unique<UINT32[]>(0x10000);
	m_3dfifo_ptr = 0;
	m_fb_page = 0;

	m_cliprect = screen.cliprect();

	for (int k=0; k < 8; k++)
	{
		m_tex_mirror_table[0][k] = std::make_unique<int[]>(128);
		m_tex_mirror_table[1][k] = std::make_unique<int[]>(128);

		int size = (k+1)*8;

		for (int i=0; i < 128; i++)
		{
			m_tex_mirror_table[0][k][i] = i % size;
			m_tex_mirror_table[1][k][i] = (i % (size*2)) >= size ? ((size - 1) - (i % size)) : (i % size);
		}
	}

	// save state
	parent.save_pointer(NAME(m_3dfifo.get()), 0x10000);
	parent.save_item(NAME(m_3dfifo_ptr));
	parent.save_item(NAME(*m_fb[0]));
	parent.save_item(NAME(*m_fb[1]));
	parent.save_item(NAME(*m_zb));
	parent.save_item(NAME(m_fb_page));
	parent.save_item(NAME(m_light_r));
	parent.save_item(NAME(m_light_g));
	parent.save_item(NAME(m_light_b));
	parent.save_item(NAME(m_ambient_r));
	parent.save_item(NAME(m_ambient_g));
	parent.save_item(NAME(m_ambient_b));
	parent.save_item(NAME(m_fog_r));
	parent.save_item(NAME(m_fog_g));
	parent.save_item(NAME(m_fog_b));
	parent.save_item(NAME(m_far_z));

}

void k001005_renderer::reset()
{
	m_3dfifo_ptr = 0;
}

void k001005_renderer::push_data(UINT32 data)
{
	// process the current vertex data if a sync command is being sent (usually means the global registers are being changed)
	if (data == 0x80000000)
	{
		render_polygons();
	}

	m_3dfifo[m_3dfifo_ptr++] = data;
}

void k001005_renderer::swap_buffers()
{
	m_fb_page ^= 1;

	m_fb[m_fb_page]->fill(0, m_cliprect);

	float zvalue = 10000000000.0f;
	m_zb->fill(*(int*)&zvalue, m_cliprect);
}

bool k001005_renderer::fifo_filled()
{
	return m_3dfifo_ptr > 0;
}

void k001005_renderer::set_param(k001005_param param, UINT32 value)
{
	switch (param)
	{
		case K001005_LIGHT_R:       m_light_r = value; break;
		case K001005_LIGHT_G:       m_light_g = value; break;
		case K001005_LIGHT_B:       m_light_b = value; break;
		case K001005_AMBIENT_R:     m_ambient_r = value; break;
		case K001005_AMBIENT_G:     m_ambient_g = value; break;
		case K001005_AMBIENT_B:     m_ambient_b = value; break;
		case K001005_FOG_R:         m_fog_r = value; break;
		case K001005_FOG_G:         m_fog_g = value; break;
		case K001005_FOG_B:         m_fog_b = value; break;
		case K001005_FAR_Z:
		{
			UINT32 fz = value << 11;
			m_far_z = *(float*)&fz;
			if (m_far_z == 0.0f)      // just in case...
				m_far_z = 1.0f;
			break;
		}
	}
}

void k001005_renderer::render_polygons()
{
	vertex_t v[4];
	int poly_type;
	int brightness;

	vertex_t *vertex1;
	vertex_t *vertex2;
	vertex_t *vertex3;
	vertex_t *vertex4;

	UINT32 *fifo = m_3dfifo.get();

	const rectangle& visarea = screen().visible_area();

	int index = 0;

	float fog_density = 1.5f;

	render_delegate rd_scan_2d = render_delegate(FUNC(k001005_renderer::draw_scanline_2d), this);
	render_delegate rd_scan_tex2d = render_delegate(FUNC(k001005_renderer::draw_scanline_2d_tex), this);
	render_delegate rd_scan = render_delegate(FUNC(k001005_renderer::draw_scanline), this);
	render_delegate rd_scan_tex = render_delegate(FUNC(k001005_renderer::draw_scanline_tex), this);
	render_delegate rd_scan_gour_blend = render_delegate(FUNC(k001005_renderer::draw_scanline_gouraud_blend), this);

	do
	{
		UINT32 cmd = fifo[index++];

		// Current guesswork on the command word bits:
		// 0x01: Z-buffer disable?
		// 0x02: Almost always set (only exception is 0x80000020 in Thunder Hurricane attract mode)
		// 0x04:
		// 0x08:
		// 0x10: Texture mirror enable
		// 0x20: Gouraud shading enable?
		// 0x40: Unused?
		// 0x80: Used by textured polygons.
		// 0x100: Alpha blending? Only used by Winding Heat car selection so far.

		if (cmd == 0x800000ae || cmd == 0x8000008e || cmd == 0x80000096 || cmd == 0x800000b6 ||
			cmd == 0x8000002e || cmd == 0x8000000e || cmd == 0x80000016 || cmd == 0x80000036 ||
			cmd == 0x800000aa || cmd == 0x800000a8 || cmd == 0x800000b2 || cmd == 0x8000009e ||
			cmd == 0x80000092 || cmd == 0x8000008a || cmd == 0x80000094 || cmd == 0x8000009a ||
			cmd == 0x8000009c || cmd == 0x8000008c || cmd == 0x800000ac || cmd == 0x800000b4)
		{
			// 0x00: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx    Command
			//
			// 0x01: xxxx---- -------- -------- --------    Texture palette
			// 0x01: ----xx-- -------- -------- --------    Unknown flags, set by commands 0x7b...0x7e
			// 0x01: ------xx x------- -------- --------    Texture width / 8 - 1
			// 0x01: -------- -xxx---- -------- --------    Texture height / 8 - 1
			// 0x01: -------- -------x xxxx---- --------    Texture page
			// 0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
			// 0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8

			// texture, Z

			int tex_x, tex_y;
			UINT32 color = 0;
			k001005_polydata &extra = object_data_alloc();

			UINT32 header = fifo[index++];

			int last_vertex = 0;
			int vert_num = 0;
			do
			{
				int x, y, z;
				INT16 tu, tv;

				x = (fifo[index] >> 0) & 0x3fff;
				y = (fifo[index] >> 16) & 0x1fff;
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = fifo[index] & 0x4000;       // 0 = triangle, 1 = quad
				last_vertex = fifo[index] & 0x8000;
				index++;

				z = fifo[index] & 0xffffff00;
				brightness = fifo[index] & 0xff;
				index++;

				if (last_vertex)
				{
					color = fifo[index++];
				}

				tu = (fifo[index] >> 16) & 0xffff;
				tv = (fifo[index] & 0xffff);
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				v[vert_num].p[POLY_Z] = *(float*)(&z);
				v[vert_num].p[POLY_W] = 1.0f / v[vert_num].p[POLY_Z];
				v[vert_num].p[POLY_U] = tu * v[vert_num].p[POLY_W];
				v[vert_num].p[POLY_V] = tv * v[vert_num].p[POLY_W];
				v[vert_num].p[POLY_BRI] = brightness;
				v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) ))) * 65536.0f;
				//v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
				if (v[vert_num].p[POLY_FOG] < 0.0f) v[vert_num].p[POLY_FOG] = 0.0f;
				if (v[vert_num].p[POLY_FOG] > 65536.0f) v[vert_num].p[POLY_FOG] = 65536.0f;
				vert_num++;
			}
			while (!last_vertex);

			tex_y = ((header & 0x400) >> 5) |
					((header & 0x100) >> 4) |
					((header & 0x040) >> 3) |
					((header & 0x010) >> 2) |
					((header & 0x004) >> 1) |
					((header & 0x001) >> 0);

			tex_x = ((header & 0x800) >> 6) |
					((header & 0x200) >> 5) |
					((header & 0x080) >> 4) |
					((header & 0x020) >> 3) |
					((header & 0x008) >> 2) |
					((header & 0x002) >> 1);

			extra.texture_x = tex_x * 8;
			extra.texture_y = tex_y * 8;
			extra.texture_width = (header >> 23) & 0x7;
			extra.texture_height = (header >> 20) & 0x7;
			extra.texture_page = (header >> 12) & 0x1f;
			extra.texture_palette = (header >> 28) & 0xf;
			extra.texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);
			extra.texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);
			extra.color = color;
			extra.light_r = m_light_r;     extra.light_g = m_light_g;     extra.light_b = m_light_b;
			extra.ambient_r = m_ambient_r; extra.ambient_g = m_ambient_g; extra.ambient_b = m_ambient_b;
			extra.fog_r = m_fog_r;         extra.fog_g = m_fog_g;         extra.fog_b = m_fog_b;
			extra.flags = cmd;

			if ((cmd & 0x20) == 0)      // possibly enable flag for gouraud shading (fixes some shading errors)
			{
				v[0].p[POLY_BRI] = brightness;
				v[1].p[POLY_BRI] = brightness;
			}

			if (poly_type == 0)     // triangle
			{
				if (vert_num == 1)
				{
					vertex1 = &m_prev_v[2];
					vertex2 = &m_prev_v[3];
					vertex3 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &m_prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
				}

				render_triangle(m_cliprect, rd_scan_tex, 6, *vertex1, *vertex2, *vertex3);

				memcpy(&m_prev_v[1], vertex1, sizeof(vertex_t));
				memcpy(&m_prev_v[2], vertex2, sizeof(vertex_t));
				memcpy(&m_prev_v[3], vertex3, sizeof(vertex_t));
			}
			else                    // quad
			{
				if (vert_num == 1)
				{
					vertex1 = &m_prev_v[1];
					vertex2 = &m_prev_v[2];
					vertex3 = &m_prev_v[3];
					vertex4 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &m_prev_v[2];
					vertex2 = &m_prev_v[3];
					vertex3 = &v[0];
					vertex4 = &v[1];
				}
				else if (vert_num == 3)
				{
					vertex1 = &m_prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
					vertex4 = &v[2];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
					vertex4 = &v[3];
				}

				render_triangle(visarea, rd_scan_tex, 6, *vertex1, *vertex2, *vertex3);
				render_triangle(visarea, rd_scan_tex, 6, *vertex3, *vertex4, *vertex1);

				memcpy(&m_prev_v[0], vertex1, sizeof(vertex_t));
				memcpy(&m_prev_v[1], vertex2, sizeof(vertex_t));
				memcpy(&m_prev_v[2], vertex3, sizeof(vertex_t));
				memcpy(&m_prev_v[3], vertex4, sizeof(vertex_t));
			}

			while ((fifo[index] & 0xffffff00) != 0x80000000 && index < m_3dfifo_ptr)
			{
				k001005_polydata &extra = object_data_alloc();
				int new_verts = 0;

				memcpy(&v[0], &m_prev_v[2], sizeof(vertex_t));
				memcpy(&v[1], &m_prev_v[3], sizeof(vertex_t));

				last_vertex = 0;
				vert_num = 2;
				do
				{
					int x, y, z;
					INT16 tu, tv;

					x = ((fifo[index] >>  0) & 0x3fff);
					y = ((fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = fifo[index] & 0x4000;
					last_vertex = fifo[index] & 0x8000;
					index++;

					z = fifo[index] & 0xffffff00;
					brightness = fifo[index] & 0xff;
					index++;

					if (last_vertex)
					{
						color = fifo[index++];
					}

					tu = (fifo[index] >> 16) & 0xffff;
					tv = (fifo[index] >>  0) & 0xffff;
					index++;

					v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
					v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
					v[vert_num].p[POLY_Z] = *(float*)(&z);
					v[vert_num].p[POLY_W] = 1.0f / v[vert_num].p[POLY_Z];
					v[vert_num].p[POLY_U] = tu * v[vert_num].p[POLY_W];
					v[vert_num].p[POLY_V] = tv * v[vert_num].p[POLY_W];
					v[vert_num].p[POLY_BRI] = brightness;
					v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) ))) * 65536.0f;
					//v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
					if (v[vert_num].p[POLY_FOG] < 0.0f) v[vert_num].p[POLY_FOG] = 0.0f;
					if (v[vert_num].p[POLY_FOG] > 65536.0f) v[vert_num].p[POLY_FOG] = 65536.0f;

					vert_num++;
					new_verts++;
				}
				while (!last_vertex);

				extra.texture_x = tex_x * 8;
				extra.texture_y = tex_y * 8;
				extra.texture_width = (header >> 23) & 0x7;
				extra.texture_height = (header >> 20) & 0x7;

				extra.texture_page = (header >> 12) & 0x1f;
				extra.texture_palette = (header >> 28) & 0xf;

				extra.texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);// & ((header & 0x00400000) ? 0x1 : 0);
				extra.texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);// & ((header & 0x00400000) ? 0x1 : 0);

				extra.color = color;
				extra.light_r = m_light_r;     extra.light_g = m_light_g;     extra.light_b = m_light_b;
				extra.ambient_r = m_ambient_r; extra.ambient_g = m_ambient_g; extra.ambient_b = m_ambient_b;
				extra.fog_r = m_fog_r;         extra.fog_g = m_fog_g;         extra.fog_b = m_fog_b;
				extra.flags = cmd;

				if ((cmd & 0x20) == 0)      // possibly enable flag for gouraud shading (fixes some shading errors)
				{
					v[0].p[POLY_BRI] = brightness;
					v[1].p[POLY_BRI] = brightness;
				}

				if (new_verts == 1)
				{
					render_triangle(visarea, rd_scan_tex, 6, v[0], v[1], v[2]);

					memcpy(&m_prev_v[1], &v[0], sizeof(vertex_t));
					memcpy(&m_prev_v[2], &v[1], sizeof(vertex_t));
					memcpy(&m_prev_v[3], &v[2], sizeof(vertex_t));
				}
				else if (new_verts == 2)
				{
					render_triangle(visarea, rd_scan_tex, 6, v[0], v[1], v[2]);
					render_triangle(visarea, rd_scan_tex, 6, v[2], v[3], v[0]);

					memcpy(&m_prev_v[0], &v[0], sizeof(vertex_t));
					memcpy(&m_prev_v[1], &v[1], sizeof(vertex_t));
					memcpy(&m_prev_v[2], &v[2], sizeof(vertex_t));
					memcpy(&m_prev_v[3], &v[3], sizeof(vertex_t));
				}
			};
		}
		else if (cmd == 0x80000006 || cmd == 0x80000026 || cmd == 0x80000002 || cmd == 0x80000020 || cmd == 0x80000022)
		{
			// no texture, Z

			k001005_polydata &extra = object_data_alloc();
			UINT32 color;
			int r, g, b, a;

			int last_vertex = 0;
			int vert_num = 0;
			do
			{
				int x, y, z;

				x = (fifo[index] >> 0) & 0x3fff;
				y = (fifo[index] >> 16) & 0x1fff;
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = fifo[index] & 0x4000;       // 0 = triangle, 1 = quad
				last_vertex = fifo[index] & 0x8000;
				index++;

				z = fifo[index] & 0xffffff00;
				brightness = fifo[index] & 0xff;
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				v[vert_num].p[POLY_Z] = *(float*)(&z);
				v[vert_num].p[POLY_BRI] = brightness;
				v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) ))) * 65536.0f;
				//v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
				if (v[vert_num].p[POLY_FOG] < 0.0f) v[vert_num].p[POLY_FOG] = 0.0f;
				if (v[vert_num].p[POLY_FOG] > 65536.0f) v[vert_num].p[POLY_FOG] = 65536.0f;
				vert_num++;
			}
			while (!last_vertex);

			r = (fifo[index] >>  0) & 0xff;
			g = (fifo[index] >>  8) & 0xff;
			b = (fifo[index] >> 16) & 0xff;
			a = (fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra.color = color;
			extra.light_r = m_light_r;       extra.light_g = m_light_g;     extra.light_b = m_light_b;
			extra.ambient_r = m_ambient_r;   extra.ambient_g = m_ambient_g; extra.ambient_b = m_ambient_b;
			extra.fog_r = m_fog_r;           extra.fog_g = m_fog_g;         extra.fog_b = m_fog_b;
			extra.flags = cmd;

			if ((cmd & 0x20) == 0)      // possibly enable flag for gouraud shading (fixes some shading errors)
			{
				v[0].p[POLY_BRI] = brightness;
				v[1].p[POLY_BRI] = brightness;
			}

			if (poly_type == 0)     // triangle
			{
				if (vert_num == 1)
				{
					vertex1 = &m_prev_v[2];
					vertex2 = &m_prev_v[3];
					vertex3 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &m_prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
				}

				render_triangle(visarea, rd_scan, 3, *vertex1, *vertex2, *vertex3);

				memcpy(&m_prev_v[1], vertex1, sizeof(vertex_t));
				memcpy(&m_prev_v[2], vertex2, sizeof(vertex_t));
				memcpy(&m_prev_v[3], vertex3, sizeof(vertex_t));
			}
			else                    // quad
			{
				if (vert_num == 1)
				{
					vertex1 = &m_prev_v[1];
					vertex2 = &m_prev_v[2];
					vertex3 = &m_prev_v[3];
					vertex4 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &m_prev_v[2];
					vertex2 = &m_prev_v[3];
					vertex3 = &v[0];
					vertex4 = &v[1];
				}
				else if (vert_num == 3)
				{
					vertex1 = &m_prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
					vertex4 = &v[2];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
					vertex4 = &v[3];
				}

				render_triangle(visarea, rd_scan, 3, *vertex1, *vertex2, *vertex3);
				render_triangle(visarea, rd_scan, 3, *vertex3, *vertex4, *vertex1);

				memcpy(&m_prev_v[0], vertex1, sizeof(vertex_t));
				memcpy(&m_prev_v[1], vertex2, sizeof(vertex_t));
				memcpy(&m_prev_v[2], vertex3, sizeof(vertex_t));
				memcpy(&m_prev_v[3], vertex4, sizeof(vertex_t));
			}

			while ((fifo[index] & 0xffffff00) != 0x80000000 && index < m_3dfifo_ptr)
			{
				int new_verts = 0;

				memcpy(&v[0], &m_prev_v[2], sizeof(vertex_t));
				memcpy(&v[1], &m_prev_v[3], sizeof(vertex_t));

				last_vertex = 0;
				vert_num = 2;
				do
				{
					int x, y, z;

					x = ((fifo[index] >>  0) & 0x3fff);
					y = ((fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = fifo[index] & 0x4000;
					last_vertex = fifo[index] & 0x8000;
					index++;

					z = fifo[index] & 0xffffff00;
					brightness = fifo[index] & 0xff;
					index++;

					v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
					v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
					v[vert_num].p[POLY_Z] = *(float*)(&z);
					v[vert_num].p[POLY_BRI] = brightness;
					v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / m_far_z) ))) * 65536.0f;
					//v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
					if (v[vert_num].p[POLY_FOG] < 0.0f) v[vert_num].p[POLY_FOG] = 0.0f;
					if (v[vert_num].p[POLY_FOG] > 65536.0f) v[vert_num].p[POLY_FOG] = 65536.0f;

					vert_num++;
					new_verts++;
				}
				while (!last_vertex);

				r = (fifo[index] >>  0) & 0xff;
				g = (fifo[index] >>  8) & 0xff;
				b = (fifo[index] >> 16) & 0xff;
				a = (fifo[index] >> 24) & 0xff;
				color = (a << 24) | (r << 16) | (g << 8) | (b);
				index++;

				extra.color = color;
				extra.light_r = m_light_r;     extra.light_g = m_light_g;     extra.light_b = m_light_b;
				extra.ambient_r = m_ambient_r; extra.ambient_g = m_ambient_g; extra.ambient_b = m_ambient_b;
				extra.fog_r = m_fog_r;         extra.fog_g = m_fog_g;         extra.fog_b = m_fog_b;
				extra.flags = cmd;

				if ((cmd & 0x20) == 0)      // possibly enable flag for gouraud shading (fixes some shading errors)
				{
					v[0].p[POLY_BRI] = brightness;
					v[1].p[POLY_BRI] = brightness;
				}

				if (new_verts == 1)
				{
					render_triangle(visarea, rd_scan, 3, v[0], v[1], v[2]);

					memcpy(&m_prev_v[1], &v[0], sizeof(vertex_t));
					memcpy(&m_prev_v[2], &v[1], sizeof(vertex_t));
					memcpy(&m_prev_v[3], &v[2], sizeof(vertex_t));
				}
				else if (new_verts == 2)
				{
					render_triangle(visarea, rd_scan, 3, v[0], v[1], v[2]);
					render_triangle(visarea, rd_scan, 3, v[2], v[3], v[0]);

					memcpy(&m_prev_v[0], &v[0], sizeof(vertex_t));
					memcpy(&m_prev_v[1], &v[1], sizeof(vertex_t));
					memcpy(&m_prev_v[2], &v[2], sizeof(vertex_t));
					memcpy(&m_prev_v[3], &v[3], sizeof(vertex_t));
				}
			}
		}
		else if (cmd == 0x80000003 || cmd == 0x80000001)
		{
			// no texture, no Z

			k001005_polydata &extra = object_data_alloc();
			int r, g, b, a;
			UINT32 color;

			int last_vertex = 0;
			int vert_num = 0;
			do
			{
				int x, y;

				x = ((fifo[index] >>  0) & 0x3fff);
				y = ((fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = fifo[index] & 0x4000;
				last_vertex = fifo[index] & 0x8000;
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				vert_num++;
			}
			while (!last_vertex);

			// unknown word
			index++;

			r = (fifo[index] >>  0) & 0xff;
			g = (fifo[index] >>  8) & 0xff;
			b = (fifo[index] >> 16) & 0xff;
			a = (fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra.color = color;
			extra.flags = cmd;

			if (poly_type == 0)
			{
				render_triangle(visarea, rd_scan_2d, 0, v[0], v[1], v[2]);
			}
			else
			{
				render_triangle(visarea, rd_scan_2d, 0, v[0], v[1], v[2]);
				render_triangle(visarea, rd_scan_2d, 0, v[2], v[3], v[0]);
			}
		}
		else if (cmd == 0x8000008b)
		{
			// texture, no Z

			int tex_x, tex_y;
			k001005_polydata &extra = object_data_alloc();
			int r, g, b, a;
			UINT32 color = 0;

			UINT32 header = fifo[index++];

			int last_vertex = 0;
			int vert_num = 0;
			do
			{
				int x, y;
				INT16 tu, tv;

				x = ((fifo[index] >>  0) & 0x3fff);
				y = ((fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = fifo[index] & 0x4000;
				last_vertex = fifo[index] & 0x8000;
				index++;

				if (last_vertex)
				{
					// unknown word
					index++;

					color = fifo[index++];
				}

				tu = (fifo[index] >> 16) & 0xffff;
				tv = (fifo[index] & 0xffff);
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				v[vert_num].p[POLY_U] = tu;
				v[vert_num].p[POLY_V] = tv;
				vert_num++;
			}
			while (!last_vertex);

			r = (color >>  0) & 0xff;
			g = (color >>  8) & 0xff;
			b = (color >> 16) & 0xff;
			a = (color >> 24) & 0xff;
			extra.color = (a << 24) | (r << 16) | (g << 8) | (b);
			extra.flags = cmd;

			tex_y = ((header & 0x400) >> 5) |
					((header & 0x100) >> 4) |
					((header & 0x040) >> 3) |
					((header & 0x010) >> 2) |
					((header & 0x004) >> 1) |
					((header & 0x001) >> 0);

			tex_x = ((header & 0x800) >> 6) |
					((header & 0x200) >> 5) |
					((header & 0x080) >> 4) |
					((header & 0x020) >> 3) |
					((header & 0x008) >> 2) |
					((header & 0x002) >> 1);

			extra.texture_x = tex_x * 8;
			extra.texture_y = tex_y * 8;
			extra.texture_width = (header >> 23) & 0x7;
			extra.texture_height = (header >> 20) & 0x7;

			extra.texture_page = (header >> 12) & 0x1f;
			extra.texture_palette = (header >> 28) & 0xf;

			extra.texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);
			extra.texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);

			if (poly_type == 0)
			{
				render_triangle(visarea, rd_scan_tex2d, 5, v[0], v[1], v[2]);
			}
			else
			{
				render_triangle(visarea, rd_scan_tex2d, 5, v[0], v[1], v[2]);
				render_triangle(visarea, rd_scan_tex2d, 5, v[2], v[3], v[0]);
			}
		}
		else if (cmd == 0x80000121 || cmd == 0x80000126)
		{
			// no texture, color gouraud, Z

			k001005_polydata &extra = object_data_alloc();
			UINT32 color;

			int last_vertex = 0;
			int vert_num = 0;
			do
			{
				int x, y, z;

				x = ((fifo[index] >>  0) & 0x3fff);
				y = ((fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = fifo[index] & 0x4000;
				last_vertex = fifo[index] & 0x8000;
				index++;

				z = fifo[index] & 0xffffff00;
				brightness = fifo[index] & 0xff;
				index++;

				color = fifo[index];
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				v[vert_num].p[POLY_Z] = *(float*)(&z);
				v[vert_num].p[POLY_R] = (color >> 16) & 0xff;
				v[vert_num].p[POLY_G] = (color >> 8) & 0xff;
				v[vert_num].p[POLY_B] = color & 0xff;
				v[vert_num].p[POLY_A] = (color >> 24) & 0xff;
				vert_num++;
			}
			while (!last_vertex);

			extra.color = color;
			extra.flags = cmd;

			if (poly_type == 0)
			{
				render_triangle(visarea, rd_scan_gour_blend, 6, v[0], v[1], v[2]);
			}
			else
			{
				render_triangle(visarea, rd_scan_gour_blend, 6, v[0], v[1], v[2]);
				render_triangle(visarea, rd_scan_gour_blend, 6, v[2], v[3], v[0]);
			}

			// TODO: can this poly type form strips?
		}
		else if (cmd == 0x80000000)
		{
		}
		else if (cmd == 0x80000018)
		{
		}
		else if ((cmd & 0xffff0000) == 0x80000000)
		{
			/*
			osd_printf_debug("Unknown polygon type %08X:\n", fifo[index-1]);
			for (int i=0; i < 0x20; i++)
			{
			    osd_printf_debug("  %02X: %08X\n", i, fifo[index+i]);
			}
			osd_printf_debug("\n");
			*/

			printf("Unknown polygon type %08X:\n", fifo[index-1]);
			for (int i=0; i < 0x20; i++)
			{
				printf("  %02X: %08X\n", i, fifo[index+i]);
			}
			printf("\n");
		}
		else
		{
		}
	}
	while (index < m_3dfifo_ptr);

#if LOG_POLY_FIFO
	printf("\nrender %d\n", K001005_3d_fifo_ptr);
	printf("------------------------------------\n");
#endif

	m_3dfifo_ptr = 0;

	wait();
}


void k001005_renderer::draw_scanline_2d(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid)
{
	UINT32 *fb = &m_fb[m_fb_page]->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	UINT32 color = extradata.color;
	int x;

	for (x = extent.startx; x < extent.stopx; x++)
	{
		if (color & 0xff000000)
		{
			fb[x] = color;
			zb[x] = FLT_MAX;        // FIXME
		}
	}
}

void k001005_renderer::draw_scanline_2d_tex(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid)
{
	//  int pal_chip = (extradata.texture_palette & 0x8) ? 1 : 0;
	k001006_device *k001006 = downcast<k001006_device*>(m_k001006);

	int tex_page = extradata.texture_page * 0x40000;
	int palette_index = (extradata.texture_palette & 0x7) * 256;
	float u = extent.param[POLY_U].start;
	float v = extent.param[POLY_V].start;
	float du = extent.param[POLY_U].dpdx;
	float dv = extent.param[POLY_V].dpdx;
	UINT32 *fb = &m_fb[m_fb_page]->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	UINT32 color = extradata.color;
	int texture_mirror_x = extradata.texture_mirror_x;
	int texture_mirror_y = extradata.texture_mirror_y;
	int texture_x = extradata.texture_x;
	int texture_y = extradata.texture_y;
	int texture_width = extradata.texture_width;
	int texture_height = extradata.texture_height;

	int *x_mirror_table = m_tex_mirror_table[texture_mirror_x][texture_width].get();
	int *y_mirror_table = m_tex_mirror_table[texture_mirror_y][texture_height].get();

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int iu = (int)(u * 0.0625f);
		int iv = (int)(v * 0.0625f);
		int iiv, iiu;

		iiu = texture_x + x_mirror_table[iu & 0x7f];
		iiv = texture_y + y_mirror_table[iv & 0x7f];

		color = k001006->fetch_texel(tex_page, palette_index, iiu, iiv);

		if (color & 0xff000000)
		{
			fb[x] = color;
			zb[x] = FLT_MAX;        // FIXME
		}

		u += du;
		v += dv;
	}
}

void k001005_renderer::draw_scanline(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid)
{
	float z = extent.param[POLY_Z].start;
	float dz = extent.param[POLY_Z].dpdx;
	float bri = extent.param[POLY_BRI].start;
	float dbri = extent.param[POLY_BRI].dpdx;
	float fog = extent.param[POLY_FOG].start;
	float dfog = extent.param[POLY_FOG].dpdx;
	UINT32 *fb = &m_fb[m_fb_page]->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);
	UINT32 color = extradata.color;

	int poly_light_r = extradata.light_r + extradata.ambient_r;
	int poly_light_g = extradata.light_g + extradata.ambient_g;
	int poly_light_b = extradata.light_b + extradata.ambient_b;
	if (poly_light_r > 255) poly_light_r = 255;
	if (poly_light_g > 255) poly_light_g = 255;
	if (poly_light_b > 255) poly_light_b = 255;
	int poly_fog_r = extradata.fog_r;
	int poly_fog_g = extradata.fog_g;
	int poly_fog_b = extradata.fog_b;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int ibri = (int)(bri);
		int ifog = (int)(fog);

		if (ibri < 0) ibri = 0; if (ibri > 255) ibri = 255;
		if (ifog < 0) ifog = 0; if (ifog > 65536) ifog = 65536;

		if (z <= zb[x])
		{
			if (color & 0xff000000)
			{
				int r = (color >> 16) & 0xff;
				int g = (color >> 8) & 0xff;
				int b = color & 0xff;

				r = ((((r * poly_light_r * ibri) >> 16) * ifog) + (poly_fog_r * (65536 - ifog))) >> 16;
				g = ((((g * poly_light_g * ibri) >> 16) * ifog) + (poly_fog_g * (65536 - ifog))) >> 16;
				b = ((((b * poly_light_b * ibri) >> 16) * ifog) + (poly_fog_b * (65536 - ifog))) >> 16;

				if (r < 0) r = 0; if (r > 255) r = 255;
				if (g < 0) g = 0; if (g > 255) g = 255;
				if (b < 0) b = 0; if (b > 255) b = 255;

				fb[x] = (color & 0xff000000) | (r << 16) | (g << 8) | b;
				zb[x] = z;
			}
		}

		z += dz;
		bri += dbri;
		fog += dfog;
	}
}

void k001005_renderer::draw_scanline_tex(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid)
{
//  int pal_chip = (extradata.texture_palette & 0x8) ? 1 : 0;
	k001006_device *k001006 = downcast<k001006_device*>(m_k001006);

	int tex_page = extradata.texture_page * 0x40000;
	int palette_index = (extradata.texture_palette & 0x7) * 256;
	float z = extent.param[POLY_Z].start;
	float u = extent.param[POLY_U].start;
	float v = extent.param[POLY_V].start;
	float w = extent.param[POLY_W].start;
	float dz = extent.param[POLY_Z].dpdx;
	float du = extent.param[POLY_U].dpdx;
	float dv = extent.param[POLY_V].dpdx;
	float dw = extent.param[POLY_W].dpdx;
	float bri = extent.param[POLY_BRI].start;
	float dbri = extent.param[POLY_BRI].dpdx;
	float fog = extent.param[POLY_FOG].start;
	float dfog = extent.param[POLY_FOG].dpdx;
	int texture_mirror_x = extradata.texture_mirror_x;
	int texture_mirror_y = extradata.texture_mirror_y;
	int texture_x = extradata.texture_x;
	int texture_y = extradata.texture_y;
	int texture_width = extradata.texture_width;
	int texture_height = extradata.texture_height;

	int poly_light_r = extradata.light_r + extradata.ambient_r;
	int poly_light_g = extradata.light_g + extradata.ambient_g;
	int poly_light_b = extradata.light_b + extradata.ambient_b;
	if (poly_light_r > 255) poly_light_r = 255;
	if (poly_light_g > 255) poly_light_g = 255;
	if (poly_light_b > 255) poly_light_b = 255;
	int poly_fog_r = extradata.fog_r;
	int poly_fog_g = extradata.fog_g;
	int poly_fog_b = extradata.fog_b;

	UINT32 *fb = &m_fb[m_fb_page]->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);

	int *x_mirror_table = m_tex_mirror_table[texture_mirror_x][texture_width].get();
	int *y_mirror_table = m_tex_mirror_table[texture_mirror_y][texture_height].get();

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int ibri = (int)(bri);
		int ifog = (int)(fog);

		if (ibri < 0) ibri = 0; if (ibri > 255) ibri = 255;
		if (ifog < 0) ifog = 0; if (ifog > 65536) ifog = 65536;

		if (z <= zb[x])
		{
			float oow = 1.0f / w;
			UINT32 color;
			int iu, iv;
			int iiv, iiu;

			iu = u * oow * 0.0625f;
			iv = v * oow * 0.0625f;

			iiu = texture_x + x_mirror_table[iu & 0x7f];
			iiv = texture_y + y_mirror_table[iv & 0x7f];

			color = k001006->fetch_texel(tex_page, palette_index, iiu, iiv);

			if (color & 0xff000000)
			{
				int r = (color >> 16) & 0xff;
				int g = (color >> 8) & 0xff;
				int b = color & 0xff;

				r = ((((r * poly_light_r * ibri) >> 16) * ifog) + (poly_fog_r * (65536 - ifog))) >> 16;
				g = ((((g * poly_light_g * ibri) >> 16) * ifog) + (poly_fog_g * (65536 - ifog))) >> 16;
				b = ((((b * poly_light_b * ibri) >> 16) * ifog) + (poly_fog_b * (65536 - ifog))) >> 16;

				if (r < 0) r = 0; if (r > 255) r = 255;
				if (g < 0) g = 0; if (g > 255) g = 255;
				if (b < 0) b = 0; if (b > 255) b = 255;

				fb[x] = 0xff000000 | (r << 16) | (g << 8) | b;
				zb[x] = z;
			}
		}

		u += du;
		v += dv;
		z += dz;
		w += dw;
		bri += dbri;
		fog += dfog;
	}
}

void k001005_renderer::draw_scanline_gouraud_blend(INT32 scanline, const extent_t &extent, const k001005_polydata &extradata, int threadid)
{
	float z = extent.param[POLY_Z].start;
	float dz = extent.param[POLY_Z].dpdx;
	float r = extent.param[POLY_R].start;
	float dr = extent.param[POLY_R].dpdx;
	float g = extent.param[POLY_G].start;
	float dg = extent.param[POLY_G].dpdx;
	float b = extent.param[POLY_B].start;
	float db = extent.param[POLY_B].dpdx;
	float a = extent.param[POLY_A].start;
	float da = extent.param[POLY_A].dpdx;
	UINT32 *fb = &m_fb[m_fb_page]->pix32(scanline);
	float *zb = (float*)&m_zb->pix32(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x])
		{
			int ir = (int)(r);
			int ig = (int)(g);
			int ib = (int)(b);
			int ia = (int)(a);

			if (ia > 0)
			{
				if (ia != 0xff)
				{
					int sr = (fb[x] >> 16) & 0xff;
					int sg = (fb[x] >> 8) & 0xff;
					int sb = fb[x] & 0xff;

					ir = ((ir * ia) >> 8) + ((sr * (0xff-ia)) >> 8);
					ig = ((ig * ia) >> 8) + ((sg * (0xff-ia)) >> 8);
					ib = ((ib * ia) >> 8) + ((sb * (0xff-ia)) >> 8);
				}

				if (ir < 0) ir = 0; if (ir > 255) ir = 255;
				if (ig < 0) ig = 0; if (ig > 255) ig = 255;
				if (ib < 0) ib = 0; if (ib > 255) ib = 255;

				fb[x] = 0xff000000 | (ir << 16) | (ig << 8) | ib;
				zb[x] = z;
			}
		}

		z += dz;
		r += dr;
		g += dg;
		b += db;
		a += da;
	}
}


void k001005_renderer::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, j;

	for (j = cliprect.min_y; j <= cliprect.max_y; j++)
	{
		UINT32 *bmp = &bitmap.pix32(j);
		UINT32 *src = &m_fb[m_fb_page^1]->pix32(j);

		for (i = cliprect.min_x; i <= cliprect.max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}



const device_type K001005 = &device_creator<k001005_device>;

k001005_device::k001005_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001005, "K001005 Polygon Renderer", tag, owner, clock, "k001005", __FILE__),
		device_video_interface(mconfig, *this),
		m_k001006(nullptr),
		m_fifo(nullptr),
		m_status(0),
		m_ram_ptr(0),
		m_fifo_read_ptr(0),
		m_fifo_write_ptr(0),
		m_reg_far_z(0)
{
		m_ram[0] = nullptr;
		m_ram[1] = nullptr;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001005_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001005_device::device_start()
{
	m_k001006 = machine().device(m_k001006_tag);

	m_ram[0] = std::make_unique<UINT16[]>(0x140000);
	m_ram[1] = std::make_unique<UINT16[]>(0x140000);

	m_fifo = std::make_unique<UINT32[]>(0x800);

	m_renderer = auto_alloc(machine(), k001005_renderer(*this, *m_screen, m_k001006));

	save_pointer(NAME(m_ram[0].get()), 0x140000);
	save_pointer(NAME(m_ram[1].get()), 0x140000);
	save_pointer(NAME(m_fifo.get()), 0x800);
	save_item(NAME(m_status));
	save_item(NAME(m_ram_ptr));
	save_item(NAME(m_fifo_read_ptr));
	save_item(NAME(m_fifo_write_ptr));
	save_item(NAME(m_reg_far_z));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001005_device::device_reset()
{
	m_status = 0;
	m_ram_ptr = 0;
	m_fifo_read_ptr = 0;
	m_fifo_write_ptr = 0;

	m_renderer->reset();
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void k001005_device::device_stop()
{
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k001005_device::swap_buffers( )
{
	m_renderer->swap_buffers();
}

READ32_MEMBER( k001005_device::read )
{
	adsp21062_device *dsp = downcast<adsp21062_device*>(&space.device());

	switch(offset)
	{
		case 0x000:         // FIFO read, high 16 bits
		{
			//osd_printf_debug("FIFO_r0: %08X\n", m_fifo_read_ptr);
			UINT16 value = m_fifo[m_fifo_read_ptr] >> 16;
			return value;
		}

		case 0x001:         // FIFO read, low 16 bits
		{
			//osd_printf_debug("FIFO_r1: %08X\n", m_fifo_read_ptr);
			UINT16 value = m_fifo[m_fifo_read_ptr] & 0xffff;

			if (m_status != 1 && m_status != 2)
			{
				if (m_fifo_read_ptr < 0x3ff)
				{
					dsp->set_flag_input(1, CLEAR_LINE);
				}
				else
				{
					dsp->set_flag_input(1, ASSERT_LINE);
				}
			}
			else
			{
				dsp->set_flag_input(1, ASSERT_LINE);
			}

			m_fifo_read_ptr++;
			m_fifo_read_ptr &= 0x7ff;
			return value;
		}

		case 0x11b:         // status ?
			return 0x8002;

		case 0x11c:         // slave status ?
			return 0x8000;

		case 0x11f:
			if (m_ram_ptr >= 0x400000)
			{
				return m_ram[1][(m_ram_ptr++) & 0x3fffff];
			}
			else
			{
				return m_ram[0][(m_ram_ptr++) & 0x3fffff];
			}

		default:
			//osd_printf_debug("m_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
			break;
	}
	return 0;
}

WRITE32_MEMBER( k001005_device::write )
{
	adsp21062_device *dsp = downcast<adsp21062_device*>(&space.device());

	switch (offset)
	{
		case 0x000:         // FIFO write
		{
			//osd_printf_debug("K001005 FIFO write: %08X at %08X\n", data, space.device().safe_pc());
			if (m_status != 1 && m_status != 2)
			{
				if (m_fifo_write_ptr < 0x400)
				{
					dsp->set_flag_input(1, ASSERT_LINE);
				}
				else
				{
					dsp->set_flag_input(1, CLEAR_LINE);
				}
			}
			else
			{
				dsp->set_flag_input(1, ASSERT_LINE);
			}

		//  osd_printf_debug("K001005 FIFO write: %08X at %08X\n", data, space.device().safe_pc());
			m_fifo[m_fifo_write_ptr] = data;
			m_fifo_write_ptr++;
			m_fifo_write_ptr &= 0x7ff;

			m_renderer->push_data(data);

#if LOG_POLY_FIFO
			printf("0x%08X, ", data);
			count++;
			if (count >= 8)
			{
				count = 0;
				printf("\n");
			}
#endif

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (space.device().safe_pc() == 0x201ee)
			{
				// This is used to make the SHARC timeout
				space.device().execute().spin_until_trigger(10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (space.device().safe_pc() == 0x201e6)
			{
				// This is used to make the SHARC timeout
				space.device().execute().spin_until_trigger(10000);
			}

			break;
		}

		case 0x100:     break;

		case 0x101:     break;      // viewport x and width?
		case 0x102:     break;      // viewport y and height?

		case 0x104:     break;      // viewport center x? (usually 0xff)
		case 0x105:     break;      // viewport center y? (usually 0xbf)

		case 0x108:                 // far Z value, 4 exponent bits?
			{
				// this register seems to hold the 4 missing exponent bits...
				m_reg_far_z = (m_reg_far_z & 0x0000ffff) | ((data & 0xf) << 16);
				m_renderer->set_param(K001005_FAR_Z, m_reg_far_z);
				break;
			}

		case 0x109:                 // far Z value
			{
				// the SHARC code throws away the bottom 11 bits of mantissa and the top 5 bits (to fit in a 16-bit register?)
				m_reg_far_z = (m_reg_far_z & 0xffff0000) | (data & 0xffff);
				m_renderer->set_param(K001005_FAR_Z, m_reg_far_z);
				break;
			}

		case 0x10a:     m_renderer->set_param(K001005_LIGHT_R, data & 0xff); break;
		case 0x10b:     m_renderer->set_param(K001005_LIGHT_G, data & 0xff); break;
		case 0x10c:     m_renderer->set_param(K001005_LIGHT_B, data & 0xff); break;

		case 0x10d:     m_renderer->set_param(K001005_AMBIENT_R, data & 0xff); break;
		case 0x10e:     m_renderer->set_param(K001005_AMBIENT_G, data & 0xff); break;
		case 0x10f:     m_renderer->set_param(K001005_AMBIENT_B, data & 0xff); break;

		case 0x110:     m_renderer->set_param(K001005_FOG_R, data & 0xff); break;
		case 0x111:     m_renderer->set_param(K001005_FOG_G, data & 0xff); break;
		case 0x112:     m_renderer->set_param(K001005_FOG_B, data & 0xff); break;


		case 0x11a:
			m_status = data;
			m_fifo_write_ptr = 0;
			m_fifo_read_ptr = 0;

			if (data == 2)
			{
				if (m_renderer->fifo_filled())
				{
					m_renderer->render_polygons();
				}

				m_renderer->swap_buffers();
			}
			break;

		case 0x11d:
			m_fifo_write_ptr = 0;
			m_fifo_read_ptr = 0;
			break;

		case 0x11e:
			m_ram_ptr = data;
			break;

		case 0x11f:
			if (m_ram_ptr >= 0x400000)
			{
				m_ram[1][(m_ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			else
			{
				m_ram[0][(m_ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			break;

		default:
			//osd_printf_debug("m_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}

}

void k001005_device::draw( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	m_renderer->draw(bitmap, cliprect);
}

void k001005_device::set_texel_chip(device_t &device, std::string tag)
{
	downcast<k001005_device &>(device).m_k001006_tag = tag;
}
