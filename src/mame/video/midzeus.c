/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#include "driver.h"
#include "eminline.h"
#include "includes/midzeus.h"
#include "video/poly.h"
#include "video/rgbutil.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define DUMP_WAVE_RAM	0

#define WAVERAM_WIDTH	512
#define WAVERAM_HEIGHT	2048



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _poly_extra_data poly_extra_data;
struct _poly_extra_data
{
	const UINT64 *	palbase;
	const UINT64 *	texbase;
	UINT16			solidcolor;
	INT16			zoffset;
	UINT16			transcolor;
	UINT16			texwidth;
	UINT16			color;
	UINT32			alpha;
};



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT32 *zeusbase;

static poly_manager *poly;
static UINT8 log_fifo;

static UINT32 zeus_fifo[20];
static UINT8 zeus_fifo_words;
static INT16 zeus_matrix[3][3];
static INT32 zeus_point[3];
static INT16 zeus_light[3];
static UINT64 *zeus_renderbase;
static UINT64 *zeus_palbase;
static int zeus_enable_logging;
static UINT32 zeus_objdata;
static rectangle zeus_cliprect;

static UINT64 *waveram[2];

static UINT8 single_step;
static poly_extra_data single_step_extra;
static UINT32 single_step_val1;
static UINT32 single_step_val2;
static INT32 single_step_dx, single_step_dy;
static UINT8 single_step_advance;



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void exit_handler(running_machine *machine);

static void zeus_pointer_w(UINT32 which, UINT32 data, int logit);
static void zeus_register16_w(offs_t offset, UINT16 data, int logit);
static void zeus_register32_w(offs_t offset, UINT32 data, int logit);
static void zeus_register_update(offs_t offset);
static int zeus_fifo_process(const UINT32 *data, int numwords);
static void zeus_draw_model(UINT32 texdata, int logit);
static void zeus_draw_quad(const UINT32 *databuffer, UINT32 texdata, int logit);

static void render_poly_4bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_shade(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_solid(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);
static void render_poly_solid_fixedz(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid);

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix);
static void log_waveram(UINT32 length_and_base);



/*************************************
 *
 *  Inlines
 *
 *************************************/

INLINE UINT64 *waveram_ptr_from_block_addr(UINT8 bank, UINT32 addr)
{
	return &waveram[bank][(addr % WAVERAM_WIDTH) + ((addr >> 12) % WAVERAM_HEIGHT) * WAVERAM_WIDTH];
}


INLINE UINT64 *waveram_ptr_from_expanded_addr(UINT8 bank, UINT32 addr)
{
	return &waveram[bank][(addr % WAVERAM_WIDTH) + ((addr >> 16) % WAVERAM_HEIGHT) * WAVERAM_WIDTH];
}


INLINE UINT64 *waveram_ptr_from_texture_addr(UINT8 bank, UINT32 addr, int width)
{
	return &waveram[bank][((addr & ~1) * width) / 8];
}


INLINE UINT16 *waveram_pixel_ptr(UINT64 *base, int y, int x, UINT16 **depthptr)
{
	UINT16 *baseptr = (UINT16 *)base + y * 512*2 + (x & ~1) * 2;
	if (depthptr != NULL)
		*depthptr = baseptr + BYTE4_XOR_LE((x & 1) + 2);
	return baseptr + BYTE4_XOR_LE(x & 1);
}


INLINE void waveram_plot(int y, int x, UINT16 color)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, y, x, NULL);
		*ptr = color;
	}
}


INLINE void waveram_plot_depth(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT16 *depthptr;
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, y, x, &depthptr);
		*ptr = color;
		*depthptr = depth;
	}
}


INLINE void waveram_plot_check_depth(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT16 *depthptr;
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, y, x, &depthptr);
		if (depth <= *depthptr)
		{
			*ptr = color;
			*depthptr = depth;
		}
	}
}


INLINE void waveram_plot_check_depth_nowrite(int y, int x, UINT16 color, UINT16 depth)
{
	if (x >= 0 && x < 400 && y >= 0 && y < 256)
	{
		UINT16 *depthptr;
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, y, x, &depthptr);
		if (depth <= *depthptr)
			*ptr = color;
	}
}


INLINE UINT8 get_texel_8bit(const UINT64 *base, int y, int x, int width)
{
	const UINT8 *base8 = (const UINT8 *)base;
	base8 += BYTE8_XOR_LE((y / 2) * (width * 2) + (x / 4) * 8 + (y & 1) * 4 + (x & 3));
	return *base8;
}


INLINE UINT8 get_texel_4bit(const UINT64 *base, int y, int x, int width)
{
	const UINT8 *base8 = (const UINT8 *)base;
	base8 += BYTE8_XOR_LE((y / 2) * (width * 2) + (x / 8) * 8 + (y & 1) * 4 + ((x / 2) & 3));
	return (*base8 >> (4 * (x & 1))) & 0x0f;
}


INLINE UINT16 get_palette_entry(const UINT64 *palbase, UINT8 texel)
{
	const UINT16 *base16 = (const UINT16 *)palbase;
	return base16[BYTE4_XOR_LE(texel)];
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( midzeus )
{
	int i;

	/* allocate memory for "wave" RAM */
	waveram[0] = auto_malloc(WAVERAM_WIDTH * WAVERAM_HEIGHT * sizeof(*waveram[0]));
	waveram[1] = auto_malloc(WAVERAM_WIDTH * WAVERAM_HEIGHT * sizeof(*waveram[1]));
	state_save_register_global_pointer(waveram[0], WAVERAM_WIDTH * WAVERAM_HEIGHT);
	state_save_register_global_pointer(waveram[1], WAVERAM_WIDTH * WAVERAM_HEIGHT);

	/* initialize a 5-5-5 palette */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine, i, pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i >> 0));

	/* initialize polygon engine */
	poly = poly_alloc(1000, sizeof(poly_extra_data), POLYFLAG_ALLOW_QUADS);

	/* we need to cleanup on exit */
	add_exit_callback(machine, exit_handler);

	zeus_renderbase = waveram[1];
}


static void exit_handler(running_machine *machine)
{
#if DUMP_WAVE_RAM
	FILE *f = fopen("waveram.dmp", "w");
	int bank;
	int i;

	for (bank = 0; bank < 2; bank++)
		for (i = 0; i < WAVERAM_WIDTH * WAVERAM_HEIGHT; i++)
		{
			if (i % 4 == 0) fprintf(f, "%03X%03X: ", i / WAVERAM_WIDTH, i % WAVERAM_WIDTH);
			fprintf(f, " %08X %08X ", (UINT32)(waveram[bank][i] >> 32), (UINT32)waveram[bank][i]);
			if (i % 4 == 3) fprintf(f, "\n");
		}
	fclose(f);
#endif

	poly_free(poly);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( midzeus )
{
	int x, y;

	/* normal update case */
	if (!input_code_pressed(KEYCODE_W))
	{
		UINT64 *base = waveram_ptr_from_expanded_addr(1, zeusbase[0xcc]);
		int xoffs = machine->screen[screen].visarea.min_x;
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
				dest[x] = *waveram_pixel_ptr(base, y, x - xoffs, NULL);
		}

		if (single_step)
		{
			int texwshift = ((single_step_val2 >> 6) + single_step_dx) & 7;
			int texwidth = 512 >> texwshift;
			int texdepth = (single_step_val2 & 0x20) ? 8 : 4;
			const UINT64 *texbase = waveram_ptr_from_texture_addr(0, single_step_val1, texwidth);
			int x, y;

			for (y = 0; y < 256; y++)
			{
				UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels + 400;
				for (x = 0; x < 256; x++)
				{
					if (x < texwidth)
						dest[x] = get_palette_entry(single_step_extra.palbase, (texdepth == 8) ? get_texel_8bit(texbase, y, x, texwidth) : get_texel_4bit(texbase, y, x, texwidth));
					else
						dest[x] = 0;
				}
			}

			popmessage("val1=%08X  val2=%08X\nwidth=%d  depth=%d", single_step_val1, single_step_val2, texwidth, texdepth);

			if (input_code_pressed(KEYCODE_UP)) { single_step_dy--; while (input_code_pressed(KEYCODE_UP)) ; }
			if (input_code_pressed(KEYCODE_DOWN)) { single_step_dy++; while (input_code_pressed(KEYCODE_DOWN)) ; }
			if (input_code_pressed(KEYCODE_LEFT)) { single_step_dx--; while (input_code_pressed(KEYCODE_LEFT)) ; }
			if (input_code_pressed(KEYCODE_RIGHT)) { single_step_dx++; while (input_code_pressed(KEYCODE_RIGHT)) ; }
			if (input_code_pressed(KEYCODE_LSHIFT)) { single_step_advance = TRUE; while (input_code_pressed(KEYCODE_LSHIFT)) ; }
		}
	}

	/* waveram drawing case */
	else
	{
		static int yoffs = 0;
		static int width = 256;
		const UINT64 *base;

		if (input_code_pressed(KEYCODE_DOWN)) yoffs += input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_UP)) yoffs -= input_code_pressed(KEYCODE_LSHIFT) ? 0x40 : 1;
		if (input_code_pressed(KEYCODE_LEFT) && width > 4) { width >>= 1; while (input_code_pressed(KEYCODE_LEFT)) ; }
		if (input_code_pressed(KEYCODE_RIGHT) && width < 512) { width <<= 1; while (input_code_pressed(KEYCODE_RIGHT)) ; }

		if (yoffs < 0) yoffs = 0;
		base = waveram_ptr_from_block_addr(0, yoffs << 12);

		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
			for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			{
				UINT8 tex = get_texel_8bit(base, y, x, width);
				dest[x] = (tex << 8) | tex;
			}
		}
		popmessage("offs = %06X", yoffs << 12);
	}

	return 0;
}



/*************************************
 *
 *  Core read handler
 *
 *************************************/

READ32_HANDLER( zeus_r )
{
	int logit = (offset < 0xb0 || offset > 0xb7);
	UINT32 result = zeusbase[offset & ~1];

	switch (offset & ~1)
	{
		case 0x00:
			// crusnexo wants bit 0x20 to be non-zero
			result = 0x20;
			break;

		case 0x51:
			// crusnexo expects a reflection of the data at 0x08 here (b425)
			result = zeusbase[0x08];
			break;

		case 0xf0:
			result = video_screen_get_hpos(0);
			logit = 0;
			break;

		case 0xf2:
			result = video_screen_get_vpos(0);
			logit = 0;
			break;

		case 0xf4:
			result = 6;
			if (video_screen_get_vblank(0))
				result |= 0x800;
			logit = 0;
			break;

		case 0xf6:		// status -- they wait for this & 9 == 0
			// value & $9600 must == $9600 to pass Zeus system test
			result = 0x9600;
			if (zeusbase[0xb6] == 0x80040000)
				result |= 1;
			logit = 0;
			break;
	}

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
	{
		if (offset & 1)
			result >>= 16;
		if (logit)
		{
			if (offset & 1)
				logerror("%06X:zeus32_r(%02X) = %08X -- unexpected in 32-bit mode\n", activecpu_get_pc(), offset, result);
			else if (offset != 0xe0)
				logerror("%06X:zeus32_r(%02X) = %08X\n", activecpu_get_pc(), offset, result);
			else
				logerror("%06X:zeus32_r(%02X) = %08X\n", activecpu_get_pc(), offset, result);
		}
	}

	/* 16-bit mode */
	else
	{
		if (offset & 1)
			result >>= 16;
		else
			result &= 0xffff;
		if (logit)
			logerror("%06X:zeus16_r(%02X) = %04X\n", activecpu_get_pc(), offset, result);
	}
	return result;
}



/*************************************
 *
 *  Core write handler
 *
 *************************************/

WRITE32_HANDLER( zeus_w )
{
	int logit = zeus_enable_logging || ((offset < 0xb0 || offset > 0xb7) && (offset < 0xe0 || offset > 0xe1));

	if (logit)
		logerror("%06X:zeus_w", activecpu_get_pc());

	/* 32-bit mode */
	if (zeusbase[0x80] & 0x00020000)
		zeus_register32_w(offset, data, logit);

	/* 16-bit mode */
	else
		zeus_register16_w(offset, data, logit);
}


READ32_HANDLER( zeus2_r )
{
	logerror("%06X:zeus_r(%02X)\n", activecpu_get_pc(), offset * 2);
	return zeus_r(offset * 2, 0);
}


WRITE32_HANDLER( zeus2_w )
{
	logerror("%06X:zeus_w(%02X) = %08X\n", activecpu_get_pc(), offset * 2, data);
}



/*************************************
 *
 *  Handle writes to an internal
 *  pointer register
 *
 *************************************/

static void zeus_pointer_w(UINT32 which, UINT32 data, int logit)
{
	switch (which & 0xffffff)
	{
		case 0x008000:
		case 0x018000:
			if (logit)
				logerror(" -- setptr(objdata)\n");
			zeus_objdata = data;
			break;

		// case 0x00c040: -- set in model data in invasn
		case 0x00c040:
			if (logit)
				logerror(" -- setptr(palbase)\n");
			zeus_palbase = waveram_ptr_from_block_addr(0, data);
			break;


		// case 0x004040: -- set via FIFO command in mk4 (len=02)

		// case 0x02c0f0: -- set in model data in mk4 (len=0f)
		// case 0x03c0f0: -- set via FIFO command in mk4 (len=00)

		// case 0x02c0e7: -- set via FIFO command in mk4 (len=08)

		// case 0x04c09c: -- set via FIFO command in mk4 (len=08)

		// case 0x05c0a5: -- set via FIFO command in mk4 (len=21)
		// case 0x80c0a5: -- set via FIFO command in mk4 (len=3f)
		// case 0x81c0a5: -- set via FIFO command in mk4 (len=35)
		// case 0x82c0a5: -- set via FIFO command in mk4 (len=41)


		// case 0x00c0f0: -- set via FIFO command in invasn (len=0f)

		// case 0x00c0b0: -- set via FIFO command in invasn (len=3f) -- seems to be the same as c0a5
		// case 0x05c0b0: -- set via FIFO command in invasn (len=21)

		// case 0x00c09c: -- set via FIFO command in invasn (len=06)

		// case 0x00c0a3: -- set via FIFO command in invasn (len=0a)


		default:
			if (logit)
				logerror(" -- setptr(%06X)\n", which & 0xffffff);
			break;
	}

	if (logit)
		log_waveram(data);
}



/*************************************
 *
 *  Handle register writes
 *
 *************************************/

static void zeus_register16_w(offs_t offset, UINT16 data, int logit)
{
	/* writes to register $CC need to force a partial update */
	if ((offset & ~1) == 0xcc)
		video_screen_update_partial(0, video_screen_get_vpos(0));

	/* write to high part on odd addresses */
	if (offset & 1)
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0x0000ffff) | (data << 16);

	/* write to low part on event addresses */
	else
		zeusbase[offset & ~1] = (zeusbase[offset & ~1] & 0xffff0000) | (data & 0xffff);

	/* log appropriately */
	if (logit)
		logerror("(%02X) = %04X [%08X]\n", offset, data & 0xffff, zeusbase[offset & ~1]);

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_register_update(offset);
}


static void zeus_register32_w(offs_t offset, UINT32 data, int logit)
{
	/* writes to register $CC need to force a partial update */
	if ((offset & ~1) == 0xcc)
		video_screen_update_partial(0, video_screen_get_vpos(0));

	/* always write to low word? */
	zeusbase[offset & ~1] = data;

	/* log appropriately */
	if (logit)
	{
		if (offset & 1)
			logerror("(%02X) = %08X -- unexpected in 32-bit mode\n", offset, data);
		else if (offset != 0xe0)
			logerror("(%02X) = %08X\n", offset, data);
		else
			logerror("(%02X) = %08X\n", offset, data);
	}

	/* handle the update */
	if ((offset & 1) == 0)
		zeus_register_update(offset);
}



/*************************************
 *
 *  Update state after a register write
 *
 *************************************/

static void zeus_register_update(offs_t offset)
{
	/* handle the writes; only trigger on low accesses */
	switch (offset)
	{
		case 0x52:
			zeusbase[0xb2] = zeusbase[0x52];
			break;

		case 0x60:
			/* invasn writes here to execute a command (?) */
			if (zeusbase[0x60] & 1)
			{
				if ((zeusbase[0x80] & 0xffffff) == 0x22FCFF)
				{
					// zeusbase[0x00] = color
					// zeusbase[0x02] = ??? = 0x000C0000
					// zeusbase[0x04] = ??? = 0x00000E01
					// zeusbase[0x06] = ??? = 0xFFFF0030
					// zeusbase[0x08] = vert[0] = (y0 << 16) | x0
					// zeusbase[0x0a] = vert[1] = (y1 << 16) | x1
					// zeusbase[0x0c] = vert[2] = (y2 << 16) | x2
					// zeusbase[0x0e] = vert[3] = (y3 << 16) | x3
					// zeusbase[0x18] = ??? = 0xFFFFFFFF
					// zeusbase[0x1a] = ??? = 0xFFFFFFFF
					// zeusbase[0x1c] = ??? = 0xFFFFFFFF
					// zeusbase[0x1e] = ??? = 0xFFFFFFFF
					// zeusbase[0x20] = ??? = 0x00000000
					// zeusbase[0x22] = ??? = 0x00000000
					// zeusbase[0x24] = ??? = 0x00000000
					// zeusbase[0x26] = ??? = 0x00000000
					// zeusbase[0x40] = ??? = 0x00000000
					// zeusbase[0x42] = ??? = 0x00000000
					// zeusbase[0x44] = ??? = 0x00000000
					// zeusbase[0x46] = ??? = 0x00000000
					// zeusbase[0x4c] = ??? = 0x00808080 (brightness?)
					// zeusbase[0x4e] = ??? = 0x00808080 (brightness?)
					poly_extra_data *extra = poly_get_extra_data(poly);
					poly_vertex vert[4];

					vert[0].x = (INT16)zeusbase[0x08];
					vert[0].y = (INT16)(zeusbase[0x08] >> 16);
					vert[1].x = (INT16)zeusbase[0x0a];
					vert[1].y = (INT16)(zeusbase[0x0a] >> 16);
					vert[2].x = (INT16)zeusbase[0x0c];
					vert[2].y = (INT16)(zeusbase[0x0c] >> 16);
					vert[3].x = (INT16)zeusbase[0x0e];
					vert[3].y = (INT16)(zeusbase[0x0e] >> 16);

					extra->solidcolor = zeusbase[0x00];
					extra->zoffset = 0x7fff;

					poly_render_quad(poly, NULL, &zeus_cliprect, render_poly_solid_fixedz, 0, &vert[0], &vert[1], &vert[2], &vert[3]);
					poly_wait(poly, "Normal");
				}
				else
					logerror("Execute unknown command\n");
			}
			break;

		case 0x70:
			zeus_point[0] = zeusbase[0x70] << 16;
			break;

		case 0x72:
			zeus_point[1] = zeusbase[0x72] << 16;
			break;

		case 0x74:
			zeus_point[2] = zeusbase[0x74] << 16;
			break;

		case 0x80:
			/* this bit enables the "FIFO empty" IRQ; since our virtual FIFO is always empty,
                we simply assert immediately if this is enabled. invasn needs this for proper
                operations */
			if (zeusbase[0x80] & 0x02000000)
				cpunum_set_input_line(0, 2, ASSERT_LINE);
			else
				cpunum_set_input_line(0, 2, CLEAR_LINE);
			break;

		case 0x84:
			/* MK4: Written in tandem with 0xcc */
			/* MK4: Writes either 0x80 (and 0x000000 to 0xcc) or 0x00 (and 0x800000 to 0xcc) */
			zeus_renderbase = waveram_ptr_from_expanded_addr(1, zeusbase[0x84] << 16);
			break;

		case 0xb0:
		case 0xb2:
			if ((zeusbase[0xb6] >> 16) != 0)
			{
				if ((offset == 0xb0 && (zeusbase[0xb6] & 0x02000000) == 0) ||
					(offset == 0xb2 && (zeusbase[0xb6] & 0x02000000) != 0))
				{
					UINT16 *dest = (UINT16 *)waveram_ptr_from_expanded_addr(zeusbase[0xb6] >> 31, zeusbase[0xb4]);
					if (zeusbase[0xb6] & 0x00100000)
						dest[BYTE4_XOR_LE(0)] = zeusbase[0xb0];
					if (zeusbase[0xb6] & 0x00200000)
						dest[BYTE4_XOR_LE(1)] = zeusbase[0xb0] >> 16;
					if (zeusbase[0xb6] & 0x00400000)
						dest[BYTE4_XOR_LE(2)] = zeusbase[0xb2];
					if (zeusbase[0xb6] & 0x00800000)
						dest[BYTE4_XOR_LE(3)] = zeusbase[0xb2] >> 16;
					if (zeusbase[0xb6] & 0x00020000)
						zeusbase[0xb4]++;
				}
			}
			break;

		case 0xb4:
			if (zeusbase[0xb6] & 0x00010000)
			{
				const UINT64 *src = waveram_ptr_from_expanded_addr(zeusbase[0xb6] >> 31, zeusbase[0xb4]);
				zeusbase[0xb0] = *src;
				zeusbase[0xb2] = *src >> 32;
			}
			break;

		case 0xc0:
		case 0xc2:
		case 0xc4:
		case 0xc6:
		case 0xc8:
		case 0xca:
			video_screen_update_partial(0, video_screen_get_vpos(0));
			{
				int vtotal = zeusbase[0xca] >> 16;
				int htotal = zeusbase[0xc6] >> 16;
				rectangle visarea;

				visarea.min_x = zeusbase[0xc6] & 0xffff;
				visarea.max_x = htotal - 1;
				visarea.min_y = 0;
				visarea.max_y = zeusbase[0xc8] & 0xffff;
				if (htotal > 0 && vtotal > 0 && visarea.min_x < visarea.max_x && visarea.max_y < vtotal)
				{
					video_screen_configure(0, htotal, vtotal, &visarea, HZ_TO_ATTOSECONDS((double)MIDZEUS_VIDEO_CLOCK / 8.0 / (htotal * vtotal)));
					zeus_cliprect = visarea;
					zeus_cliprect.max_x -= zeus_cliprect.min_x;
					zeus_cliprect.min_x = 0;
				}
			}
			break;

		case 0xcc:
			video_screen_update_partial(0, video_screen_get_vpos(0));

			log_fifo = input_code_pressed(KEYCODE_L);
//          single_step = input_code_pressed(KEYCODE_S);
			if (single_step)
				popmessage("Single Step");
			break;

		case 0xe0:
			zeus_fifo[zeus_fifo_words++] = zeusbase[0xe0];
			if (zeus_fifo_process(zeus_fifo, zeus_fifo_words))
				zeus_fifo_words = 0;
			break;
	}
}



/*************************************
 *
 *  Process the FIFO
 *
 *************************************/

static int zeus_fifo_process(const UINT32 *data, int numwords)
{
	/* handle logging */
	switch (data[0] >> 24)
	{
		/* 0x00/0x01: set pointer */
		/* in model data, this is 0x0C */
		case 0x00:
		case 0x01:
			if (numwords < 2 && data[0] != 0)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_pointer_w(data[0] & 0xffffff, data[1], log_fifo);
			break;

		/* 0x13: render model based on previously set information */
		case 0x13:	/* invasn */
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_draw_model((zeusbase[0x06] << 16), log_fifo);
			break;

		/* 0x17: write 16-bit value to low registers */
		case 0x17:
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg16");
			zeus_register16_w((data[0] >> 16) & 0x7f, data[0], log_fifo);
			break;

		/* 0x18: write 32-bit value to low registers */
		/* in model data, this is 0x19 */
		case 0x18:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- reg32");
			zeus_register32_w((data[0] >> 16) & 0x7f, data[1], log_fifo);
			break;

		/* 0x1A: sync pipeline(?) */
		case 0x1a:
			if (log_fifo)
				log_fifo_command(data, numwords, " -- sync\n");
			break;

		/* 0x1C: write matrix and translation vector */
		case 0x1c:
			if ((data[0] & 0xffff) != 0x7fff)
			{
				if (numwords < 8)
					return FALSE;
				if (log_fifo)
				{
					log_fifo_command(data, numwords, "");
					logerror("\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tvector %8.2f %8.2f %8.5f\n",
						data[2] & 0xffff,
						data[2] >> 16,
						data[0] & 0xffff,
						data[3] & 0xffff,
						data[3] >> 16,
						data[1] >> 16,
						data[4] & 0xffff,
						data[4] >> 16,
						data[1] & 0xffff,
						(float)(INT32)data[5] * (1.0f / 65536.0f),
						(float)(INT32)data[6] * (1.0f / 65536.0f),
						(float)(INT32)data[7] * (1.0f / (65536.0f * 512.0f)));
				}

				zeus_matrix[0][0] = data[2];
				zeus_matrix[0][1] = data[2] >> 16;
				zeus_matrix[0][2] = data[0];
				zeus_matrix[1][0] = data[3];
				zeus_matrix[1][1] = data[3] >> 16;
				zeus_matrix[1][2] = data[1] >> 16;
				zeus_matrix[2][0] = data[4];
				zeus_matrix[2][1] = data[4] >> 16;
				zeus_matrix[2][2] = data[1];

				zeus_point[0] = data[5];
				zeus_point[1] = data[6];
				zeus_point[2] = data[7];
			}
			else
			{
				INT16 matrix1[3][3];
				INT16 matrix2[3][3];
				if (numwords < 13)
					return FALSE;
				if (log_fifo)
				{
					log_fifo_command(data, numwords, "");
					logerror("\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tmatrix ( %04X %04X %04X ) ( %04X %04X %04X ) ( %04X %04X %04X )\n\t\tvector %8.2f %8.2f %8.5f\n",
						data[3] & 0xffff,
						data[3] >> 16,
						data[7] & 0xffff,
						data[5] & 0xffff,
						data[6] & 0xffff,
						data[2] >> 16,
						data[1] & 0xffff,
						data[1] >> 16,
						data[2] & 0xffff,
						data[4] & 0xffff,
						data[4] >> 16,
						data[5] >> 16,
						data[8] & 0xffff,
						data[8] >> 16,
						data[6] >> 16,
						data[9] & 0xffff,
						data[9] >> 16,
						data[7] >> 16,
						(float)(INT32)data[10] * (1.0f / 65536.0f),
						(float)(INT32)data[11] * (1.0f / 65536.0f),
						(float)(INT32)data[12] * (1.0f / (65536.0f * 512.0f)));
				}

				matrix1[0][0] = data[4];
				matrix1[0][1] = data[4] >> 16;
				matrix1[0][2] = data[5] >> 16;
				matrix1[1][0] = data[8];
				matrix1[1][1] = data[8] >> 16;
				matrix1[1][2] = data[6] >> 16;
				matrix1[2][0] = data[9];
				matrix1[2][1] = data[9] >> 16;
				matrix1[2][2] = data[7] >> 16;
				
				matrix2[0][0] = data[1];
				matrix2[0][1] = data[2];
				matrix2[0][2] = data[3];
				matrix2[1][0] = data[1] >> 16;
				matrix2[1][1] = data[2] >> 16;
				matrix2[1][2] = data[3] >> 16;
				matrix2[2][0] = data[5];
				matrix2[2][1] = data[6];
				matrix2[2][2] = data[7];

				zeus_matrix[0][0] = ((INT64)(matrix1[0][0] * matrix2[0][0]) + (INT64)(matrix1[0][1] * matrix2[1][0]) + (INT64)(matrix1[0][2] * matrix2[2][0])) >> 16;
				zeus_matrix[0][1] = ((INT64)(matrix1[0][0] * matrix2[0][1]) + (INT64)(matrix1[0][1] * matrix2[1][1]) + (INT64)(matrix1[0][2] * matrix2[2][1])) >> 16;
				zeus_matrix[0][2] = ((INT64)(matrix1[0][0] * matrix2[0][2]) + (INT64)(matrix1[0][1] * matrix2[1][2]) + (INT64)(matrix1[0][2] * matrix2[2][2])) >> 16;
				zeus_matrix[1][0] = ((INT64)(matrix1[1][0] * matrix2[0][0]) + (INT64)(matrix1[1][1] * matrix2[1][0]) + (INT64)(matrix1[1][2] * matrix2[2][0])) >> 16;
				zeus_matrix[1][1] = ((INT64)(matrix1[1][0] * matrix2[0][1]) + (INT64)(matrix1[1][1] * matrix2[1][1]) + (INT64)(matrix1[1][2] * matrix2[2][1])) >> 16;
				zeus_matrix[1][2] = ((INT64)(matrix1[1][0] * matrix2[0][2]) + (INT64)(matrix1[1][1] * matrix2[1][2]) + (INT64)(matrix1[1][2] * matrix2[2][2])) >> 16;
				zeus_matrix[2][0] = ((INT64)(matrix1[2][0] * matrix2[0][0]) + (INT64)(matrix1[2][1] * matrix2[1][0]) + (INT64)(matrix1[2][2] * matrix2[2][0])) >> 16;
				zeus_matrix[2][1] = ((INT64)(matrix1[2][0] * matrix2[0][1]) + (INT64)(matrix1[2][1] * matrix2[1][1]) + (INT64)(matrix1[2][2] * matrix2[2][1])) >> 16;
				zeus_matrix[2][2] = ((INT64)(matrix1[2][0] * matrix2[0][2]) + (INT64)(matrix1[2][1] * matrix2[1][2]) + (INT64)(matrix1[2][2] * matrix2[2][2])) >> 16;

				zeus_point[0] = data[10];
				zeus_point[1] = data[11];
				zeus_point[2] = data[12];
			}
			break;

		/* 0x23: some additional X,Y,Z coordinates */
		/* 0x2e: same for invasn */
		case 0x23:
		case 0x2e:
			if (numwords < 2)
				return FALSE;
			if (log_fifo)
			{
				log_fifo_command(data, numwords, "");
				logerror(" -- additional xyz = %d,%d,%d\n", (INT16)data[0], (INT16)(data[1] >> 16), (INT16)data[1]);
				zeus_light[0] = (INT16)data[0];
				zeus_light[1] = (INT16)(data[1] >> 16);
				zeus_light[2] = (INT16)data[1];
			}
			break;

		/* 0x25: display control? */
		/* 0x30: same for invasn */
		case 0x25:
		case 0x30:
			if (numwords < 4 || ((data[0] & 0x808000) && numwords < 10))
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, " -- unknown control + hack clear screen\n");
			if ((data[0] & 0xffff7f) == 0)
			{
				/* not right -- just a hack */
				int x, y;
				for (y = zeus_cliprect.min_y; y <= zeus_cliprect.max_y; y++)
					for (x = zeus_cliprect.min_x; x <= zeus_cliprect.max_x; x++)
						waveram_plot_depth(y, x, 0, 0x7fff);
			}
			break;

		/* 0x2d: unknown - invasn */
		/* 0x70: same for mk4 */
		case 0x2d:
		case 0x70:
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;

		/* 0x67: render model with inline texture info */
		case 0x67:
			if (numwords < 3)
				return FALSE;
			if (log_fifo)
				log_fifo_command(data, numwords, "");
			zeus_objdata = data[1];
			zeus_draw_model(data[2], log_fifo);
			break;

		default:
			printf("Unknown command %08X\n", data[0]);
			if (log_fifo)
				log_fifo_command(data, numwords, "\n");
			break;
	}
	return TRUE;
}



/*************************************
 *
 *  Draw a model in waveram
 *
 *************************************/

static void zeus_draw_model(UINT32 texdata, int logit)
{
	UINT32 databuffer[32];
	int databufcount = 0;
	int model_done = FALSE;

	if (logit)
		logerror(" -- model @ %08X\n", zeus_objdata);

	while (zeus_objdata != 0 && !model_done)
	{
		const UINT64 *data = waveram_ptr_from_block_addr(0, zeus_objdata);
		int count = (zeus_objdata >> 24) + 1;

		/* reset the objdata address */
		zeus_objdata = 0;

		/* loop until we run out of data */
		while (count--)
		{
			int countneeded;
			UINT8 cmd;

			/* accumulate 2 words of data */
			databuffer[databufcount++] = *data;
			databuffer[databufcount++] = *data++ >> 32;

			/* if this is enough, process the command */
			cmd = databuffer[0] >> 24;
			countneeded = (cmd == 0x25 || cmd == 0x30) ? 14 : 2;
			if (databufcount == countneeded)
			{
				/* handle logging of the command */
				if (logit)
				{
					int offs;
					logerror("\t");
					for (offs = 0; offs < databufcount; offs++)
						logerror("%08X ", databuffer[offs]);
					logerror("-- ");
				}

				/* handle the command */
				switch (cmd)
				{
					case 0x08:
						if (logit)
							logerror("end of model\n");
						model_done = TRUE;
						break;

					case 0x0c:	/* mk4/invasn */
						zeus_pointer_w(databuffer[0] & 0xffffff, databuffer[1], logit);
						break;

					case 0x17:	/* mk4 */
						if (logit)
							logerror("reg16");
						zeus_register16_w((databuffer[0] >> 16) & 0x7f, databuffer[0], logit);
						if (((databuffer[0] >> 16) & 0x7f) == 0x06)
							texdata = (texdata & 0xffff) | (zeusbase[0x06] << 16);
						break;

					case 0x19:	/* invasn */
						if (logit)
							logerror("reg32");
						zeus_register32_w((databuffer[0] >> 16) & 0x7f, databuffer[1], logit);
						if (((databuffer[0] >> 16) & 0x7f) == 0x06)
							texdata = (texdata & 0xffff) | (zeusbase[0x06] << 16);
						break;

					case 0x25:	/* mk4 */
					case 0x30:	/* invasn */
						zeus_draw_quad(databuffer, texdata, logit);
						break;

					default:
						if (logit)
							logerror("unknown\n");
						break;
				}

				/* reset the count */
				databufcount = 0;
			}
		}
	}
}



/*************************************
 *
 *  Draw a quad
 *
 *************************************/

static void zeus_draw_quad(const UINT32 *databuffer, UINT32 texdata, int logit)
{
	poly_draw_scanline callback;
	poly_extra_data *extra;
	poly_vertex clipvert[8];
	poly_vertex vert[4];
	float uscale, vscale;
	int val1, val2, texwshift;
	int numverts;
	int i;
	INT16 normal[3];
	INT32 rotnormal[3];

/* look for interesting data patterns  */
if (
	(databuffer[1] & 0xffffffff) != 0x200c0000 && /* mk4 sometimes */
	(databuffer[1] & 0xfffe0000) != 0x21000000 && /* most of mk4 */
	(databuffer[1] & 0xffffffff) != 0x008c0000 && /* invasn */
	(databuffer[1] & 0xfffeffff) != 0x028c0000 && /* invasn */
	(databuffer[1] & 0xfffe0000) != 0x21800000 && /* invasn */
	(databuffer[1] & 0xfffe0000) != 0x23800000 && /* invasn */
	1)
	printf("zeus_draw_quad: databuffer[1] = %08X\n", databuffer[1]);

	
	/* do a simple backface cull; not sure if the hardware does it, but I see no other
	   reason for a polygon normal here */
	
	/* extract the polygon normal */
	normal[0] = (INT8)(databuffer[0] >> 0);
	normal[1] = (INT8)(databuffer[0] >> 8);
	normal[2] = (INT8)(databuffer[0] >> 16);

	/* rotate the normal into camera view; we only need the Z coordinate */
	rotnormal[2] = normal[0] * zeus_matrix[2][0] + normal[1] * zeus_matrix[2][1] + normal[2] * zeus_matrix[2][2];

	/* if we're pointing away from the camera, toss */
	if (rotnormal[2] > 0)
	{
		if (logit)
			logerror("quad (culled %08X)\n", rotnormal[2]);
//		return;
	}

	if (logit)
		logerror("quad\n");

	texdata = (texdata & 0xffff0000) | ((texdata + databuffer[1]) & 0xffff);

	val1 = ((texdata >> 10) & 0x3f0000) | (texdata & 0xffff);
	val2 = (texdata >> 16) & 0x3ff;
	texwshift = (val2 >> 6) & 7;

	uscale = (8 >> ((zeusbase[0x04] >> 4) & 3)) * 0.125f * 256.0f;
	vscale = (8 >> ((zeusbase[0x04] >> 6) & 3)) * 0.125f * 256.0f;

	if ((databuffer[1] & 0x000c0000) == 0x000c0000)
		callback = render_poly_solid;
	else if (val2 == 0x182)
		callback = render_poly_shade;
	else if (val2 & 0x20)
		callback = render_poly_8bit;
	else
		callback = render_poly_4bit;

	for (i = 0; i < 4; i++)
	{
		UINT32 ixy = databuffer[2 + i*2];
		UINT32 iuvz = databuffer[3 + i*2];
		UINT32 inormal = databuffer[10 + i];
		INT32 xo = (INT16)ixy;
		INT32 yo = (INT16)(ixy >> 16);
		INT32 zo = (INT16)iuvz;
		INT32 xn = (INT32)(inormal << 22) >> 20;
		INT32 yn = (INT32)(inormal << 12) >> 20;
		INT32 zn = (INT32)(inormal <<  2) >> 20;
		UINT8 u = iuvz >> 16;
		UINT8 v = iuvz >> 24;
		INT32 dotnormal;
		INT64 x, y, z;
		
		x = (INT64)(xo * zeus_matrix[0][0]) + (INT64)(yo * zeus_matrix[0][1]) + (INT64)(zo * zeus_matrix[0][2]) + zeus_point[0];
		y = (INT64)(xo * zeus_matrix[1][0]) + (INT64)(yo * zeus_matrix[1][1]) + (INT64)(zo * zeus_matrix[1][2]) + zeus_point[1];
		z = (INT64)(xo * zeus_matrix[2][0]) + (INT64)(yo * zeus_matrix[2][1]) + (INT64)(zo * zeus_matrix[2][2]) + zeus_point[2];

		rotnormal[0] = ((INT64)(xn * zeus_matrix[0][0]) + (INT64)(yn * zeus_matrix[0][1]) + (INT64)(zn * zeus_matrix[0][2])) >> 14;
		rotnormal[1] = ((INT64)(xn * zeus_matrix[1][0]) + (INT64)(yn * zeus_matrix[1][1]) + (INT64)(zn * zeus_matrix[1][2])) >> 14;
		rotnormal[2] = ((INT64)(xn * zeus_matrix[2][0]) + (INT64)(yn * zeus_matrix[2][1]) + (INT64)(zn * zeus_matrix[2][2])) >> 14;

		dotnormal = rotnormal[0] * ((x >> 16) + zeus_light[0]) + rotnormal[1] * ((y >> 16) + zeus_light[1]) + rotnormal[2] * ((z >> 16) + zeus_light[2]);

		vert[i].x = x;
		vert[i].y = y;
		vert[i].p[0] = z;
		vert[i].p[1] = u * uscale;
		vert[i].p[2] = v * vscale;
		vert[i].p[3] = dotnormal;

		if (logit)
		{
			logerror("\t\t(%f,%f,%f) (%02X,%02X) (%03X,%03X,%03X) dot=%08X\n",
					vert[i].x * (1.0f / 65536.0f), vert[i].y * (1.0f / 65536.0f), vert[i].p[0] * (1.0f / 65536.0f),
					(int)(vert[i].p[1] / 256.0f), (int)(vert[i].p[2] / 256.0f),
					(databuffer[10 + i] >> 20) & 0x3ff, (databuffer[10 + i] >> 10) & 0x3ff, (databuffer[10 + i] >> 0) & 0x3ff,
					dotnormal);
		}
	}

	numverts = poly_zclip_if_less(4, &vert[0], &clipvert[0], 4, 512.0f);
	if (numverts < 3)
		return;

	for (i = 0; i < numverts; i++)
	{
		float ooz = 512.0f / clipvert[i].p[0];

		clipvert[i].x *= ooz;
		clipvert[i].y *= ooz;
		clipvert[i].x += 200.0f;
		clipvert[i].y += 128.0f;
	}

	extra = poly_get_extra_data(poly);
	extra->solidcolor = zeusbase[0x00] & 0x7fff;
	extra->zoffset = zeusbase[0x7e] >> 16;
	extra->alpha = zeusbase[0x4e];
	extra->transcolor = ((databuffer[1] >> 16) & 1) ? 0 : 0x100;
	extra->texwidth = 512 >> texwshift;
	extra->texbase = waveram_ptr_from_texture_addr(0, val1, extra->texwidth);
	extra->palbase = zeus_palbase;

	if (single_step)
	{
		single_step_extra = *extra;
		single_step_val1 = val1;
		single_step_val2 = val2;
		single_step_dx = single_step_dy = 0;
		single_step_advance = FALSE;
	}

	poly_render_quad_fan(poly, NULL, &zeus_cliprect, callback, 4, numverts, &clipvert[0]);
	poly_wait(poly, "Normal");

	if (single_step)
		while (!single_step_advance) video_frame_update(TRUE);
}



/*************************************
 *
 *  Rasterizers
 *
 *************************************/

static void render_poly_4bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	INT32 curz = extent->param[0].start;
	INT32 curu = extent->param[1].start;
	INT32 curv = extent->param[2].start;
	INT32 curi = extent->param[3].start;
	INT32 dzdx = extent->param[0].dpdx;
	INT32 dudx = extent->param[1].dpdx;
	INT32 dvdx = extent->param[2].dpdx;
	INT32 didx = extent->param[3].dpdx;
	const UINT64 *texbase = extra->texbase;
	const UINT64 *palbase = extra->palbase;
	UINT16 transcolor = extra->transcolor;
	int texwidth = extra->texwidth;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT16 *depthptr;
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, scanline, x, &depthptr);
		INT32 depth = (curz >> 16) + extra->zoffset;
		if (depth > 0x7fff) depth = 0x7fff;
		if (depth >= 0 && depth <= *depthptr)
		{
			int u0 = (curu >> 8);// & (texwidth - 1);
			int v0 = (curv >> 8);// & 255;
			int u1 = (u0 + 1);
			int v1 = (v0 + 1);
			UINT8 texel0 = get_texel_4bit(texbase, v0, u0, texwidth);
			UINT8 texel1 = get_texel_4bit(texbase, v0, u1, texwidth);
			UINT8 texel2 = get_texel_4bit(texbase, v1, u0, texwidth);
			UINT8 texel3 = get_texel_4bit(texbase, v1, u1, texwidth);
			if (texel0 != transcolor)
			{
				rgb_t color0 = get_palette_entry(palbase, texel0);
				rgb_t color1 = get_palette_entry(palbase, texel1);
				rgb_t color2 = get_palette_entry(palbase, texel2);
				rgb_t color3 = get_palette_entry(palbase, texel3);
				rgb_t filtered;
				color0 = ((color0 & 0x7fe0) << 6) | (color0 & 0x1f);
				color1 = ((color1 & 0x7fe0) << 6) | (color1 & 0x1f);
				color2 = ((color2 & 0x7fe0) << 6) | (color2 & 0x1f);
				color3 = ((color3 & 0x7fe0) << 6) | (color3 & 0x1f);
				filtered = rgb_bilinear_filter(color0, color1, color2, color3, curu, curv);
				*ptr = ((filtered >> 6) & 0x7fe0) | (filtered & 0x1f);
				*depthptr = depth;
			}
		}

		curz += dzdx;
		curu += dudx;
		curv += dvdx;
		curi += didx;
	}
}


static void render_poly_8bit(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	INT32 curz = extent->param[0].start;
	INT32 curu = extent->param[1].start;
	INT32 curv = extent->param[2].start;
	INT32 curi = extent->param[3].start;
	INT32 dzdx = extent->param[0].dpdx;
	INT32 dudx = extent->param[1].dpdx;
	INT32 dvdx = extent->param[2].dpdx;
	INT32 didx = extent->param[3].dpdx;
	const UINT64 *texbase = extra->texbase;
	const UINT64 *palbase = extra->palbase;
	UINT16 transcolor = extra->transcolor;
	int texwidth = extra->texwidth;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT16 *depthptr;
		UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, scanline, x, &depthptr);
		INT32 depth = (curz >> 16) + extra->zoffset;
		if (depth > 0x7fff) depth = 0x7fff;
		if (depth >= 0 && depth <= *depthptr)
		{
			int u0 = (curu >> 8);// & (texwidth - 1);
			int v0 = (curv >> 8);// & 255;
			int u1 = (u0 + 1);
			int v1 = (v0 + 1);
			UINT8 texel0 = get_texel_8bit(texbase, v0, u0, texwidth);
			UINT8 texel1 = get_texel_8bit(texbase, v0, u1, texwidth);
			UINT8 texel2 = get_texel_8bit(texbase, v1, u0, texwidth);
			UINT8 texel3 = get_texel_8bit(texbase, v1, u1, texwidth);
			if (texel0 != transcolor)
			{
				rgb_t color0 = get_palette_entry(palbase, texel0);
				rgb_t color1 = get_palette_entry(palbase, texel1);
				rgb_t color2 = get_palette_entry(palbase, texel2);
				rgb_t color3 = get_palette_entry(palbase, texel3);
				rgb_t filtered;
				color0 = ((color0 & 0x7fe0) << 6) | (color0 & 0x1f);
				color1 = ((color1 & 0x7fe0) << 6) | (color1 & 0x1f);
				color2 = ((color2 & 0x7fe0) << 6) | (color2 & 0x1f);
				color3 = ((color3 & 0x7fe0) << 6) | (color3 & 0x1f);
				filtered = rgb_bilinear_filter(color0, color1, color2, color3, curu, curv);
				*ptr = ((filtered >> 6) & 0x7fe0) | (filtered & 0x1f);
				*depthptr = depth;
			}
		}

		curz += dzdx;
		curu += dudx;
		curv += dvdx;
		curi += didx;
	}
}


static void render_poly_shade(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		if (x >= 0 && x < 400)
		{
			if (extra->alpha <= 0x80)
			{
				UINT16 *ptr = waveram_pixel_ptr(zeus_renderbase, scanline, x, NULL);
				UINT16 pix = *ptr;

				*ptr = ((((pix & 0x7c00) * extra->alpha) >> 7) & 0x7c00) |
					  ((((pix & 0x03e0) * extra->alpha) >> 7) & 0x03e0) |
					  ((((pix & 0x001f) * extra->alpha) >> 7) & 0x001f);
			}
			else
			{
				waveram_plot(scanline, x, 0);
			}
		}
	}
}


static void render_poly_solid(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	UINT16 color = extra->solidcolor;
	INT32 curz = (INT32)(extent->param[0].start);
	INT32 curv = extent->param[2].start;
	INT32 dzdx = (INT32)(extent->param[0].dpdx);
	INT32 dvdx = extent->param[2].dpdx;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		INT32 depth = (curz >> 16) + extra->zoffset;
		if (depth > 0x7fff) depth = 0x7fff;
		if (depth >= 0)
		{
//			UINT32 finalcolor = (((color & 0x7c00) * curv) & 0x7c000000) | (((color & 0x03e0) * curv) & 0x03e00000) | (((color & 0x001f) * curv) & 0x001f0000);
//			waveram_plot_check_depth(scanline, x, finalcolor >> 16, depth);
			waveram_plot_check_depth(scanline, x, color, depth);
		}
		curz += dzdx;
		curv += dvdx;
	}
}


static void render_poly_solid_fixedz(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = extradata;
	UINT16 color = extra->solidcolor;
	UINT16 depth = extra->zoffset;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
		waveram_plot_depth(scanline, x, color, depth);
}



/*************************************
 *
 *  Debugging tools
 *
 *************************************/

static void log_fifo_command(const UINT32 *data, int numwords, const char *suffix)
{
	int wordnum;

	logerror("Zeus cmd %02X :", data[0] >> 24);
	for (wordnum = 0; wordnum < numwords; wordnum++)
		logerror(" %08X", data[wordnum]);
	logerror("%s", suffix);
}


static void log_waveram(UINT32 length_and_base)
{
	static struct
	{
		UINT32 lab;
		UINT64 checksum;
	} recent_entries[100];

	UINT32 numoctets = (length_and_base >> 24) + 1;
	const UINT64 *ptr = waveram_ptr_from_block_addr(0, length_and_base);
	UINT64 checksum = length_and_base;
	int foundit = FALSE;
	int i;

	for (i = 0; i < numoctets; i++)
		checksum += ptr[i];

	for (i = 0; i < ARRAY_LENGTH(recent_entries); i++)
		if (recent_entries[i].lab == length_and_base && recent_entries[i].checksum == checksum)
		{
			foundit = TRUE;
			break;
		}

	if (i == ARRAY_LENGTH(recent_entries))
		i--;
	if (i != 0)
	{
		memmove(&recent_entries[1], &recent_entries[0], i * sizeof(recent_entries[0]));
		recent_entries[0].lab = length_and_base;
		recent_entries[0].checksum = checksum;
	}
	if (foundit)
		return;

	for (i = 0; i < numoctets; i++)
		logerror("\t%02X: %08X %08X\n", i, (UINT32)ptr[i], (UINT32)(ptr[i] >> 32));
}
