#include "emu.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "video/poly.h"
#include "video/konicdev.h"
#include "video/gticlub.h"


typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
};

extern UINT8 gticlub_led_reg0;
extern UINT8 gticlub_led_reg1;


/*****************************************************************************/
/* Konami K001006 Custom 3D Texel Renderer chip (KS10081) */

#define MAX_K001006_CHIPS		2

static UINT16 *K001006_pal_ram[MAX_K001006_CHIPS];
static UINT16 *K001006_unknown_ram[MAX_K001006_CHIPS];
static UINT32 K001006_addr[MAX_K001006_CHIPS] = { 0, 0 };
static int K001006_device_sel[MAX_K001006_CHIPS] = { 0, 0 };

static UINT32 *K001006_palette[MAX_K001006_CHIPS];

void K001006_init(running_machine *machine)
{
	int i;
	for (i=0; i<MAX_K001006_CHIPS; i++)
	{
		K001006_pal_ram[i] = auto_alloc_array_clear(machine, UINT16, 0x800);
		K001006_unknown_ram[i] = auto_alloc_array_clear(machine, UINT16, 0x1000);
		K001006_addr[i] = 0;
		K001006_device_sel[i] = 0;
		K001006_palette[i] = auto_alloc_array(machine, UINT32, 0x800);
		memset(K001006_palette[i], 0, 0x800*sizeof(UINT32));
	}
}

static UINT32 K001006_r(running_machine *machine, int chip, int offset, UINT32 mem_mask)
{
	if (offset == 1)
	{
		switch (K001006_device_sel[chip])
		{
			case 0x0b:		// CG Board ROM read
			{
				UINT16 *rom = (UINT16*)memory_region(machine, "gfx1");
				return rom[K001006_addr[chip] / 2] << 16;
			}
			case 0x0d:		// Palette RAM read
			{
				UINT32 addr = K001006_addr[chip];

				K001006_addr[chip] += 2;
				return K001006_pal_ram[chip][addr>>1];
			}
			case 0x0f:		// Unknown RAM read
			{
				return K001006_unknown_ram[chip][K001006_addr[chip]++];
			}
			default:
			{
				fatalerror("K001006_r chip %d, unknown device %02X", chip, K001006_device_sel[chip]);
			}
		}
	}
	return 0;
}

static void K001006_w(int chip, int offset, UINT32 data, UINT32 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&K001006_addr[chip]);
	}
	else if (offset == 1)
	{
		switch (K001006_device_sel[chip])
		{
			case 0xd:	// Palette RAM write
			{
				int r, g, b, a;
				UINT32 index = K001006_addr[chip];

				K001006_pal_ram[chip][index>>1] = data & 0xffff;

				a = (data & 0x8000) ? 0x00 : 0xff;
				b = ((data >> 10) & 0x1f) << 3;
				g = ((data >>  5) & 0x1f) << 3;
				r = ((data >>  0) & 0x1f) << 3;
				b |= (b >> 5);
				g |= (g >> 5);
				r |= (r >> 5);
				K001006_palette[chip][index>>1] = MAKE_ARGB(a, r, g, b);

				K001006_addr[chip] += 2;
				break;
			}
			case 0xf:	// Unknown RAM write
			{
			//  mame_printf_debug("Unknown RAM %08X = %04X\n", K001006_addr[chip], data & 0xffff);
				K001006_unknown_ram[chip][K001006_addr[chip]++] = data & 0xffff;
				break;
			}
			default:
			{
				mame_printf_debug("K001006_w: chip %d, device %02X, write %04X to %08X\n", chip, K001006_device_sel[chip], data & 0xffff, K001006_addr[0]++);
			}
		}
	}
	else if (offset == 2)
	{
		if (ACCESSING_BITS_16_31)
		{
			K001006_device_sel[chip] = (data >> 16) & 0xf;
		}
	}
}

READ32_HANDLER(K001006_0_r)
{
	return K001006_r(space->machine, 0, offset, mem_mask);
}

WRITE32_HANDLER(K001006_0_w)
{
	K001006_w(0, offset, data, mem_mask);
}

READ32_HANDLER(K001006_1_r)
{
	return K001006_r(space->machine, 1, offset, mem_mask);
}

WRITE32_HANDLER(K001006_1_w)
{
	K001006_w(1, offset, data, mem_mask);
}


/*****************************************************************************/
/* Konami K001005 Custom 3D Pixel Renderer chip (KS10071) */

static const int decode_x_gti[8] = {  0, 16, 2, 18, 4, 20, 6, 22 };
static const int decode_y_gti[16] = {  0, 8, 32, 40, 1, 9, 33, 41, 64, 72, 96, 104, 65, 73, 97, 105 };

static const int decode_x_zr107[8] = {  0, 16, 1, 17, 2, 18, 3, 19 };
static const int decode_y_zr107[16] = {  0, 8, 32, 40, 4, 12, 36, 44, 64, 72, 96, 104, 68, 76, 100, 108 };

static UINT32 K001005_status = 0;
static bitmap_t *K001005_bitmap[2];
static bitmap_t *K001005_zbuffer;
static rectangle K001005_cliprect;

static void render_polygons(running_machine *machine);

static UINT8 *K001005_texture;

static UINT16 *K001005_ram[2];
static int K001005_ram_ptr = 0;
static UINT32 *K001005_fifo;
static int K001005_fifo_read_ptr = 0;
static int K001005_fifo_write_ptr = 0;

static UINT32 *K001005_3d_fifo;
static int K001005_3d_fifo_ptr = 0;

static int tex_mirror_table[4][128];

static int K001005_bitmap_page = 0;

static poly_manager *poly;
static poly_vertex prev_v[4];
static int prev_poly_type;

static UINT8 *gfxrom;


void K001005_swap_buffers(running_machine *machine);

static void K001005_exit(running_machine &machine)
{
	poly_free(poly);
}

void K001005_init(running_machine *machine)
{
	int i;

	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();
	K001005_zbuffer = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);

	gfxrom = memory_region(machine, "gfx1");

	K001005_bitmap[0] = machine->primary_screen->alloc_compatible_bitmap();
	K001005_bitmap[1] = machine->primary_screen->alloc_compatible_bitmap();

	K001005_texture = auto_alloc_array(machine, UINT8, 0x800000);

	K001005_ram[0] = auto_alloc_array(machine, UINT16, 0x140000);
	K001005_ram[1] = auto_alloc_array(machine, UINT16, 0x140000);

	K001005_fifo = auto_alloc_array(machine, UINT32, 0x800);

	K001005_3d_fifo = auto_alloc_array(machine, UINT32, 0x10000);

	poly = poly_alloc(machine, 4000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, K001005_exit);

	for (i=0; i < 128; i++)
	{
		tex_mirror_table[0][i] = i & 0x3f;
		tex_mirror_table[1][i] = i & 0x3f;
		tex_mirror_table[2][i] = ((i & 0x3f) >= 0x20) ? (0x1f - (i & 0x1f)) : i & 0x1f;
		tex_mirror_table[3][i] = ((i & 0x7f) >= 0x40) ? (0x3f - (i & 0x3f)) : i & 0x3f;
	}

	K001005_status = 0;
	K001005_ram_ptr = 0;
	K001005_fifo_read_ptr = 0;
	K001005_fifo_write_ptr = 0;
	K001005_3d_fifo_ptr = 0;
	K001005_bitmap_page = 0;

	memset(prev_v, 0, sizeof(prev_v));
	prev_poly_type = 0;
}

// rearranges the texture data to a more practical order
void K001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub)
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

	for (index=0; index < length; index += 0x40000)
	{
		int offset = index;

		memset(temp, 0, 0x40000);

		for (i=0; i < 0x800; i++)
		{
			int tx = ((i & 0x400) >> 5) | ((i & 0x100) >> 4) | ((i & 0x40) >> 3) | ((i & 0x10) >> 2) | ((i & 0x4) >> 1) | (i & 0x1);
			int ty = ((i & 0x200) >> 5) | ((i & 0x80) >> 4) | ((i & 0x20) >> 3) | ((i & 0x8) >> 2) | ((i & 0x2) >> 1);

			tx <<= 3;
			ty <<= 4;

			for (y=0; y < 16; y++)
			{
				for (x=0; x < 8; x++)
				{
					UINT8 pixel = rom[offset + decode_y[y] + decode_x[x]];

					temp[((ty+y) * 512) + (tx+x)] = pixel;
				}
			}

			offset += 128;
		}

		memcpy(&rom[index], temp, 0x40000);
	}
}

READ32_HANDLER( K001005_r )
{
	switch(offset)
	{
		case 0x000:			// FIFO read, high 16 bits
		{
			UINT16 value = K001005_fifo[K001005_fifo_read_ptr] >> 16;
		//  mame_printf_debug("FIFO_r0: %08X\n", K001005_fifo_ptr);
			return value;
		}

		case 0x001:			// FIFO read, low 16 bits
		{
			UINT16 value = K001005_fifo[K001005_fifo_read_ptr] & 0xffff;
		//  mame_printf_debug("FIFO_r1: %08X\n", K001005_fifo_ptr);

			if (K001005_status != 1 && K001005_status != 2)
			{
				if (K001005_fifo_read_ptr < 0x3ff)
				{
					//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, CLEAR_LINE);
				}
				else
				{
					//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, ASSERT_LINE);
				}
			}
			else
			{
				//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, ASSERT_LINE);
			}

			K001005_fifo_read_ptr++;
			K001005_fifo_read_ptr &= 0x7ff;
			return value;
		}

		case 0x11b:			// status ?
			return 0x8002;

		case 0x11c:			// slave status ?
			return 0x8000;

		case 0x11f:
			if (K001005_ram_ptr >= 0x400000)
			{
				return K001005_ram[1][(K001005_ram_ptr++) & 0x3fffff];
			}
			else
			{
				return K001005_ram[0][(K001005_ram_ptr++) & 0x3fffff];
			}

		default:
			mame_printf_debug("K001005_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
	return 0;
}

WRITE32_HANDLER( K001005_w )
{
	switch(offset)
	{
		case 0x000:			// FIFO write
		{
			if (K001005_status != 1 && K001005_status != 2)
			{
				if (K001005_fifo_write_ptr < 0x400)
				{
					//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, ASSERT_LINE);
				}
				else
				{
					//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, CLEAR_LINE);
				}
			}
			else
			{
				//cputag_set_input_line(space->machine, "dsp", SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(devtag_get_device(space->machine, "dsp"), 1, ASSERT_LINE);
			}

	    //  mame_printf_debug("K001005 FIFO write: %08X at %08X\n", data, cpu_get_pc(space->cpu));
			K001005_fifo[K001005_fifo_write_ptr] = data;
			K001005_fifo_write_ptr++;
			K001005_fifo_write_ptr &= 0x7ff;

			K001005_3d_fifo[K001005_3d_fifo_ptr++] = data;

			// !!! HACK to get past the FIFO B test (GTI Club & Thunder Hurricane) !!!
			if (cpu_get_pc(space->cpu) == 0x201ee)
			{
				// This is used to make the SHARC timeout
				cpu_spinuntil_trigger(space->cpu, 10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (cpu_get_pc(space->cpu) == 0x201e6)
			{
				// This is used to make the SHARC timeout
				cpu_spinuntil_trigger(space->cpu, 10000);
			}

			break;
		}

		case 0x100: break;

	//  case 0x10a:     poly_r = data & 0xff; break;
	//  case 0x10b:     poly_g = data & 0xff; break;
	//  case 0x10c:     poly_b = data & 0xff; break;

		case 0x11a:
			K001005_status = data;
			K001005_fifo_write_ptr = 0;
			K001005_fifo_read_ptr = 0;

			if (data == 2 && K001005_3d_fifo_ptr > 0)
			{
				K001005_swap_buffers(space->machine);
				render_polygons(space->machine);
				poly_wait(poly, "render_polygons");
				K001005_3d_fifo_ptr = 0;
			}
			break;

		case 0x11d:
			K001005_fifo_write_ptr = 0;
			K001005_fifo_read_ptr = 0;
			break;

		case 0x11e:
			K001005_ram_ptr = data;
			break;

		case 0x11f:
			if (K001005_ram_ptr >= 0x400000)
			{
				K001005_ram[1][(K001005_ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			else
			{
				K001005_ram[0][(K001005_ram_ptr++) & 0x3fffff] = data & 0xffff;
			}
			break;

		default:
			//mame_printf_debug("K001005_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}

}

static void draw_scanline(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	float z = extent->param[0].start;
	float dz = extent->param[0].dpdx;
	UINT32 *fb = BITMAP_ADDR32(destmap, scanline, 0);
	UINT32 *zb = BITMAP_ADDR32(K001005_zbuffer, scanline, 0);
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

static void draw_scanline_tex(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	UINT8 *texrom = gfxrom + (extra->texture_page * 0x40000);
	int pal_chip = (extra->texture_palette & 0x8) ? 1 : 0;
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

	UINT32 *fb = BITMAP_ADDR32(destmap, scanline, 0);
	UINT32 *zb = BITMAP_ADDR32(K001005_zbuffer, scanline, 0);

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

			iiu = texture_x + tex_mirror_table[texture_mirror_x][(iu >> 4) & 0x7f];
			iiv = texture_y + tex_mirror_table[texture_mirror_y][(iv >> 4) & 0x7f];
			texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
			color = K001006_palette[pal_chip][palette_index + texel];

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

static void render_polygons(running_machine *machine)
{
	int i, j;
	const rectangle &visarea = machine->primary_screen->visible_area();

//  mame_printf_debug("K001005_fifo_ptr = %08X\n", K001005_3d_fifo_ptr);

	for (i=0; i < K001005_3d_fifo_ptr; i++)
	{
		if (K001005_3d_fifo[i] == 0x80000003)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int index = i;

			++index;

			for (j=0; j < 4; j++)
			{
				int x, y;

				x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
				y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = 0;	/* ??? */
			}

			++index;

			r = (K001005_3d_fifo[index] >>  0) & 0xff;
			g = (K001005_3d_fifo[index] >>  8) & 0xff;
			b = (K001005_3d_fifo[index] >> 16) & 0xff;
			a = (K001005_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			++index;

			extra->color = color;
			poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//          poly_render_polygon(poly, K001005_bitmap[K001005_bitmap_page],  &visarea, draw_scanline, 1, 4, v);

			i = index - 1;
		}
		else if (K001005_3d_fifo[i] == 0x800000ae || K001005_3d_fifo[i] == 0x8000008e ||
				 K001005_3d_fifo[i] == 0x80000096 || K001005_3d_fifo[i] == 0x800000b6 ||
				 K001005_3d_fifo[i] == 0x8000002e || K001005_3d_fifo[i] == 0x8000000e ||
				 K001005_3d_fifo[i] == 0x80000016 || K001005_3d_fifo[i] == 0x80000036 ||
				 K001005_3d_fifo[i] == 0x800000aa || K001005_3d_fifo[i] == 0x800000a8 ||
				 K001005_3d_fifo[i] == 0x800000b2)
		{
			// 0x00: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx    Command
			//
			// 0x01: xxxx---- -------- -------- --------    Texture palette
			// 0x01: -------- -------x xxxx---- --------    Texture page
			// 0x01: -------- -------- ----x-x- x-x-x-x-    Texture X / 8
			// 0x01: -------- -------- -----x-x -x-x-x-x    Texture Y / 8

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
			poly_vertex v[4];
			int tx, ty;
			UINT32 color = 0;
			UINT32 header;
			UINT32 command;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			command = K001005_3d_fifo[index++];
			header = K001005_3d_fifo[index++];

			for (j=0; j < 4; j++)
			{
				INT16 u2, v2;
				int x, y, z;
				int end = 0;

				x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
				y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = K001005_3d_fifo[index] & 0x4000;
				end = K001005_3d_fifo[index] & 0x8000;
				++index;

				z = K001005_3d_fifo[index];
				++index;

				if (end)
				{
					color = K001005_3d_fifo[index];
					++index;

					u2 = (K001005_3d_fifo[index] >> 16) & 0xffff;
					v2 = (K001005_3d_fifo[index] >>  0) & 0xffff;
					++index;
				}
				else
				{
					u2 = (K001005_3d_fifo[index] >> 16) & 0xffff;
					v2 = (K001005_3d_fifo[index] >>  0) & 0xffff;
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
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &prev_v[2], &v[0], &v[1]);
				if (prev_poly_type)
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &prev_v[2], &prev_v[3], &v[0]);
//              if (prev_poly_type)
//                  poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &prev_v[2], &prev_v[3], &v[0], &v[1]);
//              else
//                  poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &prev_v[2], &v[0], &v[1]);

				memcpy(&prev_v[0], &prev_v[2], sizeof(poly_vertex));
				memcpy(&prev_v[1], &prev_v[3], sizeof(poly_vertex));
				memcpy(&prev_v[2], &v[0], sizeof(poly_vertex));
				memcpy(&prev_v[3], &v[1], sizeof(poly_vertex));
			}
			else
			{
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (num_verts > 3)
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, num_verts, v);

				memcpy(prev_v, v, sizeof(poly_vertex) * 4);
			}

			prev_poly_type = poly_type;

			while ((K001005_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < K001005_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &prev_v[2], sizeof(poly_vertex));
				}

				for (j=2; j < 4; j++)
				{
					INT16 u2, v2;
					int x, y, z;
					int end = 0;

					x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
					y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = K001005_3d_fifo[index] & 0x4000;
					end = K001005_3d_fifo[index] & 0x8000;
					++index;

					z = K001005_3d_fifo[index];
					++index;

					if (end)
					{
						color = K001005_3d_fifo[index];
						++index;

						u2 = (K001005_3d_fifo[index] >> 16) & 0xffff;
						v2 = (K001005_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}
					else
					{
						u2 = (K001005_3d_fifo[index] >> 16) & 0xffff;
						v2 = (K001005_3d_fifo[index] >>  0) & 0xffff;
						++index;
					}

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);
					v[j].p[3] = 1.0f / v[j].p[0];
					v[j].p[1] = u2 * v[j].p[3];
					v[j].p[2] = v2 * v[j].p[3];

					++new_verts;

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

				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, &v[2], &v[3], &v[0]);
//              poly_render_polygon(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline_tex, 4, new_verts + 2, v);

				memcpy(prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (K001005_3d_fifo[i] == 0x80000006 || K001005_3d_fifo[i] == 0x80000026 ||
				 K001005_3d_fifo[i] == 0x80000020 || K001005_3d_fifo[i] == 0x80000022)
		{
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
			poly_vertex v[4];
			int r, g, b, a;
			UINT32 color;
			int num_verts = 0;
			int index = i;
			int poly_type = 0;

			++index;

			for (j=0; j < 4; j++)
			{
				int x, y, z;
				int end = 0;

				x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
				y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
				x |= ((x & 0x2000) ? 0xffffc000 : 0);
				y |= ((y & 0x1000) ? 0xffffe000 : 0);

				poly_type = K001005_3d_fifo[index] & 0x4000;
				end = K001005_3d_fifo[index] & 0x8000;
				++index;

				z = K001005_3d_fifo[index];
				++index;

				v[j].x = ((float)(x) / 16.0f) + 256.0f;
				v[j].y = ((float)(-y) / 16.0f) + 192.0f;
				v[j].p[0] = *(float*)(&z);

				++num_verts;

				if (end)
					break;
			}

			r = (K001005_3d_fifo[index] >>  0) & 0xff;
			g = (K001005_3d_fifo[index] >>  8) & 0xff;
			b = (K001005_3d_fifo[index] >> 16) & 0xff;
			a = (K001005_3d_fifo[index] >> 24) & 0xff;
			color = (a << 24) | (r << 16) | (g << 8) | (b);
			index++;

			extra->color = color;

			poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
			if (num_verts > 3)
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[2], &v[3], &v[0]);
//          poly_render_polygon(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, num_verts, v);

			memcpy(prev_v, v, sizeof(poly_vertex) * 4);

			while ((K001005_3d_fifo[index] & 0xffffff00) != 0x80000000 && index < K001005_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
				int new_verts = 0;

				if (poly_type)
				{
					memcpy(&v[0], &prev_v[2], sizeof(poly_vertex));
					memcpy(&v[1], &prev_v[3], sizeof(poly_vertex));
				}
				else
				{
					memcpy(&v[0], &prev_v[1], sizeof(poly_vertex));
					memcpy(&v[1], &prev_v[2], sizeof(poly_vertex));
				}

				for (j=2; j < 4; j++)
				{
					int x, y, z;
					int end = 0;

					x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
					y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
					x |= ((x & 0x2000) ? 0xffffc000 : 0);
					y |= ((y & 0x1000) ? 0xffffe000 : 0);

					poly_type = K001005_3d_fifo[index] & 0x4000;
					end = K001005_3d_fifo[index] & 0x8000;
					++index;

					z = K001005_3d_fifo[index];
					++index;

					v[j].x = ((float)(x) / 16.0f) + 256.0f;
					v[j].y = ((float)(-y) / 16.0f) + 192.0f;
					v[j].p[0] = *(float*)(&z);

					++new_verts;

					if (end)
						break;
				}

				r = (K001005_3d_fifo[index] >>  0) & 0xff;
				g = (K001005_3d_fifo[index] >>  8) & 0xff;
				b = (K001005_3d_fifo[index] >> 16) & 0xff;
				a = (K001005_3d_fifo[index] >> 24) & 0xff;
				color = (a << 24) | (r << 16) | (g << 8) | (b);
				index++;

				extra->color = color;

				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[1], &v[2]);
				if (new_verts > 1)
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, &v[0], &v[2], &v[3]);
//              poly_render_polygon(poly, K001005_bitmap[K001005_bitmap_page], &visarea, draw_scanline, 1, new_verts + 2, v);

				memcpy(prev_v, v, sizeof(poly_vertex) * 4);
			};

			i = index - 1;
		}
		else if (K001005_3d_fifo[i] == 0x80000000)
		{

		}
		else if ((K001005_3d_fifo[i] & 0xffffff00) == 0x80000000)
		{
			/*
            mame_printf_debug("Unknown polygon type %08X:\n", K001005_3d_fifo[i]);
            for (j=0; j < 0x20; j++)
            {
                mame_printf_debug("  %02X: %08X\n", j, K001005_3d_fifo[i+1+j]);
            }
            mame_printf_debug("\n");
            */
		}
	}
}

void K001005_draw(bitmap_t *bitmap, const rectangle *cliprect)
{
	int i, j;

	memcpy(&K001005_cliprect, cliprect, sizeof(rectangle));

	for (j=cliprect->min_y; j <= cliprect->max_y; j++)
	{
		UINT32 *bmp = BITMAP_ADDR32(bitmap, j, 0);
		UINT32 *src = BITMAP_ADDR32(K001005_bitmap[K001005_bitmap_page^1], j, 0);

		for (i=cliprect->min_x; i <= cliprect->max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}

void K001005_swap_buffers(running_machine *machine)
{
	K001005_bitmap_page ^= 1;

	//if (K001005_status == 2)
	{
		bitmap_fill(K001005_bitmap[K001005_bitmap_page], &K001005_cliprect, machine->pens[0]&0x00ffffff);
		bitmap_fill(K001005_zbuffer, &K001005_cliprect, 0xffffffff);
	}
}

static int tick = 0;
static int debug_tex_page = 0;
static int debug_tex_palette = 0;

VIDEO_START( gticlub )
{
	tick = 0;
	debug_tex_page = 0;
	debug_tex_palette = 0;

	K001006_init(machine);
	K001005_init(machine);
}

VIDEO_UPDATE( gticlub )
{
	running_device *k001604 = devtag_get_device(screen->machine, "k001604_1");

	k001604_draw_back_layer(k001604, bitmap, cliprect);

	K001005_draw(bitmap, cliprect);

	k001604_draw_front_layer(k001604, bitmap, cliprect);

	tick++;
	if( tick >= 5 ) {
		tick = 0;

		if( input_code_pressed(screen->machine, KEYCODE_O) )
			debug_tex_page++;

		if( input_code_pressed(screen->machine, KEYCODE_I) )
			debug_tex_page--;

		if (input_code_pressed(screen->machine, KEYCODE_U))
			debug_tex_palette++;
		if (input_code_pressed(screen->machine, KEYCODE_Y))
			debug_tex_palette--;

		if (debug_tex_page < 0)
			debug_tex_page = 32;
		if (debug_tex_page > 32)
			debug_tex_page = 0;

		if (debug_tex_palette < 0)
			debug_tex_palette = 15;
		if (debug_tex_palette > 15)
			debug_tex_palette = 0;
	}

#if 0
    if (debug_tex_page > 0)
    {
        char string[200];
        int x,y;
        int index = (debug_tex_page - 1) * 0x40000;
        int pal = debug_tex_palette & 7;
        int tp = (debug_tex_palette >> 3) & 1;
        UINT8 *rom = memory_region(machine, "gfx1");

        for (y=0; y < 384; y++)
        {
            for (x=0; x < 512; x++)
            {
                UINT8 pixel = rom[index + (y*512) + x];
                *BITMAP_ADDR32(bitmap, y, x) = K001006_palette[tp][(pal * 256) + pixel];
            }
        }

        sprintf(string, "Texture page %d\nPalette %d", debug_tex_page, debug_tex_palette);
        //popmessage("%s", string);
    }
#endif

	draw_7segment_led(bitmap, 3, 3, gticlub_led_reg0);
	draw_7segment_led(bitmap, 9, 3, gticlub_led_reg1);

	//cputag_set_input_line(screen->machine, "dsp", SHARC_INPUT_FLAG1, ASSERT_LINE);
	sharc_set_flag_input(devtag_get_device(screen->machine, "dsp"), 1, ASSERT_LINE);
	return 0;
}

