// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "k001005.h"
#include "screen.h"

/*****************************************************************************/
/* Konami K001005 Polygon Renderer (KS10071) */

/***************************************************************************/
/*                                                                         */
/*                                  001005                                 */
/*                                                                         */
/***************************************************************************/

/*
    TODO:
        - Winding Heat (and maybe others) have slight Z-fighting problems.
        - Player car shadow not visible in Winding Heat. Hidden by road, needs polygon priority or something similar?

*/

k001005_renderer::k001005_renderer(device_t &parent, screen_device &screen, device_t *k001006)
	: poly_manager<float, k001005_polydata, 10>(screen.machine())
{
	m_k001006 = k001006;

	int const width = 512;
	int const height = 384;

	m_fb[0].allocate(width, height);
	m_fb[1].allocate(width, height);

	m_zb.allocate(width, height);

	m_3dfifo = std::make_unique<uint32_t []>(0x10000);
	m_3dfifo_ptr = 0;
	m_fb_page = 0;
	m_light_r = 0;
	m_light_g = 0;
	m_light_b = 0;
	m_ambient_r = 0;
	m_ambient_g = 0;
	m_ambient_b = 0;
	m_fog_r = 0;
	m_fog_g = 0;
	m_fog_b = 0;
	m_far_z = 0;
	m_fog_start_z = 0;
	m_fog_end_z = 0;
	m_reg_fog_start = 0;
	m_viewport_min_x = 0;
	m_viewport_max_x = 0;
	m_viewport_min_y = 0;
	m_viewport_max_y = 0;
	m_viewport_center_x = 0;
	m_viewport_center_y = 0;

	m_cliprect = rectangle(0, width - 1, 0, height - 1);

	for (int k = 0; k < 8; k++)
	{
		m_tex_mirror_table[0][k] = std::make_unique<int []>(128);
		m_tex_mirror_table[1][k] = std::make_unique<int []>(128);

		int const size = (k + 1) * 8;

		for (int i = 0; i < 128; i++)
		{
			m_tex_mirror_table[0][k][i] = i % size;
			m_tex_mirror_table[1][k][i] = (i % (size * 2)) >= size ? ((size - 1) - (i % size)) : (i % size);
		}
	}

	// save state
	parent.save_pointer(NAME(m_3dfifo), 0x10000);
	parent.save_item(NAME(m_3dfifo_ptr));
	parent.save_item(NAME(m_fb[0]));
	parent.save_item(NAME(m_fb[1]));
	parent.save_item(NAME(m_zb));
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
	parent.save_item(NAME(m_fog_start_z));
	parent.save_item(NAME(m_fog_end_z));
	parent.save_item(NAME(m_reg_fog_start));
	parent.save_item(NAME(m_viewport_min_x));
	parent.save_item(NAME(m_viewport_max_x));
	parent.save_item(NAME(m_viewport_min_y));
	parent.save_item(NAME(m_viewport_max_y));
	parent.save_item(NAME(m_viewport_center_x));
	parent.save_item(NAME(m_viewport_center_y));
}

void k001005_renderer::reset()
{
	m_3dfifo_ptr = 0;

	m_vertexb_ptr = 0;
}

void k001005_renderer::push_data(uint32_t data)
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

	m_fb[m_fb_page].fill(0, m_cliprect);

	float const zvalue = 10000000000.0F;
	m_zb.fill(*(int*)&zvalue, m_cliprect);
}

bool k001005_renderer::fifo_filled()
{
	return m_3dfifo_ptr > 0;
}


template<bool UseTexture, bool UseVertexColor>
void k001005_renderer::draw_scanline_generic(int32_t scanline, const extent_t& extent, const k001005_polydata& extradata, int threadid)
{
	k001006_device* k001006 = downcast<k001006_device*>(m_k001006);

	uint32_t *const fb = &m_fb[m_fb_page].pix(scanline);
	float *const zb = (float*)&m_zb.pix(scanline);

	float z = extent.param[POLY_Z].start;
	float const dz = extent.param[POLY_Z].dpdx;
	float diff = extent.param[POLY_DIFF].start;
	float const ddiff = extent.param[POLY_DIFF].dpdx;
	float fog = extent.param[POLY_FOG].start;
	float const dfog = extent.param[POLY_FOG].dpdx;

	float u, v, w, du, dv, dw;
	if (UseTexture)
	{
		u = extent.param[POLY_U].start;
		v = extent.param[POLY_V].start;
		w = extent.param[POLY_W].start;
		du = extent.param[POLY_U].dpdx;
		dv = extent.param[POLY_V].dpdx;
		dw = extent.param[POLY_W].dpdx;
	}

	float r, g, b, a, dr, dg, db, da;
	if (UseVertexColor)
	{
		r = extent.param[POLY_R].start;
		dr = extent.param[POLY_R].dpdx;
		g = extent.param[POLY_G].start;
		dg = extent.param[POLY_G].dpdx;
		b = extent.param[POLY_B].start;
		db = extent.param[POLY_B].dpdx;
		a = extent.param[POLY_A].start;
		da = extent.param[POLY_A].dpdx;
	}

	rgbaint_t ambient_color(extradata.ambient_light);
	rgbaint_t diffuse_color(extradata.diffuse_light);
	rgbaint_t fog_color(extradata.fog_color);

	rgbaint_t poly_color(extradata.poly_color);
	int const poly_color_a = (extradata.poly_color >> 24) & 0xff;

	int const texture_mirror_x = extradata.texture_mirror;
	int const texture_mirror_y = extradata.texture_mirror;
	int const texture_x = extradata.texture_x * 8;
	int const texture_y = extradata.texture_y * 8;
	int const texture_width = extradata.texture_width;
	int const texture_height = extradata.texture_height;
	int const tex_page = extradata.texture_page * 0x40000;
	int const palette_index = extradata.texture_palette * 256;

	int const *const x_mirror_table = m_tex_mirror_table[texture_mirror_x][texture_width].get();
	int const *const y_mirror_table = m_tex_mirror_table[texture_mirror_y][texture_height].get();

	bool const UseZCompare = BIT(extradata.cmd, 2);
	bool const UseFBBlend = BIT(~extradata.cmd, 1);
	bool const UseFog = extradata.fog_enable;

	bool const WriteZ = true;
	bool const UseBilinear = k001006->bilinear_enabled();

	uint32_t texel = 0;
	uint32_t texel_alpha = 0;

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		if (z <= zb[x] || !UseZCompare)
		{
			if (UseTexture)
			{
				float const oow = 1.0F / w;

				int iu = int(u * oow);
				int iv = int(v * oow);

				if (!UseBilinear)
				{
					int const texel_u = texture_x + x_mirror_table[(iu >> 4) & 0x7f];
					int const texel_v = texture_y + y_mirror_table[(iv >> 4) & 0x7f];

					texel = k001006->fetch_texel(tex_page, palette_index, texel_u, texel_v);
					texel_alpha = texel >> 24;
				}
				else
				{
					// sub-texel bias to avoid seams
					iu -= 7;
					iv -= 7;

					int const ufrac = iu & 0xf;
					int const vfrac = iv & 0xf;
					int const texel_u0 = texture_x + x_mirror_table[(iu >> 4) & 0x7f];
					int const texel_u1 = texture_x + x_mirror_table[((iu >> 4) + 1) & 0x7f];
					int const texel_v0 = texture_y + y_mirror_table[(iv >> 4) & 0x7f];
					int const texel_v1 = texture_y + y_mirror_table[((iv >> 4) + 1) & 0x7f];

					uint32_t const tex00 = k001006->fetch_texel(tex_page, palette_index, texel_u0, texel_v0);
					uint32_t const tex01 = k001006->fetch_texel(tex_page, palette_index, texel_u1, texel_v0);
					uint32_t const tex10 = k001006->fetch_texel(tex_page, palette_index, texel_u0, texel_v1);
					uint32_t const tex11 = k001006->fetch_texel(tex_page, palette_index, texel_u1, texel_v1);

					texel = rgbaint_t::bilinear_filter(tex00, tex01, tex10, tex11, ufrac * 16, vfrac * 16);
					texel_alpha = tex00 >> 24;
				}
			}

			int const idiff = std::clamp((int)(diff), 0, 255);
			int const ifog = std::clamp((int)(fog), 0, 255);

			rgbaint_t light_color(extradata.diffuse_light);
			light_color.scale_imm_and_clamp(idiff);
			light_color.add(ambient_color);
			light_color.clamp_to_uint8();

			if (UseVertexColor)
			{
				int const ir = std::clamp((int)(r), 0, 255);
				int const ig = std::clamp((int)(g), 0, 255);
				int const ib = std::clamp((int)(b), 0, 255);
				int const ia = std::clamp((int)(a), 0, 255);

				if (ia != 0)
				{
					rgbaint_t color(ia, ir, ig, ib);

					if (UseTexture && texel_alpha != 0)
					{
						color.set(texel);
					}

					color.scale_and_clamp(light_color);

					if (UseFog)
					{
						color.blend(fog_color, ifog);
					}

					// framebuffer blend
					if (UseFBBlend)
					{
						rgbaint_t fb_color(fb[x]);
						color.blend(fb_color, ia);
					}

					fb[x] = color.to_rgba();
					if (WriteZ)
						zb[x] = z;
				}
			}
			else
			{
				if (UseTexture)
				{
					if (texel_alpha != 0)
					{
						rgbaint_t texel_color(texel);
						texel_color.scale_and_clamp(light_color);

						// TODO: is there a toggle for texture blending? cmd bit 0x02 doesn't seem like it
						if (UseBilinear && texel_alpha < 0xff)
						{
							rgbaint_t fb_color(fb[x]);
							texel_color.blend(fb_color, texel_alpha);
						}

						if (UseFog)
						{
							texel_color.blend(fog_color, ifog);
						}

						if (UseFBBlend)
						{
							rgbaint_t fb_color(fb[x]);
							texel_color.blend(fb_color, poly_color_a);
						}

						fb[x] = texel_color.to_rgba();
						if (WriteZ)
							zb[x] = z;
					}
				}
				else
				{
					//if (poly_color_a != 0)
					{
						rgbaint_t color(extradata.poly_color);
						color.scale_and_clamp(light_color);

						if (UseFog)
						{
							color.blend(fog_color, ifog);
						}

						// framebuffer blend
						if (UseFBBlend)
						{
							rgbaint_t fb_color(fb[x]);
							color.blend(fb_color, poly_color_a);
						}

						fb[x] = color.to_rgba();
						if (WriteZ)
							zb[x] = z;
					}
				}
			}

		}

		z += dz;
		diff += ddiff;
		fog += dfog;

		if (UseVertexColor)
		{
			r += dr;
			g += dg;
			b += db;
			a += da;
		}

		if (UseTexture)
		{
			u += du;
			v += dv;
			w += dw;
		}
	}
}


/*
    Command
    0x00: xxxxxxxx xxxxxxxx xxxxxxx- --------    0x80000000 (exact number of bits unknown)
    0x00: -------- -------- -------x --------    0 = per-poly color, 1 = per-vertex color
    0x00: -------- -------- -------- x-------    ? Texture related
    0x00: -------- -------- -------- -x------    Unused?
    0x00: -------- -------- -------- --x-----    1 = enable smooth shading?
    0x00: -------- -------- -------- ---x----    1 = texture mirroring
    0x00: -------- -------- -------- ----x---    ? Texture related
    0x00: -------- -------- -------- -----x--    1 = enable Z-buffer read
    0x00: -------- -------- -------- ------x-    0 = blend enabled, 1 = disabled
    0x00: -------- -------- -------- -------x    0 = per-vertex Z, 1 = per-poly Z (0x80000121 seems like an exception)

    Texture header
    0x01: -xxx---- -------- -------- --------    Texture palette
    0x01: ----xx-- -------- -------- --------    Unknown flags, set by commands 0x7b...0x7e. Used mostly on polygons further away from camera. Some kind of depth-based effect?
    0x01: ------xx x------- -------- --------    Texture width / 8 - 1
    0x01: -------- -xxx---- -------- --------    Texture height / 8 - 1
    0x01: -------- -------x xxxx---- --------    Texture page
    0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
    0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8
*/

int k001005_renderer::parse_polygon(int index, uint32_t cmd)
{
	render_delegate rd_scan_tex = render_delegate(&k001005_renderer::draw_scanline_generic<true, false>, this);
	render_delegate rd_scan_vertex_color = render_delegate(&k001005_renderer::draw_scanline_generic<false, true>, this);
	render_delegate rd_scan_vertex_color_tex = render_delegate(&k001005_renderer::draw_scanline_generic<true, true>, this);
	render_delegate rd_scan_color = render_delegate(&k001005_renderer::draw_scanline_generic<false, false>, this);

	int const viewport_min_x = std::clamp(256 + m_viewport_min_x + m_viewport_center_x, m_cliprect.min_x, m_cliprect.max_x);
	int const viewport_max_x = std::clamp(256 + m_viewport_max_x + m_viewport_center_x + 1, m_cliprect.min_x, m_cliprect.max_x);
	int const viewport_min_y = std::clamp(200 + m_viewport_min_y - m_viewport_center_y, m_cliprect.min_y, m_cliprect.max_y);
	int const viewport_max_y = std::clamp(200 + m_viewport_max_y - m_viewport_center_y + 1, m_cliprect.min_y, m_cliprect.max_y);

	rectangle cliprect(viewport_min_x, viewport_max_x, viewport_min_y, viewport_max_y);

	int const start_index = index;

	uint32_t const *const fifo = m_3dfifo.get();

	bool const has_texture = (cmd & 0x18) != 0;
	bool const has_vertex_color = BIT(cmd, 8);
	bool const has_vertex_z = BIT(~cmd, 0) || has_vertex_color;     // command 0x121 breaks the logic here, maybe vertex color enforces vertex z too?

	uint32_t texture_x = 0;
	uint32_t texture_y = 0;
	uint32_t texture_width = 0;
	uint32_t texture_height = 0;
	uint32_t texture_page = 0;
	uint32_t texture_palette = 0;

	uint32_t tex_header = 0;

	// texture header - only for textured polys
	if (has_texture)
	{
		tex_header = fifo[index++];

		texture_x = (((tex_header >> 6) & 0x20) | ((tex_header >> 5) & 0x10) | ((tex_header >> 4) & 0x8) |
					((tex_header >> 3) & 0x4) | ((tex_header >> 2) & 0x2) | ((tex_header >> 1) & 0x1));

		texture_y = (((tex_header >> 5) & 0x20) | ((tex_header >> 4) & 0x10) | ((tex_header >> 3) & 0x8) |
					((tex_header >> 2) & 0x4) | ((tex_header >> 1) & 0x2) | (tex_header & 0x1));

		texture_width = (tex_header >> 23) & 0x7;
		texture_height = (tex_header >> 20) & 0x7;
		texture_page = (tex_header >> 12) & 0x1f;
		texture_palette = (tex_header >> 28) & 0x7;
	}

	while ((fifo[index] & 0xffff0000) != 0x80000000 && index < m_3dfifo_ptr)
	{
		k001005_polydata& extra = object_data().next();

		bool last_vertex = false;
		bool is_quad = false;

		uint32_t polygon_color = 0;
		float polygon_z = 0.0f;
		uint32_t polygon_diffuse = 0;

		int num_new_verts = 0;

		do
		{
			// X/Y coords, flags - all polys have this
			// -------------------------------------------------------------------------
			int x = fifo[index] & 0x3fff;
			x |= ((x & 0x2000) ? 0xffffc000 : 0);
			int y = (fifo[index] >> 16) & 0x1fff;
			y |= ((y & 0x1000) ? 0xffffe000 : 0);

			m_vertexb[m_vertexb_ptr].x = ((float)(x) / 16.0f) + 256.0f;
			m_vertexb[m_vertexb_ptr].y = ((float)(-y) / 16.0f) + 200.0f;

			is_quad = BIT(fifo[index], 14);
			last_vertex = BIT(fifo[index], 15);
			index++;

			// Z + diffuse intensity - if Z enabled
			// -------------------------------------------------------------------------
			if (has_vertex_z)
			{
				uint32_t const z = fifo[index] & 0xffffff00;      // 32-bit float with low 8-bits of mantissa masked out
				int const diffuse = fifo[index] & 0xff;
				index++;

				m_vertexb[m_vertexb_ptr].p[POLY_Z] = u2f(z);
				m_vertexb[m_vertexb_ptr].p[POLY_DIFF] = diffuse;
				m_vertexb[m_vertexb_ptr].p[POLY_W] = 1.0f / m_vertexb[m_vertexb_ptr].p[POLY_Z];
			}
			else
			{
				m_vertexb[m_vertexb_ptr].p[POLY_W] = 1.0f;
			}

			// textured polygons have a polygon color field after last vertex data, but before the last UV coords
			// -------------------------------------------------------------------------
			if (last_vertex && has_texture)
			{
				// polygon Z comes before the last UV coords for textured polygons
				if (!has_vertex_z)
				{
					uint32_t const z = (fifo[index] & 0x07ffff00) | 0x48000000;   // like fog values, these seem to be missing the 4 upper bits of exponent
					polygon_diffuse = fifo[index] & 0xff;
					index++;
					polygon_z = u2f(z);
				}

				if (!has_vertex_color)
				{
					polygon_color = fifo[index];
					index++;
				}
			}

			// vertex color
			if (has_vertex_color)
			{
				uint32_t const vertex_color = fifo[index];
				index++;

				m_vertexb[m_vertexb_ptr].p[POLY_A] = (vertex_color >> 24) & 0xff;
				m_vertexb[m_vertexb_ptr].p[POLY_B] = (vertex_color >> 16) & 0xff;
				m_vertexb[m_vertexb_ptr].p[POLY_G] = (vertex_color >> 8) & 0xff;
				m_vertexb[m_vertexb_ptr].p[POLY_R] = vertex_color & 0xff;

			}

			// UV coords - only for texture polys
			if (has_texture)
			{
				int32_t const tu = (int16_t)(fifo[index] >> 16);
				int32_t const tv = (int16_t)(fifo[index] & 0xffff);
				index++;

				m_vertexb[m_vertexb_ptr].p[POLY_U] = float(tu) * m_vertexb[m_vertexb_ptr].p[POLY_W];
				m_vertexb[m_vertexb_ptr].p[POLY_V] = float(tv) * m_vertexb[m_vertexb_ptr].p[POLY_W];
			}

			// fog
			if (m_reg_fog_start == 0xffff)
			{
				// max fog start value means fog is off
				m_vertexb[m_vertexb_ptr].p[POLY_FOG] = 0.0f;
			}
			else
			{
				float const fog_factor = (m_fog_end_z - m_vertexb[m_vertexb_ptr].p[POLY_Z]) / (m_fog_end_z - m_fog_start_z);
				m_vertexb[m_vertexb_ptr].p[POLY_FOG] = fog_factor * 255.0f;
			}

			num_new_verts++;
			m_vertexb_ptr = (m_vertexb_ptr + 1) & 3;
		}
		while (!last_vertex && num_new_verts < 4);

		// for non-textured polygons, polygon color comes after vertex data
		if (!has_texture)
		{
			// polygon Z
			if (!has_vertex_z)
			{
				uint32_t const z = (fifo[index] & 0x07ffff00) | 0x48000000;   // like fog values, these seem to be missing the 4 upper bits of exponent
				polygon_diffuse = fifo[index] & 0xff;
				index++;

				polygon_z = u2f(z);
			}

			// polygon color
			if (!has_vertex_color)
			{
				polygon_color = fifo[index];
				index++;
			}
		}

		// apply constant Z to all verts if needed
		if (!has_vertex_z)
		{
			for (auto j = 0; j < 4; j++)
			{
				m_vertexb[j].p[POLY_Z] = polygon_z;
				m_vertexb[j].p[POLY_DIFF] = polygon_diffuse;
			}
		}

		extra.texture_x = texture_x;
		extra.texture_y = texture_y;
		extra.texture_width = texture_width;
		extra.texture_height = texture_height;
		extra.texture_page = texture_page;
		extra.texture_palette = texture_palette;
		extra.texture_mirror = (cmd & 0x10);
		extra.diffuse_light = rgb_t(m_light_r, m_light_g, m_light_b);
		extra.ambient_light = rgb_t(m_ambient_r, m_ambient_g, m_ambient_b);
		extra.fog_color = rgb_t(m_fog_r, m_fog_g, m_fog_b);
		extra.fog_enable = (m_reg_fog_start != 0xffff) && !(cmd & 1);
		extra.cmd = cmd;

		extra.poly_color = rgb_t((polygon_color >> 24) & 0xff, polygon_color & 0xff, (polygon_color >> 8) & 0xff, (polygon_color >> 16) & 0xff);


		// If 4 new vertices were found, but no last vertex tag - we're reading garbage.
		// Midnrun writes garbage after a 0x80000003 command. The data comes directly from the display list, so it seems intentional.
		if (num_new_verts >= 4 && !last_vertex)
			break;


		// The vertex buffer is a 4-entry circular buffer.
		// Each polygon has at least one new vertex. 0-3 vertices are reused based on how many new vertices were inserted.
		int const v0 = (m_vertexb_ptr - 4) & 3;
		int const v1 = (m_vertexb_ptr - 3) & 3;
		int const v2 = (m_vertexb_ptr - 2) & 3;
		int const v3 = (m_vertexb_ptr - 1) & 3;


		// This fixes shading issues in the Konami logo in Solar Assault.
		// Some triangle strips have different shading values compared to reused vertices, causing unintended smooth shading.
		// This ensures all vertices have the same shading value.
		// Bit 0x20 could be a select between flat shading and gouraud shading.
		if (BIT(~cmd, 5) && num_new_verts < 3)
		{
			int const last_diffuse = m_vertexb[v3].p[POLY_DIFF];
			m_vertexb[v0].p[POLY_DIFF] = last_diffuse;
			m_vertexb[v1].p[POLY_DIFF] = last_diffuse;
			m_vertexb[v2].p[POLY_DIFF] = last_diffuse;
		}

		// No texture, constant color:   Z, Fog, Diffuse
		// Texture, constant color:      Z, Fog, Diffuse, U, V, W
		// Per-vertex color:             Z, Fog, Diffuse, U, V, W, R, G, B, A
		if (is_quad)
		{
			if (has_vertex_color)
			{
				render_triangle<10>(cliprect, has_texture ? rd_scan_vertex_color_tex : rd_scan_vertex_color, m_vertexb[v0], m_vertexb[v1], m_vertexb[v2]);
				render_triangle<10>(cliprect, has_texture ? rd_scan_vertex_color_tex : rd_scan_vertex_color, m_vertexb[v2], m_vertexb[v3], m_vertexb[v0]);
			}
			else if (has_texture)
			{
				render_triangle<6>(cliprect, rd_scan_tex, m_vertexb[v0], m_vertexb[v1], m_vertexb[v2]);
				render_triangle<6>(cliprect, rd_scan_tex, m_vertexb[v2], m_vertexb[v3], m_vertexb[v0]);
			}
			else
			{
				render_triangle<3>(cliprect, rd_scan_color, m_vertexb[v0], m_vertexb[v1], m_vertexb[v2]);
				render_triangle<3>(cliprect, rd_scan_color, m_vertexb[v2], m_vertexb[v3], m_vertexb[v0]);
			}
		}
		else
		{
			if (has_vertex_color)
			{
				render_triangle<10>(cliprect, has_texture ? rd_scan_vertex_color_tex : rd_scan_vertex_color, m_vertexb[v1], m_vertexb[v2], m_vertexb[v3]);
			}
			else if (has_texture)
			{
				render_triangle<6>(cliprect, rd_scan_tex, m_vertexb[v1], m_vertexb[v2], m_vertexb[v3]);
			}
			else
			{
				render_triangle<3>(cliprect, rd_scan_color, m_vertexb[v1], m_vertexb[v2], m_vertexb[v3]);
			}
		}
	}
	return index - start_index;
}


void k001005_renderer::render_polygons()
{
	uint32_t const *const fifo = m_3dfifo.get();
	int index = 0;

	do
	{
		uint32_t const cmd = fifo[index++];

		if (cmd == 0x80000000 || cmd == 0x80000018)
		{
		}
		else if ((cmd & 0xffff0000) == 0x80000000)
		{
			index += parse_polygon(index, cmd);
		}
	}
	while (index < m_3dfifo_ptr);

	m_3dfifo_ptr = 0;
	wait();
}


void k001005_renderer::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int j = cliprect.min_y; j <= cliprect.max_y; j++)
	{
		uint32_t *const bmp = &bitmap.pix(j);
		uint32_t const *const src = &m_fb[m_fb_page ^ 1].pix(j - cliprect.min_y);

		for (int i = cliprect.min_x; i <= cliprect.max_x; i++)
		{
			if (src[i - cliprect.min_x] & 0xff000000)
			{
				bmp[i] = src[i - cliprect.min_x];
			}
		}
	}
}



DEFINE_DEVICE_TYPE(K001005, k001005_device, "k001005", "K001005 Polygon Renderer")

k001005_device::k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K001005, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_k001006(*this, finder_base::DUMMY_TAG)
	, m_fifo(nullptr)
	, m_status(0)
	, m_ram_ptr(0)
	, m_fifo_read_ptr(0)
	, m_fifo_write_ptr(0)
{
	m_ram[0] = nullptr;
	m_ram[1] = nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001005_device::device_start()
{
	m_ram[0] = std::make_unique<uint16_t[]>(0x140000);
	m_ram[1] = std::make_unique<uint16_t[]>(0x140000);

	m_fifo = std::make_unique<uint32_t[]>(0x800);

	m_renderer = std::make_unique<k001005_renderer>(*this, screen(), m_k001006);

	save_pointer(NAME(m_ram[0]), 0x140000);
	save_pointer(NAME(m_ram[1]), 0x140000);
	save_pointer(NAME(m_fifo), 0x800);
	save_item(NAME(m_status));
	save_item(NAME(m_ram_ptr));
	save_item(NAME(m_fifo_read_ptr));
	save_item(NAME(m_fifo_write_ptr));
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

uint32_t k001005_device::read(address_space &space, offs_t offset, uint32_t mem_mask)
{
	adsp21062_device *dsp = downcast<adsp21062_device*>(&space.device());

	switch(offset)
	{
		case 0x000:         // FIFO read, high 16 bits
			//osd_printf_debug("FIFO_r0: %08X\n", m_fifo_read_ptr);
			return m_fifo[m_fifo_read_ptr] >> 16;

		case 0x001:         // FIFO read, low 16 bits
			{
				//osd_printf_debug("FIFO_r1: %08X\n", m_fifo_read_ptr);
				uint16_t const value = m_fifo[m_fifo_read_ptr] & 0xffff;

				if (!machine().side_effects_disabled())
				{
					if (m_status != 1 && m_status != 2)
					{
						if (m_fifo_read_ptr < 0x3ff)
							dsp->set_flag_input(1, CLEAR_LINE);
						else
							dsp->set_flag_input(1, ASSERT_LINE);
					}
					else
					{
						dsp->set_flag_input(1, ASSERT_LINE);
					}

					m_fifo_read_ptr++;
					m_fifo_read_ptr &= 0x7ff;
				}
				return value;
			}

		case 0x11b:         // status ?
			return 0x8002;

		case 0x11c:         // slave status ?
			return 0x8000;

		case 0x11f:
			{
				uint32_t ret = 0;
				if (m_ram_ptr >= 0x400000)
					ret = m_ram[1][m_ram_ptr & 0x3fffff];
				else
					ret = m_ram[0][m_ram_ptr & 0x3fffff];
				if (!machine().side_effects_disabled())
					m_ram_ptr++;
				return ret;
			}

		default:
			//osd_printf_debug("%s m_r: %08X, %08X\n", machine().describe_context(), offset, mem_mask);
			break;
	}
	return 0;
}

void k001005_device::write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	adsp21062_device *dsp = downcast<adsp21062_device*>(&space.device());

	switch (offset)
	{
		case 0x000:         // FIFO write
			//osd_printf_debug("%s K001005 FIFO write: %08X\n", machine().describe_context(), data);
			if (m_status != 1 && m_status != 2)
			{
				if (m_fifo_write_ptr < 0x400)
					dsp->set_flag_input(1, ASSERT_LINE);
				else
					dsp->set_flag_input(1, CLEAR_LINE);
			}
			else
			{
				dsp->set_flag_input(1, ASSERT_LINE);
			}

			//osd_printf_debug("%s K001005 FIFO write: %08X\n", machine().describe_context(), data);
			m_fifo[m_fifo_write_ptr] = data;
			m_fifo_write_ptr++;
			m_fifo_write_ptr &= 0x7ff;

			m_renderer->push_data(data);

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (dsp->pc() == 0x201ee)
			{
				// This is used to make the SHARC timeout
				dsp->spin_until_trigger(10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (dsp->pc() == 0x201e6)
			{
				// This is used to make the SHARC timeout
				dsp->spin_until_trigger(10000);
			}
			break;

		case 0x100:     break;

		case 0x101:     break;      // framebuffer width?
		case 0x102:     break;      // framebuffer height?

		case 0x103:     m_renderer->m_viewport_min_x = data & 0xffff; break;
		case 0x104:     m_renderer->m_viewport_max_x = data & 0xffff; break;
		case 0x105:     m_renderer->m_viewport_max_y = data & 0xffff; break;
		case 0x106:     m_renderer->m_viewport_min_y = data & 0xffff; break;

		case 0x107:     m_renderer->m_viewport_center_x = data & 0xffff; break;
		case 0x108:     m_renderer->m_viewport_center_y = data & 0xffff; break;

		case 0x109:                 // far Z value
			// the SHARC code throws away the bottom 11 bits of mantissa and the top 5 bits (to fit in a 16-bit register?)
			m_renderer->m_far_z = u2f((data & 0xffff) << 11);
			break;

		case 0x10a:     m_renderer->m_light_r = data & 0xff; break;
		case 0x10b:     m_renderer->m_light_g = data & 0xff; break;
		case 0x10c:     m_renderer->m_light_b = data & 0xff; break;

		case 0x10d:     m_renderer->m_ambient_r = data & 0xff; break;
		case 0x10e:     m_renderer->m_ambient_g = data & 0xff; break;
		case 0x10f:     m_renderer->m_ambient_b = data & 0xff; break;

		case 0x110:     m_renderer->m_fog_r = data & 0xff; break;
		case 0x111:     m_renderer->m_fog_g = data & 0xff; break;
		case 0x112:     m_renderer->m_fog_b = data & 0xff; break;

		case 0x117:                 // linear fog start Z
			// 4 bits exponent + 12 bits mantissa, similar to far Z value
			// value of 0xffff is used to effectively turn off fog

			// reconstruct float from 16-bit data
			// assuming implicit exponent 1001xxxx, sign bit 0 (z-values are all positive)
			m_renderer->m_reg_fog_start = data & 0xffff;
			m_renderer->m_fog_start_z = u2f((0x90000 | (data & 0xffff)) << 11);
			break;

		case 0x118:                 // linear fog end Z
			// 4 bits exponent + 12 bits mantissa, similar to far Z value
			m_renderer->m_fog_end_z = u2f((0x90000 | (data & 0xffff)) << 11);
			break;

		case 0x119:                 // 1 / (end_fog - start_fog) ?
			// 5 bits exponent + 11 bits mantissa
			break;


		case 0x11a:
			m_status = data;
			m_fifo_write_ptr = 0;
			m_fifo_read_ptr = 0;

			if (data == 2)
			{
				if (m_renderer->fifo_filled())
					m_renderer->render_polygons();

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
				m_ram[1][(m_ram_ptr++) & 0x3fffff] = data & 0xffff;
			else
				m_ram[0][(m_ram_ptr++) & 0x3fffff] = data & 0xffff;
			break;

		default:
			//osd_printf_debug("%s m_w: %08X, %08X, %08X\n", machine().describe_context(), data, offset, mem_mask);
			break;
	}

}

void k001005_device::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_renderer->draw(bitmap, cliprect);
}
