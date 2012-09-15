#include <float.h>
#include "emu.h"
#include "cpu/sharc/sharc.h"
#include "machine/konppc.h"
#include "video/voodoo.h"
#include "video/poly.h"
#include "video/konicdev.h"
#include "video/gticlub.h"

/*
    TODO:
        - Fog equation and parameters are probably not accurate.
        - Winding Heat (and maybe others) have slight Z-fighting problems.
        - 3D isn't always turned off properly (during title screens for example).
          Figure out what controls this. Video mixer, layer priority or some 3D register?

*/

#define POLY_Z		0
#define POLY_BRI	2
#define POLY_FOG	1
#define POLY_U		3
#define POLY_V		4
#define POLY_W		5

#define POLY_R		3
#define POLY_G		4
#define POLY_B		5
#define POLY_A		2

#define ZBUFFER_MAX					10000000000.0f

#define LOG_POLY_FIFO				0

#if LOG_POLY_FIFO
static int count = 0;
#endif

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	UINT32 color;
	int texture_x, texture_y;
	int texture_width, texture_height;
	int texture_page;
	int texture_palette;
	int texture_mirror_x;
	int texture_mirror_y;
	int light_r, light_g, light_b;
	int ambient_r, ambient_g, ambient_b;
	int fog_r, fog_g, fog_b;
	UINT32 flags;
};

static UINT8 gticlub_led_reg[2];

void gticlub_led_setreg(int offset, UINT8 data)
{
	gticlub_led_reg[offset] = data;
}


/*****************************************************************************/
/* Konami K001006 Custom 3D Texel Renderer chip (KS10081) */

#define MAX_K001006_CHIPS		2

static UINT16 *K001006_pal_ram[MAX_K001006_CHIPS];
static UINT16 *K001006_unknown_ram[MAX_K001006_CHIPS];
static UINT32 K001006_addr[MAX_K001006_CHIPS] = { 0, 0 };
static int K001006_device_sel[MAX_K001006_CHIPS] = { 0, 0 };

static UINT32 *K001006_palette[MAX_K001006_CHIPS];

void K001006_init(running_machine &machine)
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

static UINT32 K001006_r(running_machine &machine, int chip, int offset, UINT32 mem_mask)
{
	if (offset == 1)
	{
		switch (K001006_device_sel[chip])
		{
			case 0x0b:		// CG Board ROM read
			{
				UINT16 *rom = (UINT16*)machine.root_device().memregion("gfx1")->base();
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
				fatalerror("K001006_r chip %d, unknown device %02X\n", chip, K001006_device_sel[chip]);
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
				//mame_printf_debug("K001006_w: chip %d, device %02X, write %04X to %08X\n", chip, K001006_device_sel[chip], data & 0xffff, K001006_addr[0]++);
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
	return K001006_r(space->machine(), 0, offset, mem_mask);
}

WRITE32_HANDLER(K001006_0_w)
{
	K001006_w(0, offset, data, mem_mask);
}

READ32_HANDLER(K001006_1_r)
{
	return K001006_r(space->machine(), 1, offset, mem_mask);
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
static bitmap_rgb32 *K001005_bitmap[2];
static bitmap_ind32 *K001005_zbuffer;
static rectangle K001005_cliprect;

static void render_polygons(running_machine &machine);

static UINT8 *K001005_texture;

static UINT16 *K001005_ram[2];
static int K001005_ram_ptr = 0;
static UINT32 *K001005_fifo;
static int K001005_fifo_read_ptr = 0;
static int K001005_fifo_write_ptr = 0;

static UINT32 *K001005_3d_fifo;
static int K001005_3d_fifo_ptr = 0;

static int *tex_mirror_table[2][8];

static int K001005_bitmap_page = 0;

static poly_manager *poly;
static poly_vertex prev_v[4];

static UINT32 fog_r, fog_g, fog_b;
static UINT32 ambient_r, ambient_g, ambient_b;
static UINT32 light_r, light_g, light_b;

static UINT32 reg_far_z;
static float far_z;
static float fog_density = 1.5f;

static UINT8 *gfxrom;

static void K001005_exit(running_machine &machine)
{
	poly_free(poly);
}

void K001005_init(running_machine &machine)
{
	int i,k;

	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();
	K001005_zbuffer = auto_bitmap_ind32_alloc(machine, width, height);

	gfxrom = machine.root_device().memregion("gfx1")->base();

	K001005_bitmap[0] = auto_bitmap_rgb32_alloc(machine, machine.primary_screen->width(), machine.primary_screen->height());
	K001005_bitmap[1] = auto_bitmap_rgb32_alloc(machine, machine.primary_screen->width(), machine.primary_screen->height());

	K001005_texture = auto_alloc_array(machine, UINT8, 0x800000);

	K001005_ram[0] = auto_alloc_array(machine, UINT16, 0x140000);
	K001005_ram[1] = auto_alloc_array(machine, UINT16, 0x140000);

	K001005_fifo = auto_alloc_array(machine, UINT32, 0x800);

	K001005_3d_fifo = auto_alloc_array(machine, UINT32, 0x10000);

	poly = poly_alloc(machine, 10000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(K001005_exit), &machine));

	for (k=0; k < 8; k++)
	{
		tex_mirror_table[0][k] = auto_alloc_array(machine, int, 128);
		tex_mirror_table[1][k] = auto_alloc_array(machine, int, 128);

		int size = (k+1)*8;

		for (i=0; i < 128; i++)
		{
			tex_mirror_table[0][k][i] = i % size;
			tex_mirror_table[1][k][i] = (i % (size*2)) >= size ? ((size - 1) - (i % size)) : (i % size);
		}
	}

	K001005_status = 0;
	K001005_ram_ptr = 0;
	K001005_fifo_read_ptr = 0;
	K001005_fifo_write_ptr = 0;
	K001005_3d_fifo_ptr = 0;
	K001005_bitmap_page = 0;

	memset(prev_v, 0, sizeof(poly_vertex)*4);
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
					//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(space->machine().device("dsp"), 1, CLEAR_LINE);
				}
				else
				{
					//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(space->machine().device("dsp"), 1, ASSERT_LINE);
				}
			}
			else
			{
				//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(space->machine().device("dsp"), 1, ASSERT_LINE);
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
			mame_printf_debug("K001005_r: %08X, %08X at %08X\n", offset, mem_mask, space->device().safe_pc());
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
					//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
					sharc_set_flag_input(space->machine().device("dsp"), 1, ASSERT_LINE);
				}
				else
				{
					//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, CLEAR_LINE);
					sharc_set_flag_input(space->machine().device("dsp"), 1, CLEAR_LINE);
				}
			}
			else
			{
				//space->machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
				sharc_set_flag_input(space->machine().device("dsp"), 1, ASSERT_LINE);
			}

	    //  mame_printf_debug("K001005 FIFO write: %08X at %08X\n", data, space->device().safe_pc());
			K001005_fifo[K001005_fifo_write_ptr] = data;
			K001005_fifo_write_ptr++;
			K001005_fifo_write_ptr &= 0x7ff;

			// process the current vertex data if a sync command is being sent (usually means the global registers are being changed)
			if (data == 0x80000000)
			{
				render_polygons(space->machine());
				K001005_3d_fifo_ptr = 0;
			}

			K001005_3d_fifo[K001005_3d_fifo_ptr++] = data;

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
			if (space->device().safe_pc() == 0x201ee)
			{
				// This is used to make the SHARC timeout
				space->device().execute().spin_until_trigger(10000);
			}
			// !!! HACK to get past the FIFO B test (Winding Heat & Midnight Run) !!!
			if (space->device().safe_pc() == 0x201e6)
			{
				// This is used to make the SHARC timeout
				space->device().execute().spin_until_trigger(10000);
			}

			break;
		}

		case 0x100:		break;

		case 0x101:		break;		// viewport x and width?
		case 0x102:		break;		// viewport y and height?

		case 0x104:		break;		// viewport center x? (usually 0xff)
		case 0x105:		break;		// viewport center y? (usually 0xbf)

		case 0x108:					// far Z value, 4 exponent bits?
			{
				// this register seems to hold the 4 missing exponent bits...
				reg_far_z = (reg_far_z & 0x0000ffff) | ((data & 0xf) << 16);
				UINT32 fz = reg_far_z << 11;
				far_z = *(float*)&fz;
				if (far_z == 0.0f)		// just in case...
					far_z = 1.0f;
				break;
			}

		case 0x109:					// far Z value
			{
				// the SHARC code throws away the bottom 11 bits of mantissa and the top 5 bits (to fit in a 16-bit register?)
				reg_far_z = (reg_far_z & 0xffff0000) | (data & 0xffff);
				UINT32 fz = reg_far_z << 11;
				far_z = *(float*)&fz;
				if (far_z == 0.0f)		// just in case...
					far_z = 1.0f;
				break;
			}

		case 0x10a:     light_r = data & 0xff; break;
		case 0x10b:     light_g = data & 0xff; break;
		case 0x10c:     light_b = data & 0xff; break;

		case 0x10d:		ambient_r = data & 0xff; break;
		case 0x10e:		ambient_g = data & 0xff; break;
		case 0x10f:		ambient_b = data & 0xff; break;

		case 0x110:		fog_r = data & 0xff; break;
		case 0x111:		fog_g = data & 0xff; break;
		case 0x112:		fog_b = data & 0xff; break;

		case 0x11a:
			K001005_status = data;
			K001005_fifo_write_ptr = 0;
			K001005_fifo_read_ptr = 0;

			if (data == 2 && K001005_3d_fifo_ptr > 0)
			{
				render_polygons(space->machine());
				poly_wait(poly, "render_polygons");

#if LOG_POLY_FIFO
				count = 0;
				printf("\nrender %d\n", K001005_3d_fifo_ptr);
				printf("------------------------------------\n");
#endif

				K001005_3d_fifo_ptr = 0;
				K001005_swap_buffers(space->machine());
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
			//mame_printf_debug("K001005_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, space->device().safe_pc());
			break;
	}

}

static void draw_scanline_2d(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_rgb32 *destmap = (bitmap_rgb32 *)dest;
	UINT32 *fb = &destmap->pix32(scanline);
	float *zb = (float*)&K001005_zbuffer->pix32(scanline);
	UINT32 color = extra->color;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		if (color & 0xff000000)
		{
			fb[x] = color;
			zb[x] = FLT_MAX;		// FIXME
		}
	}
}

static void draw_scanline_2d_tex(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_rgb32 *destmap = (bitmap_rgb32 *)dest;
	UINT8 *texrom = gfxrom + (extra->texture_page * 0x40000);
	int pal_chip = (extra->texture_palette & 0x8) ? 1 : 0;
	int palette_index = (extra->texture_palette & 0x7) * 256;
	float u = extent->param[POLY_U].start;
	float v = extent->param[POLY_V].start;
	float du = extent->param[POLY_U].dpdx;
	float dv = extent->param[POLY_V].dpdx;
	UINT32 *fb = &destmap->pix32(scanline);
	float *zb = (float*)&K001005_zbuffer->pix32(scanline);
	UINT32 color = extra->color;
	int texture_mirror_x = extra->texture_mirror_x;
	int texture_mirror_y = extra->texture_mirror_y;
	int texture_x = extra->texture_x;
	int texture_y = extra->texture_y;
	int texture_width = extra->texture_width;
	int texture_height = extra->texture_height;
	int x;

	int *x_mirror_table = tex_mirror_table[texture_mirror_x][texture_width];
	int *y_mirror_table = tex_mirror_table[texture_mirror_y][texture_height];

	for (x = extent->startx; x < extent->stopx; x++)
	{
		int iu = (int)(u);
		int iv = (int)(v);
		int iiv, iiu, texel;

		int iu2 = (iu >> 4) + ((iu & 0x8) ? 1 : 0);
		int iv2 = (iv >> 4) + ((iv & 0x8) ? 1 : 0);

		iiu = texture_x + x_mirror_table[iu2 & 0x7f];
		iiv = texture_y + y_mirror_table[iv2 & 0x7f];

		texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
		color = K001006_palette[pal_chip][palette_index + texel];

		if (color & 0xff000000)
		{
			fb[x] = color;
			zb[x] = FLT_MAX;		// FIXME
		}

		u += du;
		v += dv;
	}
}

static void draw_scanline(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_rgb32 *destmap = (bitmap_rgb32 *)dest;
	float z = extent->param[POLY_Z].start;
	float dz = extent->param[POLY_Z].dpdx;
	float bri = extent->param[POLY_BRI].start;
	float dbri = extent->param[POLY_BRI].dpdx;
	float fog = extent->param[POLY_FOG].start;
	float dfog = extent->param[POLY_FOG].dpdx;
	UINT32 *fb = &destmap->pix32(scanline);
	float *zb = (float*)&K001005_zbuffer->pix32(scanline);
	UINT32 color = extra->color;
	int x;

	int poly_light_r = extra->light_r + extra->ambient_r;
	int poly_light_g = extra->light_g + extra->ambient_g;
	int poly_light_b = extra->light_b + extra->ambient_b;
	if (poly_light_r > 255) poly_light_r = 255;
	if (poly_light_g > 255) poly_light_g = 255;
	if (poly_light_b > 255) poly_light_b = 255;
	int poly_fog_r = extra->fog_r;
	int poly_fog_g = extra->fog_g;
	int poly_fog_b = extra->fog_b;

	for (x = extent->startx; x < extent->stopx; x++)
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

static void draw_scanline_tex(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_rgb32 *destmap = (bitmap_rgb32 *)dest;
	UINT8 *texrom = gfxrom + (extra->texture_page * 0x40000);
	int pal_chip = (extra->texture_palette & 0x8) ? 1 : 0;
	int palette_index = (extra->texture_palette & 0x7) * 256;
	float z = extent->param[POLY_Z].start;
	float u = extent->param[POLY_U].start;
	float v = extent->param[POLY_V].start;
	float w = extent->param[POLY_W].start;
	float dz = extent->param[POLY_Z].dpdx;
	float du = extent->param[POLY_U].dpdx;
	float dv = extent->param[POLY_V].dpdx;
	float dw = extent->param[POLY_W].dpdx;
	float bri = extent->param[POLY_BRI].start;
	float dbri = extent->param[POLY_BRI].dpdx;
	float fog = extent->param[POLY_FOG].start;
	float dfog = extent->param[POLY_FOG].dpdx;
	int texture_mirror_x = extra->texture_mirror_x;
	int texture_mirror_y = extra->texture_mirror_y;
	int texture_x = extra->texture_x;
	int texture_y = extra->texture_y;
	int texture_width = extra->texture_width;
	int texture_height = extra->texture_height;
	int x;

	int poly_light_r = extra->light_r + extra->ambient_r;
	int poly_light_g = extra->light_g + extra->ambient_g;
	int poly_light_b = extra->light_b + extra->ambient_b;
	if (poly_light_r > 255) poly_light_r = 255;
	if (poly_light_g > 255) poly_light_g = 255;
	if (poly_light_b > 255) poly_light_b = 255;
	int poly_fog_r = extra->fog_r;
	int poly_fog_g = extra->fog_g;
	int poly_fog_b = extra->fog_b;

	UINT32 *fb = &destmap->pix32(scanline);
	float *zb = (float*)&K001005_zbuffer->pix32(scanline);
	int *x_mirror_table = tex_mirror_table[texture_mirror_x][texture_width];
	int *y_mirror_table = tex_mirror_table[texture_mirror_y][texture_height];

	for (x = extent->startx; x < extent->stopx; x++)
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
			int iiv, iiu, texel;

			iu = u * oow;
			iv = v * oow;

			int iu2 = (iu >> 4) + ((iu & 0x8) ? 1 : 0);
			int iv2 = (iv >> 4) + ((iv & 0x8) ? 1 : 0);

			iiu = texture_x + x_mirror_table[iu2 & 0x7f];
			iiv = texture_y + y_mirror_table[iv2 & 0x7f];

			texel = texrom[((iiv & 0x1ff) * 512) + (iiu & 0x1ff)];
			color = K001006_palette[pal_chip][palette_index + texel];

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

static void draw_scanline_gouraud_blend(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	bitmap_rgb32 *destmap = (bitmap_rgb32 *)dest;
	float z = extent->param[POLY_Z].start;
	float dz = extent->param[POLY_Z].dpdx;
	float r = extent->param[POLY_R].start;
	float dr = extent->param[POLY_R].dpdx;
	float g = extent->param[POLY_G].start;
	float dg = extent->param[POLY_G].dpdx;
	float b = extent->param[POLY_B].start;
	float db = extent->param[POLY_B].dpdx;
	float a = extent->param[POLY_A].start;
	float da = extent->param[POLY_A].dpdx;
	UINT32 *fb = &destmap->pix32(scanline);
	float *zb = (float*)&K001005_zbuffer->pix32(scanline);
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
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

static void render_polygons(running_machine &machine)
{
	const rectangle& visarea = machine.primary_screen->visible_area();
	poly_vertex v[4];
	int poly_type;
	int brightness;

	poly_vertex *vertex1;
	poly_vertex *vertex2;
	poly_vertex *vertex3;
	poly_vertex *vertex4;

	UINT32 *fifo = K001005_3d_fifo;

	int index = 0;

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
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);

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

				poly_type = fifo[index] & 0x4000;		// 0 = triangle, 1 = quad
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
				v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
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

			extra->texture_x = tex_x * 8;
			extra->texture_y = tex_y * 8;
			extra->texture_width = (header >> 23) & 0x7;
			extra->texture_height = (header >> 20) & 0x7;
			extra->texture_page = (header >> 12) & 0x1f;
			extra->texture_palette = (header >> 28) & 0xf;
			extra->texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);
			extra->texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);
			extra->color = color;
			extra->light_r = light_r;		extra->light_g = light_g;		extra->light_b = light_b;
			extra->ambient_r = ambient_r;	extra->ambient_g = ambient_g;	extra->ambient_b = ambient_b;
			extra->fog_r = fog_r;			extra->fog_g = fog_g;			extra->fog_b = fog_b;
			extra->flags = cmd;

			if ((cmd & 0x20) == 0)		// possibly enable flag for gouraud shading (fixes some shading errors)
			{
				v[0].p[POLY_BRI] = brightness;
				v[1].p[POLY_BRI] = brightness;
			}

			if (poly_type == 0)		// triangle
			{
				if (vert_num == 1)
				{
					vertex1 = &prev_v[2];
					vertex2 = &prev_v[3];
					vertex3 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
				}

				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, vertex1, vertex2, vertex3);

				memcpy(&prev_v[1], vertex1, sizeof(poly_vertex));
				memcpy(&prev_v[2], vertex2, sizeof(poly_vertex));
				memcpy(&prev_v[3], vertex3, sizeof(poly_vertex));
			}
			else					// quad
			{
				if (vert_num == 1)
				{
					vertex1 = &prev_v[1];
					vertex2 = &prev_v[2];
					vertex3 = &prev_v[3];
					vertex4 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &prev_v[2];
					vertex2 = &prev_v[3];
					vertex3 = &v[0];
					vertex4 = &v[1];
				}
				else if (vert_num == 3)
				{
					vertex1 = &prev_v[3];
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

				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, vertex1, vertex2, vertex3);
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, vertex3, vertex4, vertex1);

				memcpy(&prev_v[0], vertex1, sizeof(poly_vertex));
				memcpy(&prev_v[1], vertex2, sizeof(poly_vertex));
				memcpy(&prev_v[2], vertex3, sizeof(poly_vertex));
				memcpy(&prev_v[3], vertex4, sizeof(poly_vertex));
			}

			while ((fifo[index] & 0xffffff00) != 0x80000000 && index < K001005_3d_fifo_ptr)
			{
				poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
				int new_verts = 0;

				memcpy(&v[0], &prev_v[2], sizeof(poly_vertex));
				memcpy(&v[1], &prev_v[3], sizeof(poly_vertex));

				last_vertex = 0;
				vert_num = 2;
				do
				{
					int x, y, z;
					INT16 tu, tv;

					x = ((K001005_3d_fifo[index] >>  0) & 0x3fff);
					y = ((K001005_3d_fifo[index] >> 16) & 0x1fff);
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

					tu = (K001005_3d_fifo[index] >> 16) & 0xffff;
					tv = (K001005_3d_fifo[index] >>  0) & 0xffff;
					index++;

					v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
					v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
					v[vert_num].p[POLY_Z] = *(float*)(&z);
					v[vert_num].p[POLY_W] = 1.0f / v[vert_num].p[POLY_Z];
					v[vert_num].p[POLY_U] = tu * v[vert_num].p[POLY_W];
					v[vert_num].p[POLY_V] = tv * v[vert_num].p[POLY_W];
					v[vert_num].p[POLY_BRI] = brightness;
					v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
					//v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
					if (v[vert_num].p[POLY_FOG] < 0.0f) v[vert_num].p[POLY_FOG] = 0.0f;
					if (v[vert_num].p[POLY_FOG] > 65536.0f) v[vert_num].p[POLY_FOG] = 65536.0f;

					vert_num++;
					new_verts++;
				}
				while (!last_vertex);

				extra->texture_x = tex_x * 8;
				extra->texture_y = tex_y * 8;
				extra->texture_width = (header >> 23) & 0x7;
				extra->texture_height = (header >> 20) & 0x7;

				extra->texture_page = (header >> 12) & 0x1f;
				extra->texture_palette = (header >> 28) & 0xf;

				extra->texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);// & ((header & 0x00400000) ? 0x1 : 0);
				extra->texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);// & ((header & 0x00400000) ? 0x1 : 0);

				extra->color = color;
				extra->light_r = light_r;		extra->light_g = light_g;		extra->light_b = light_b;
				extra->ambient_r = ambient_r;	extra->ambient_g = ambient_g;	extra->ambient_b = ambient_b;
				extra->fog_r = fog_r;			extra->fog_g = fog_g;			extra->fog_b = fog_b;
				extra->flags = cmd;

				if ((cmd & 0x20) == 0)		// possibly enable flag for gouraud shading (fixes some shading errors)
				{
					v[0].p[POLY_BRI] = brightness;
					v[1].p[POLY_BRI] = brightness;
				}

				if (new_verts == 1)
				{
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, &v[0], &v[1], &v[2]);

					memcpy(&prev_v[1], &v[0], sizeof(poly_vertex));
					memcpy(&prev_v[2], &v[1], sizeof(poly_vertex));
					memcpy(&prev_v[3], &v[2], sizeof(poly_vertex));
				}
				else if (new_verts == 2)
				{
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, &v[0], &v[1], &v[2]);
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_tex, 6, &v[2], &v[3], &v[0]);

					memcpy(&prev_v[0], &v[0], sizeof(poly_vertex));
					memcpy(&prev_v[1], &v[1], sizeof(poly_vertex));
					memcpy(&prev_v[2], &v[2], sizeof(poly_vertex));
					memcpy(&prev_v[3], &v[3], sizeof(poly_vertex));
				}
			};
		}
		else if (cmd == 0x80000006 || cmd == 0x80000026 || cmd == 0x80000002 || cmd == 0x80000020 || cmd == 0x80000022)
		{
			// no texture, Z

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
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

				poly_type = fifo[index] & 0x4000;		// 0 = triangle, 1 = quad
				last_vertex = fifo[index] & 0x8000;
				index++;

				z = fifo[index] & 0xffffff00;
				brightness = fifo[index] & 0xff;
				index++;

				v[vert_num].x = ((float)(x) / 16.0f) + 256.0f;
				v[vert_num].y = ((float)(-y) / 16.0f) + 192.0f + 8;
				v[vert_num].p[POLY_Z] = *(float*)(&z);
				v[vert_num].p[POLY_BRI] = brightness;
				v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
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

			extra->color = color;
			extra->light_r = light_r;		extra->light_g = light_g;		extra->light_b = light_b;
			extra->ambient_r = ambient_r;	extra->ambient_g = ambient_g;	extra->ambient_b = ambient_b;
			extra->fog_r = fog_r;			extra->fog_g = fog_g;			extra->fog_b = fog_b;
			extra->flags = cmd;

			if ((cmd & 0x20) == 0)		// possibly enable flag for gouraud shading (fixes some shading errors)
			{
				v[0].p[POLY_BRI] = brightness;
				v[1].p[POLY_BRI] = brightness;
			}

			if (poly_type == 0)		// triangle
			{
				if (vert_num == 1)
				{
					vertex1 = &prev_v[2];
					vertex2 = &prev_v[3];
					vertex3 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &prev_v[3];
					vertex2 = &v[0];
					vertex3 = &v[1];
				}
				else
				{
					vertex1 = &v[0];
					vertex2 = &v[1];
					vertex3 = &v[2];
				}

				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline, 3, vertex1, vertex2, vertex3);

				memcpy(&prev_v[1], vertex1, sizeof(poly_vertex));
				memcpy(&prev_v[2], vertex2, sizeof(poly_vertex));
				memcpy(&prev_v[3], vertex3, sizeof(poly_vertex));
			}
			else					// quad
			{
				if (vert_num == 1)
				{
					vertex1 = &prev_v[1];
					vertex2 = &prev_v[2];
					vertex3 = &prev_v[3];
					vertex4 = &v[0];
				}
				else if (vert_num == 2)
				{
					vertex1 = &prev_v[2];
					vertex2 = &prev_v[3];
					vertex3 = &v[0];
					vertex4 = &v[1];
				}
				else if (vert_num == 3)
				{
					vertex1 = &prev_v[3];
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

				poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline, 3, vertex1, vertex2, vertex3, vertex4);

				memcpy(&prev_v[0], vertex1, sizeof(poly_vertex));
				memcpy(&prev_v[1], vertex2, sizeof(poly_vertex));
				memcpy(&prev_v[2], vertex3, sizeof(poly_vertex));
				memcpy(&prev_v[3], vertex4, sizeof(poly_vertex));
			}

			while ((fifo[index] & 0xffffff00) != 0x80000000 && index < K001005_3d_fifo_ptr)
			{
				int new_verts = 0;

				memcpy(&v[0], &prev_v[2], sizeof(poly_vertex));
				memcpy(&v[1], &prev_v[3], sizeof(poly_vertex));

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
					v[vert_num].p[POLY_FOG] = (1.0f / (exp( ((v[vert_num].p[POLY_Z] * fog_density) / far_z) * ((v[vert_num].p[POLY_Z] * fog_density) / far_z) ))) * 65536.0f;
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

				extra->color = color;
				extra->light_r = light_r;		extra->light_g = light_g;		extra->light_b = light_b;
				extra->ambient_r = ambient_r;	extra->ambient_g = ambient_g;	extra->ambient_b = ambient_b;
				extra->fog_r = fog_r;			extra->fog_g = fog_g;			extra->fog_b = fog_b;
				extra->flags = cmd;

				if ((cmd & 0x20) == 0)		// possibly enable flag for gouraud shading (fixes some shading errors)
				{
					v[0].p[POLY_BRI] = brightness;
					v[1].p[POLY_BRI] = brightness;
				}

				if (new_verts == 1)
				{
					poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline, 3, &v[0], &v[1], &v[2]);

					memcpy(&prev_v[1], &v[0], sizeof(poly_vertex));
					memcpy(&prev_v[2], &v[1], sizeof(poly_vertex));
					memcpy(&prev_v[3], &v[2], sizeof(poly_vertex));
				}
				else if (new_verts == 2)
				{
					poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline, 3, &v[0], &v[1], &v[2], &v[3]);

					memcpy(&prev_v[0], &v[0], sizeof(poly_vertex));
					memcpy(&prev_v[1], &v[1], sizeof(poly_vertex));
					memcpy(&prev_v[2], &v[2], sizeof(poly_vertex));
					memcpy(&prev_v[3], &v[3], sizeof(poly_vertex));
				}
			}
		}
		else if (cmd == 0x80000003 || cmd == 0x80000001)
		{
			// no texture, no Z

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
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

			extra->color = color;
			extra->flags = cmd;

			if (poly_type == 0)
			{
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_2d, 0, &v[0], &v[1], &v[2]);
			}
			else
			{
				poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_2d, 0, &v[0], &v[1], &v[2], &v[3]);
			}
		}
		else if (cmd == 0x8000008b)
		{
			// texture, no Z

			int tex_x, tex_y;
			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
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
			extra->color = (a << 24) | (r << 16) | (g << 8) | (b);
			extra->flags = cmd;

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

			extra->texture_x = tex_x * 8;
			extra->texture_y = tex_y * 8;
			extra->texture_width = (header >> 23) & 0x7;
			extra->texture_height = (header >> 20) & 0x7;

			extra->texture_page = (header >> 12) & 0x1f;
			extra->texture_palette = (header >> 28) & 0xf;

			extra->texture_mirror_x = ((cmd & 0x10) ? 0x1 : 0);
			extra->texture_mirror_y = ((cmd & 0x10) ? 0x1 : 0);

			if (poly_type == 0)
			{
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_2d_tex, 5, &v[0], &v[1], &v[2]);
			}
			else
			{
				poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_2d_tex, 5, &v[0], &v[1], &v[2], &v[3]);
			}
		}
		else if (cmd == 0x80000121)
		{
			// no texture, color gouraud, Z

			poly_extra_data *extra = (poly_extra_data *)poly_get_extra_data(poly);
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

			extra->color = color;
			extra->flags = cmd;

			if (poly_type == 0)
			{
				poly_render_triangle(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_gouraud_blend, 6, &v[0], &v[1], &v[2]);
			}
			else
			{
				poly_render_quad(poly, K001005_bitmap[K001005_bitmap_page], visarea, draw_scanline_gouraud_blend, 6, &v[0], &v[1], &v[2], &v[3]);
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
            mame_printf_debug("Unknown polygon type %08X:\n", fifo[index-1]);
            for (int i=0; i < 0x20; i++)
            {
                mame_printf_debug("  %02X: %08X\n", i, fifo[index+i]);
            }
            mame_printf_debug("\n");
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
	while (index < K001005_3d_fifo_ptr);
}


void K001005_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, j;

	memcpy(&K001005_cliprect, &cliprect, sizeof(rectangle));

	for (j=cliprect.min_y; j <= cliprect.max_y; j++)
	{
		UINT32 *bmp = &bitmap.pix32(j);
		UINT32 *src = &K001005_bitmap[K001005_bitmap_page^1]->pix32(j);

		for (i=cliprect.min_x; i <= cliprect.max_x; i++)
		{
			if (src[i] & 0xff000000)
			{
				bmp[i] = src[i];
			}
		}
	}
}

void K001005_swap_buffers(running_machine &machine)
{
	K001005_bitmap_page ^= 1;

	//if (K001005_status == 2)
	{
		float zvalue = ZBUFFER_MAX;
		K001005_bitmap[K001005_bitmap_page]->fill(machine.pens[0]&0x00ffffff, K001005_cliprect);
		K001005_zbuffer->fill(*(int*)&zvalue, K001005_cliprect);
	}
}

/*
static int tick = 0;
static int debug_tex_page = 0;
static int debug_tex_palette = 0;
*/

VIDEO_START( gticlub )
{
	gticlub_led_reg[0] = gticlub_led_reg[1] = 0x7f;
	/*
    tick = 0;
    debug_tex_page = 0;
    debug_tex_palette = 0;
    */

	K001006_init(machine);
	K001005_init(machine);
}

SCREEN_UPDATE_RGB32( gticlub )
{
	device_t *k001604 = screen.machine().device("k001604_1");

	k001604_draw_back_layer(k001604, bitmap, cliprect);

	K001005_draw(bitmap, cliprect);

	k001604_draw_front_layer(k001604, bitmap, cliprect);

#if 0
	tick++;
	if( tick >= 5 ) {
		tick = 0;

		if( screen.machine().input().code_pressed(KEYCODE_O) )
			debug_tex_page++;

		if( screen.machine().input().code_pressed(KEYCODE_I) )
			debug_tex_page--;

		if (screen.machine().input().code_pressed(KEYCODE_U))
			debug_tex_palette++;
		if (screen.machine().input().code_pressed(KEYCODE_Y))
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

    if (debug_tex_page > 0)
    {
        char string[200];
        int x,y;
        int index = (debug_tex_page - 1) * 0x40000;
        int pal = debug_tex_palette & 7;
        int tp = (debug_tex_palette >> 3) & 1;
        UINT8 *rom = machine.root_device().memregion("gfx1")->base();

        for (y=0; y < 384; y++)
        {
            for (x=0; x < 512; x++)
            {
                UINT8 pixel = rom[index + (y*512) + x];
                bitmap.pix32(y, x) = K001006_palette[tp][(pal * 256) + pixel];
            }
        }

        sprintf(string, "Texture page %d\nPalette %d", debug_tex_page, debug_tex_palette);
        //popmessage("%s", string);
    }
#endif

	draw_7segment_led(bitmap, 3, 3, gticlub_led_reg[0]);
	draw_7segment_led(bitmap, 9, 3, gticlub_led_reg[1]);

	//screen.machine().device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG1, ASSERT_LINE);
	sharc_set_flag_input(screen.machine().device("dsp"), 1, ASSERT_LINE);
	return 0;
}

SCREEN_UPDATE_RGB32( hangplt )
{
	bitmap.fill(screen.machine().pens[0], cliprect);

	if (strcmp(screen.tag(), ":lscreen") == 0)
	{
		device_t *k001604 = screen.machine().device("k001604_1");
		device_t *voodoo = screen.machine().device("voodoo0");

	//  k001604_draw_back_layer(k001604, bitmap, cliprect);

		voodoo_update(voodoo, bitmap, cliprect);

		k001604_draw_front_layer(k001604, bitmap, cliprect);
	}
	else if (strcmp(screen.tag(), ":rscreen") == 0)
	{
		device_t *k001604 = screen.machine().device("k001604_2");
		device_t *voodoo = screen.machine().device("voodoo1");

	//  k001604_draw_back_layer(k001604, bitmap, cliprect);

		voodoo_update(voodoo, bitmap, cliprect);

		k001604_draw_front_layer(k001604, bitmap, cliprect);
	}

	draw_7segment_led(bitmap, 3, 3, gticlub_led_reg[0]);
	draw_7segment_led(bitmap, 9, 3, gticlub_led_reg[1]);

	return 0;
}

