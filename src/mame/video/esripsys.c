/***************************************************************************

    Entertainment Sciences Real-Time Image Processor (RIP) video hardware

****************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "esripsys.h"

typedef struct _pixel_
{
	UINT8 colour;
	UINT8 intensity;
	UINT8 priority;
} pixel;

static emu_timer *hblank_end_timer;
static emu_timer *hblank_start_timer;

static UINT8 *fig_scale_table;
static UINT8 *scale_table;
static pixel *line_buffer_1;
static pixel *line_buffer_2;

static int vcount;
static int video_firq;
static UINT8 bg_intensity;

int esripsys_video_firq_en;
int esripsys_frame_vbl;
int esripsys__12sel;
UINT8 *esripsys_pal_ram;


INTERRUPT_GEN( esripsys_vblank_irq )
{
	cpu_set_input_line(device->machine->cpu[ESRIPSYS_GAME_CPU], M6809_IRQ_LINE, ASSERT_LINE);
	esripsys_frame_vbl = 0;
}

static TIMER_CALLBACK( hblank_start_callback )
{
	int v = vcount;

	if (video_firq)
	{
		video_firq = 0;
		cpu_set_input_line(machine->cpu[ESRIPSYS_GAME_CPU], M6809_FIRQ_LINE, CLEAR_LINE);
	}

	if (!(vcount % 6) && vcount && esripsys_video_firq_en && vcount < ESRIPSYS_VBLANK_START)
	{
		video_firq = 1;
		cpu_set_input_line(machine->cpu[ESRIPSYS_GAME_CPU], M6809_FIRQ_LINE, ASSERT_LINE);
	}

	/* Adjust for next scanline */
	if (++vcount >= ESRIPSYS_VTOTAL)
		vcount = 0;

	/* Set end of HBLANK timer */
	timer_adjust_oneshot(hblank_end_timer, video_screen_get_time_until_pos(machine->primary_screen, v, ESRIPSYS_HTOTAL-1), v);
}

static TIMER_CALLBACK( hblank_end_callback )
{
	video_screen_update_partial(machine->primary_screen, param);

	esripsys__12sel ^= 1;
	timer_adjust_oneshot(hblank_start_timer, video_screen_get_time_until_pos(machine->primary_screen, vcount, ESRIPSYS_HBLANK_START), 0);
}

VIDEO_START( esripsys )
{
	int i;

	/* Allocate memory for the two 512-pixel line buffers */
	line_buffer_1 = auto_malloc(512 * sizeof(pixel));
	line_buffer_2 = auto_malloc(512 * sizeof(pixel));

	/* Create and initialise the HBLANK timers */
	hblank_start_timer = timer_alloc(machine, hblank_start_callback, NULL);
	hblank_end_timer = timer_alloc(machine, hblank_end_callback, NULL);
	timer_adjust_oneshot(hblank_start_timer, video_screen_get_time_until_pos(machine->primary_screen, 0, ESRIPSYS_HBLANK_START), 0);

	/* Create the sprite scaling table */
	scale_table = auto_malloc(64 * 64);

	for (i = 0; i < 64; ++i)
	{
		int j;

		for (j = 1; j < 65; ++j)
		{
			int p0 = 0;
			int p1 = 0;
			int p2 = 0;
			int p3 = 0;
			int p4 = 0;
			int p5 = 0;

			if (i & 0x1)
				p0 = BIT(j, 5) && !BIT(j, 4) && !BIT(j,3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x2)
				p1 = BIT(j, 4) && !BIT(j, 3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x4)
				p2 = BIT(j,3) && !BIT(j, 2) && !BIT(j, 1) && !BIT(j, 0);
			if (i & 0x8)
				p3 = BIT(j, 2) && !BIT(j,1) && !BIT(j,0);
			if (i & 0x10)
				p4 = BIT(j, 1) && !BIT(j, 0);
			if (i & 0x20)
				p5 = BIT(j, 0);

			scale_table[i * 64 + j - 1] = p0 | p1 | p2 | p3 | p4 | p5;
		}
	}

	/* Now create a lookup table for scaling the sprite 'fig' value */
	fig_scale_table = auto_malloc(1024 * 64);

	for (i = 0; i < 1024; ++i)
	{
		int scale;

		for (scale = 0; scale < 64; ++scale)
		{
			int input_pixels = i + 1;
			int scaled_pixels = 0;

			while (input_pixels)
			{
				if (scale_table[scale * 64 + (scaled_pixels & 0x3f)] == 0)
					input_pixels--;

				scaled_pixels++;
			}

			fig_scale_table[i * 64 + scale] = scaled_pixels - 1;
		}
	}
}

VIDEO_UPDATE( esripsys )
{
	int x;
	int y;
	pixel *src;

	/* Select line buffer to scan out */
	if (esripsys__12sel)
		src = line_buffer_2;
	else
		src = line_buffer_1;

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; ++x)
		{
			int idx = src[x].colour;
			int r = (esripsys_pal_ram[idx] & 0xf);
			int g = (esripsys_pal_ram[256 + idx] & 0xf);
			int b = (esripsys_pal_ram[512 + idx] & 0xf);
			int i = src[x].intensity;

			*dest++ = MAKE_RGB(r*i, g*i, b*i);

			/* Clear the line buffer as we scan out */
			src[x].colour = 0xff;
			src[x].intensity = bg_intensity;
			src[x].priority = 0;
		}
	}

	return 0;
}

WRITE8_HANDLER( esripsys_bg_intensity_w )
{
	bg_intensity = data & 0xf;
}

/* Draw graphics to a line buffer */
int esripsys_draw(running_machine *machine, int l, int r, int fig, int attr, int addr, int col, int x_scale, int line_latch)
{
	pixel *dst;
	UINT8 pri = attr & 0xff;
	UINT8 iny = (attr >> 8) & 0xf;
	UINT8 pal = col << 4;
	int x_flip = x_scale & 0x80;
	int xs_typ = x_scale & 0x40;
	int xs_val = x_scale & 0x3f;

	/* Fig is the number of pixels to draw / 2 - 1 */
	if (xs_typ)
		fig = fig_scale_table[fig * 64 + xs_val];

	if (esripsys__12sel)
		dst = line_buffer_1;
	else
		dst = line_buffer_2;

	/* 8bpp case */
	if (attr & 0x8000)
	{
		int ptr = 0;
		int cnt;
		UINT8 *rom_l;
		UINT8 *rom_r;
		UINT32 lpos = l;
		UINT32 rpos = r;

		if (x_flip)
		{
			rom_l = memory_region(machine, "8bpp_r");
			rom_r = memory_region(machine, "8bpp_l");
		}
		else
		{
			rom_l = memory_region(machine, "8bpp_l");
			rom_r = memory_region(machine, "8bpp_r");
		}

		for (cnt = 0; cnt <= fig; cnt++)
		{
			UINT32 rom_addr = (ptr * 0x10000) + addr;
			UINT8 pix1 = rom_l[rom_addr];
			UINT8 pix2 = rom_r[rom_addr];

			if ((UINT32)lpos < 512)
			{
				if ((pri > dst[lpos].priority) && pix1 != 0xff)
				{
					dst[lpos].colour = pix1;
					dst[lpos].priority = pri;
					dst[lpos].intensity = iny;
				}
			}

			if ((UINT32)rpos < 512)
			{
				if ((pri > dst[rpos].priority) && pix2 != 0xff)
				{
					dst[rpos].colour = pix2;
					dst[rpos].priority = pri;
					dst[rpos].intensity = iny;
				}
			}

			/* Shrink */
			if (!xs_typ)
			{
				if (scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					--lpos;
					++rpos;
				}

				if (++ptr == 4)
				{
					++addr;
					ptr = 0;
				}
			}
			else
			{
				if (!scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					if (++ptr == 4)
					{
						++addr;
						ptr = 0;
					}
				}

				lpos--;
				rpos++;
			}
		}
	}
	/* 4bpp case */
	else
	{
		const UINT8* const rom = memory_region(machine, "4bpp");
		int ptr = 0;
		int cnt;
		UINT32 lpos = l;
		UINT32 rpos = r;

		for (cnt = 0; cnt <= fig; cnt++)
		{
			UINT8 px8 = rom[(ptr * 0x10000) + addr];
			UINT8 px1;
			UINT8 px2;

			if (x_flip)
			{
				px1 = px8 & 0xf;
				px2 = (px8 >> 4) & 0xf;
			}
			else
			{
				px2 = px8 & 0xf;
				px1 = (px8 >> 4) & 0xf;
			}

			if ((UINT32)lpos < 512)
			{
				if ((pri > dst[lpos].priority) && px1 != 0xf)
				{
					dst[lpos].colour = pal | px1;
					dst[lpos].priority = pri;
					dst[lpos].intensity = iny;
				}
			}

			if ((UINT32)rpos < 512)
			{
				if (pri > dst[rpos].priority && px2 != 0xf)
				{
					dst[rpos].colour = pal | px2;
					dst[rpos].priority = pri;
					dst[rpos].intensity = iny;
				}
			}

			/* Shrink */
			if (!xs_typ)
			{
				if (scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					lpos--;
					rpos++;
				}

				if (++ptr == 4)
				{
					addr++;
					ptr = 0;
				}
			}
			else
			{
				if (!scale_table[xs_val * 64 + (cnt & 0x3f)])
				{
					if (++ptr == 4)
					{
						addr++;
						ptr = 0;
					}
				}
				lpos--;
				rpos++;
			}
		}
	}

	return fig + 1;
}
