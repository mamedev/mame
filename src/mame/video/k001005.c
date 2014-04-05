/* This is currently unused, video/gticlub.c has it's own implementation, why? */

#include "emu.h"
#include "k001005.h"


/*****************************************************************************/
/* Konami K001005 Custom 3D Pixel Renderer chip (KS10071) */

/***************************************************************************/
/*                                                                         */
/*                                  001005                                 */
/*                                                                         */
/***************************************************************************/


struct poly_extra_data
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
};


const device_type K001005 = &device_creator<k001005_device>;

k001005_device::k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001005, "Konami 001005", tag, owner, clock, "k001005", __FILE__),
		device_video_interface(mconfig, *this),
		m_cpu(NULL),
		m_dsp(NULL),
		m_k001006_1(NULL),
		m_k001006_2(NULL),
		m_texture(NULL),
		m_fifo(NULL),
		m_3d_fifo(NULL),
		m_status(0),
		m_zbuffer(NULL),
		m_ram_ptr(0),
		m_fifo_read_ptr(0),
		m_fifo_write_ptr(0),
		m_3d_fifo_ptr(0),
		m_bitmap_page(0),
		m_poly(NULL),
		m_prev_poly_type(0),
		m_gfxrom(NULL),
		m_palette(*this)
{
		m_ram[0] = 0;
		m_ram[1] = 0;
		m_bitmap[0] = NULL;
		m_bitmap[1] = NULL;

		for (int i = 0; i < 128; i++)
		{
			m_tex_mirror_table[0][i] = 0;
			m_tex_mirror_table[1][i] = 0;
			m_tex_mirror_table[2][i] = 0;
			m_tex_mirror_table[3][i] = 0;
		}
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001005_device::device_config_complete()
{
	// inherit a copy of the static data
	const k001005_interface *intf = reinterpret_cast<const k001005_interface *>(static_config());
	if (intf != NULL)
	*static_cast<k001005_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_cpu_tag = "";
		m_dsp_tag = "";
		m_k001006_1_tag = "";
		m_k001006_2_tag = "";

		m_gfx_memory_region_tag = "";
		m_gfx_index = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001005_device::device_start()
{
	int i, width, height;

	m_cpu = machine().device(m_cpu_tag);
	m_dsp = machine().device<adsp21062_device>(m_dsp_tag);
	m_k001006_1 = machine().device(m_k001006_1_tag);
	m_k001006_2 = machine().device(m_k001006_2_tag);

	width = m_screen->width();
	height = m_screen->height();
	m_zbuffer = auto_bitmap_ind32_alloc(machine(), width, height);

	m_gfxrom = machine().root_device().memregion(m_gfx_memory_region_tag)->base();

	m_bitmap[0] = auto_bitmap_rgb32_alloc(machine(), m_screen->width(), m_screen->height());
	m_bitmap[1] = auto_bitmap_rgb32_alloc(machine(), m_screen->width(), m_screen->height());

	m_texture = auto_alloc_array(machine(), UINT8, 0x800000);

	m_ram[0] = auto_alloc_array(machine(), UINT16, 0x140000);
	m_ram[1] = auto_alloc_array(machine(), UINT16, 0x140000);

	m_fifo = auto_alloc_array(machine(), UINT32, 0x800);

	m_3d_fifo = auto_alloc_array(machine(), UINT32, 0x10000);

	m_poly = poly_alloc(machine(), 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	for (i = 0; i < 128; i++)
	{
		m_tex_mirror_table[0][i] = i & 0x3f;
		m_tex_mirror_table[1][i] = i & 0x3f;
		m_tex_mirror_table[2][i] = ((i & 0x3f) >= 0x20) ? (0x1f - (i & 0x1f)) : i & 0x1f;
		m_tex_mirror_table[3][i] = ((i & 0x7f) >= 0x40) ? (0x3f - (i & 0x3f)) : i & 0x3f;
	}


	save_pointer(NAME(m_texture), 0x800000);
	save_pointer(NAME(m_ram[0]), 0x140000);
	save_pointer(NAME(m_ram[1]), 0x140000);
	save_pointer(NAME(m_fifo), 0x800);
	save_pointer(NAME(m_3d_fifo), 0x10000);
	save_item(NAME(m_status));
	save_item(NAME(m_ram_ptr));
	save_item(NAME(m_fifo_read_ptr));
	save_item(NAME(m_fifo_write_ptr));
	save_item(NAME(m_3d_fifo_ptr));
	save_item(NAME(m_bitmap_page));
	save_item(NAME(m_prev_poly_type));
	save_item(NAME(*m_bitmap[0]));
	save_item(NAME(*m_bitmap[1]));
	save_item(NAME(*m_zbuffer));

	// FIXME: shall we save poly as well?
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
	m_3d_fifo_ptr = 0;
	m_bitmap_page = 0;

	memset(m_prev_v, 0, sizeof(m_prev_v));
	m_prev_poly_type = 0;
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void k001005_device::device_stop()
{
	poly_free(m_poly);
}


static const int decode_x_gti[8] = {  0, 16, 2, 18, 4, 20, 6, 22 };
static const int decode_y_gti[16] = {  0, 8, 32, 40, 1, 9, 33, 41, 64, 72, 96, 104, 65, 73, 97, 105 };

static const int decode_x_zr107[8] = {  0, 16, 1, 17, 2, 18, 3, 19 };
static const int decode_y_zr107[16] = {  0, 8, 32, 40, 4, 12, 36, 44, 64, 72, 96, 104, 68, 76, 100, 108 };


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

// rearranges the texture data to a more practical order
void k001005_device::preprocess_texture_data( UINT8 *rom, int length, int gticlub )
{
	int index;
	int i, x, y;
	UINT8 temp[0x40000];

	const int *decode_x;
	const int *decode_y;

	if (gticlub)
	{
		decode_x = decode_x_gti;
		decode_y = decode_y_gti;
	}
	else
	{
		decode_x = decode_x_zr107;
		decode_y = decode_y_zr107;
	}

	for (index = 0; index < length; index += 0x40000)
	{
		int offset = index;

		memset(temp, 0, 0x40000);

		for (i = 0; i < 0x800; i++)
		{
			int tx = ((i & 0x400) >> 5) | ((i & 0x100) >> 4) | ((i & 0x40) >> 3) | ((i & 0x10) >> 2) | ((i & 0x4) >> 1) | (i & 0x1);
			int ty = ((i & 0x200) >> 5) | ((i & 0x80) >> 4) | ((i & 0x20) >> 3) | ((i & 0x8) >> 2) | ((i & 0x2) >> 1);

			tx <<= 3;
			ty <<= 4;

			for (y = 0; y < 16; y++)
			{
				for (x = 0; x < 8; x++)
				{
					UINT8 pixel = rom[offset + decode_y[y] + decode_x[x]];

					temp[((ty + y) * 512) + (tx + x)] = pixel;
				}
			}

			offset += 128;
		}

		memcpy(&rom[index], temp, 0x40000);
	}
}

void k001005_device::swap_buffers( )
{
	m_bitmap_page ^= 1;

	//if (m_status == 2)
	{
		m_bitmap[m_bitmap_page]->fill(m_palette->pen(0) & 0x00ffffff, m_cliprect);
		m_zbuffer->fill(0xffffffff, m_cliprect);
	}
}

READ32_MEMBER( k001005_device::read )
{
	switch(offset)
	{
		case 0x000:         // FIFO read, high 16 bits
		{
			UINT16 value = m_fifo[m_fifo_read_ptr] >> 16;
		//  mame_printf_debug("FIFO_r0: %08X\n", m_fifo_ptr);
			return value;
		}

		case 0x001:         // FIFO read, low 16 bits
		{
			UINT16 value = m_fifo[m_fifo_read_ptr] & 0xffff;
		//  mame_printf_debug("FIFO_r1: %08X\n", m_fifo_ptr);

			if (m_status != 1 && m_status != 2)
			{
				if (m_fifo_read_ptr < 0x3ff)
				{
					//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					m_dsp->set_flag_input(1, CLEAR_LINE);
				}
				else
				{
					//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					m_dsp->set_flag_input(1, ASSERT_LINE);
				}
			}
			else
			{
				//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				m_dsp->set_flag_input(1, ASSERT_LINE);
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
			//mame_printf_debug("m_r: %08X, %08X at %08X\n", offset, mem_mask, space.device().safe_pc());
			break;
	}
	return 0;
}

WRITE32_MEMBER( k001005_device::write )
{
	switch (offset)
	{
		case 0x000:         // FIFO write
		{
			if (m_status != 1 && m_status != 2)
			{
				if (m_fifo_write_ptr < 0x400)
				{
					//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					m_dsp->set_flag_input(1, ASSERT_LINE);
				}
				else
				{
					//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					m_dsp->set_flag_input(1, CLEAR_LINE);
				}
			}
			else
			{
				//m_dsp->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				m_dsp->set_flag_input(1, ASSERT_LINE);
			}

		//  mame_printf_debug("K001005 FIFO write: %08X at %08X\n", data, space.device().safe_pc());
			m_fifo[m_fifo_write_ptr] = data;
			m_fifo_write_ptr++;
			m_fifo_write_ptr &= 0x7ff;

			m_3d_fifo[m_3d_fifo_ptr++] = data;

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (m_cpu->safe_pc() == 0x201ee)
			{
				// This is used to make the SHARC timeout
				m_cpu->execute().spin_until_trigger(10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (m_cpu->safe_pc() == 0x201e6)
			{
				// This is used to make the SHARC timeout
				m_cpu->execute().spin_until_trigger(10000);
			}

			break;
		}

		case 0x100: break;

	//  case 0x10a:     poly_r = data & 0xff; break;
	//  case 0x10b:     poly_g = data & 0xff; break;
	//  case 0x10c:     poly_b = data & 0xff; break;

		case 0x11a:
			m_status = data;
			m_fifo_write_ptr = 0;
			m_fifo_read_ptr = 0;

			if (data == 2 && m_3d_fifo_ptr > 0)
			{
				swap_buffers();
				render_polygons();
				poly_wait(m_poly, "render_polygons");
				m_3d_fifo_ptr = 0;
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
			//mame_printf_debug("m_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space.device().safe_pc());
			break;
	}

}

/* legacy_poly_manager cannot handle atm callbacks passing a device parameter */

#if POLY_DEVICE
void k001005_device::draw_scanline( void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	float z = extent->param[0].start;
	float dz = extent->param[0].dpdx;
	UINT32 *fb = &destmap->pix32(scanline);
	UINT32 *zb = &m_zbuffer->pix32(scanline);
	UINT32 color = extra->color;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;

		if (iz <= zb[x])
		{
			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		z += dz;
	}
}
#endif

#if POLY_DEVICE
void k001005_device::draw_scanline_tex( device_t *device, void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid )
{
	k001005_state *k001005 = k001005_get_safe_token(device);
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT8 *texrom = m_gfxrom + (extra->texture_page * 0x40000);
	device_t *pal_device = (extra->texture_palette & 0x8) ? m_k001006_2 : m_k001006_1;
	int palette_index = (extra->texture_palette & 0x7) * 256;
	float z = extent->param[0].start;
	float u = extent->param[1].start;
	float v = extent->param[2].start;
	float w = extent->param[3].start;
	float dz = extent->param[0].dpdx;
	float du = extent->param[1].dpdx;
	float dv = extent->param[2].dpdx;
	float dw = extent->param[3].dpdx;
	int texture_mirror_x = extra->texture_mirror_x;
	int texture_mirror_y = extra->texture_mirror_y;
	int texture_x = extra->texture_x;
	int texture_y = extra->texture_y;
	int x;

	UINT32 *fb = &destmap->pix32(scanline);
	UINT32 *zb = &m_zbuffer->pix32(scanline);

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = (UINT32)z >> 16;
		//int iu = u >> 16;
		//int iv = v >> 16;

		if (iz < zb[x])
		{
			float oow = 1.0f / w;
			UINT32 color;
			int iu, iv;
			int iiv, iiu, texel;

			iu = u * oow;
			iv = v * oow;

			iiu = texture_x + m_tex_mirror_table[texture_mirror_x][(iu >> 4) & 0x7f];
			iiv = texture_y + m_tex_mirror_table[texture_mirror_y][(iv >> 4) & 0x7f];
			texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
			color = k001006_get_palette(pal_device, palette_index + texel);

			if (color & 0xff000000)
			{
				fb[x] = color;
				zb[x] = iz;
			}
		}

		u += du;
		v += dv;
		z += dz;
		w += dw;
	}
}
#endif


void k001005_device::render_polygons( )
{
	int i, j;
#if POLY_DEVICE
	const rectangle &visarea = visible_area();
#endif

//  mame_printf_debug("m_fifo_ptr = %08X\n", m_3d_fifo_ptr);

	for (i = 0; i < m_3d_fifo_ptr; i++)
	{
		if (m_3d_fifo[i] == 0x80000003)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(m_poly);
//          poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int index = i;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y;

				x = ((m_3d_fifo[index] >>  0) & 0x3fff);
				y = ((m_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);
				++index;
#if POLY_DEVICE
				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = 0;  /* ??? */
#endif
			}

			++index;

			r = (m_3d_fifo[index] >>  0) & 0xff;
			g = (m_3d_fifo[index] >>  8) & 0xff;
			b = (m_3d_fifo[index] >> 16) & 0xff;
			a = (m_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			++index;

			extra->color = color;
#if POLY_DEVICE
			poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//          poly_render_polygon(m_poly, m_bitmap[m_bitmap_page],  &visarea, draw_scanline, 1, 4, v);
#endif
			i = index - 1;
		}
		else if (m_3d_fifo[i] == 0x800000ae || m_3d_fifo[i] == 0x8000008e ||
					m_3d_fifo[i] == 0x80000096 || m_3d_fifo[i] == 0x800000b6 ||
					m_3d_fifo[i] == 0x8000002e || m_3d_fifo[i] == 0x8000000e ||
					m_3d_fifo[i] == 0x80000016 || m_3d_fifo[i] == 0x80000036 ||
					m_3d_fifo[i] == 0x800000aa || m_3d_fifo[i] == 0x800000a8 ||
					m_3d_fifo[i] == 0x800000b2)
		{
			// 0x00: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx    Command
			//
			// 0x01: xxxx---- -------- -------- --------    Texture palette
			// 0x01: -------- -------x xxxx---- --------    Texture page
			// 0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
			// 0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(m_poly);
			poly_vertex v[4];
			int tx, ty;
			UINT32 color = 0;
			UINT32 header;
			UINT32 command;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			command = m_3d_fifo[index++];
			header = m_3d_fifo[index++];

			for (j = 0; j < 4; j++)
			{
				INT16 u2, v2;
				int x, y, z;
				int end = 0;

				x = ((m_3d_fifo[index] >>  0) & 0x3fff);
				y = ((m_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = m_3d_fifo[index] & 0x4000;
				end = m_3d_fifo[index] & 0x8000;
				++index;

				z = m_3d_fifo[index];
				++index;

				if (end)
				{
					color = m_3d_fifo[index];
					++index;

					u2 = (m_3d_fifo[index] >> 16) & 0xffff;
					v2 = (m_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}
				else
				{
					u2 = (m_3d_fifo[index] >> 16) & 0xffff;
					v2 = (m_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);
				v[j].p[3] = 1.0f / v[j].p[0];
				v[j].p[1] = u2 * v[j].p[3];
				v[j].p[2] = v2 * v[j].p[3];

				++num_verts;

				if (end)
					break;
			}

			ty = ((header & 0x400) >> 5) |
					((header & 0x100) >> 4) |
					((header & 0x040) >> 3) |
					((header & 0x010) >> 2) |
					((header & 0x004) >> 1) |
					((header & 0x001) >> 0);

			tx = ((header & 0x800) >> 6) |
					((header & 0x200) >> 5) |
					((header & 0x080) >> 4) |
					((header & 0x020) >> 3) |
					((header & 0x008) >> 2) |
					((header & 0x002) >> 1);

			extra->texture_x = tx * 8;
			extra->texture_y = ty * 8;

			extra->texture_page = (header >> 12) & 0x1f;
			extra->texture_palette = (header >> 28) & 0xf;

			extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
			extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

			extra->color = color;

			if (num_verts < 3)
			{
#if POLY_DEVICE
				poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &m_prev_v[2], &v[0], &v[1]);
				if (m_prev_poly_type)
					poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &m_prev_v[2], &m_prev_v[3], &v[0]);
//              if (m_prev_poly_type)
//                  poly_render_quad(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &m_prev_v[2], &m_prev_v[3], &v[0], &v[1]);
//              else
//                  poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &m_prev_v[2], &v[0], &v[1]);
#endif
				memcpy(&m_prev_v[0], &m_prev_v[2], sizeof(poly_vertex));
				memcpy(&m_prev_v[1], &m_prev_v[3], sizeof(poly_vertex));
				memcpy(&m_prev_v[2], &v[0], sizeof(poly_vertex));
				memcpy(&m_prev_v[3], &v[1], sizeof(poly_vertex));
			}
			else
			{
#if POLY_DEVICE
				poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (num_verts > 3)
					poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, num_verts, v);
#endif
				memcpy(m_prev_v, v, sizeof(poly_vertex) * 4);
			}

			m_prev_poly_type = poly_type;

			while ((m_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < m_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(m_poly);
#if POLY_DEVICE
				int new_verts = 0;
#endif
				if (poly_type)
				{
					memcpy(&v[0], &m_prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &m_prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &m_prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &m_prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					INT16 u2, v2;
					int x, y, z;
					int end = 0;

					x = ((m_3d_fifo[index] >>  0) & 0x3fff);
					y = ((m_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = m_3d_fifo[index] & 0x4000;
					end = m_3d_fifo[index] & 0x8000;
					++index;

					z = m_3d_fifo[index];
					++index;

					if (end)
					{
						color = m_3d_fifo[index];
						++index;

						u2 = (m_3d_fifo[index] >> 16) & 0xffff;
						v2 = (m_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}
					else
					{
						u2 = (m_3d_fifo[index] >> 16) & 0xffff;
						v2 = (m_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);
					v[j].p[3] = 1.0f / v[j].p[0];
					v[j].p[1] = u2 * v[j].p[3];
					v[j].p[2] = v2 * v[j].p[3];

#if POLY_DEVICE
					++new_verts;
#endif

					if (end)
						break;
				}

				extra->texture_x = tx * 8;
				extra->texture_y = ty * 8;

				extra->texture_page = (header >> 12) & 0x1f;
				extra->texture_palette = (header >> 28) & 0xf;

				extra->texture_mirror_x = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);
				extra->texture_mirror_y = ((command & 0x10) ? 0x2 : 0) | ((header & 0x00400000) ? 0x1 : 0);

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline_tex, 4, new_verts + 2, v);
#endif
				memcpy(m_prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (m_3d_fifo[i] == 0x80000006 || m_3d_fifo[i] == 0x80000026 ||
					m_3d_fifo[i] == 0x80000020 || m_3d_fifo[i] == 0x80000022)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(m_poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			++index;

			for (j = 0; j < 4; j++)
			{
				int x, y, z;
				int end = 0;

				x = ((m_3d_fifo[index] >>  0) & 0x3fff);
				y = ((m_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = m_3d_fifo[index] & 0x4000;
				end = m_3d_fifo[index] & 0x8000;
				++index;

				z = m_3d_fifo[index];
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);

				++num_verts;

				if (end)
					break;
			}

			r = (m_3d_fifo[index] >>  0) & 0xff;
			g = (m_3d_fifo[index] >>  8) & 0xff;
			b = (m_3d_fifo[index] >> 16) & 0xff;
			a = (m_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra->color = color;

#if POLY_DEVICE
			poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			if (num_verts > 3)
				poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[2], &v[3], &v[0]);
//          poly_render_polygon(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, num_verts, v);
#endif
			memcpy(m_prev_v, v, sizeof(poly_vertex) * 4);

			while ((m_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < m_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(m_poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &m_prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &m_prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &m_prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &m_prev_v[2], sizeof(poly_vertex));
				}

				for (j = 2; j < 4; j++)
				{
					int x, y, z;
					int end = 0;

					x = ((m_3d_fifo[index] >>  0) & 0x3fff);
					y = ((m_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = m_3d_fifo[index] & 0x4000;
					end = m_3d_fifo[index] & 0x8000;
					++index;

					z = m_3d_fifo[index];
					++index;

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);

					++new_verts;

					if (end)
						break;
				}

				r = (m_3d_fifo[index] >>  0) & 0xff;
				g = (m_3d_fifo[index] >>  8) & 0xff;
				b = (m_3d_fifo[index] >> 16) & 0xff;
				a = (m_3d_fifo[index] >> 24) & 0xff;
				color = (a << 24) | (r << 16) | (g << 8) | (b);
				index++;

				extra->color = color;

#if POLY_DEVICE
				poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//              poly_render_polygon(m_poly, m_bitmap[m_bitmap_page], &visarea, draw_scanline, 1, new_verts + 2, v);
#endif
				memcpy(m_prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (m_3d_fifo[i] == 0x80000000)
		{
		}
		else if ((m_3d_fifo[i] & 0xffffff00) == 0x80000000)
		{
			/*
			mame_printf_debug("Unknown polygon type %08X:\n", m_3d_fifo[i]);
			for (j = 0; j < 0x20; j++)
			{
			    mame_printf_debug("  %02X: %08X\n", j, m_3d_fifo[i + 1 + j]);
			}
			mame_printf_debug("\n");
			*/
		}
	}
}

void k001005_device::draw( bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int i, j;

	memcpy(&m_cliprect, &cliprect, sizeof(rectangle));

	for (j = cliprect.min_y; j <= cliprect.max_y; j++)
	{
		UINT32 *bmp = &bitmap.pix32(j);
		UINT32 *src = &m_bitmap[m_bitmap_page ^ 1]->pix32(j);

		for (i = cliprect.min_x; i <= cliprect.max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void k001005_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<k001005_device &>(device).m_palette.set_tag(tag);
}
