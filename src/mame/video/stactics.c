/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

---

The Video system used in Space Tactics is unusual.
Here are my notes on how the video system works:

There are 4, 4K pages of Video RAM. (B,D,E & F)

The first 1K of each VRAM page contains the following:
0 0 V V V V V H H H H H   offset value for each 8x8 bitmap
     (v-tile)  (h-tile)

The offset values are used to generate an access into the
last 2K of the VRAM page:
1 D D D D D D D D V V V   here we find 8x8 character data
     (offset)    (line)

In addition, in Page B, the upper nibble of the offset is
also used to select the color palette for the tile.

Page B, D, E, and F all work similarly, except that pages
D, E, and F can be offset from Page B by a given
number of scanlines, based on the contents of the offset
RAM

The offset RAM is addressed in this format:
1 0 0 0 P P P V V V V V V V V V
        (Page)   (scanline)
Page 4=D, 5=E, 6=F

Page D, E, and F are drawn offset from the top of the screen,
starting on the first scanline which contains a 1 in the
appropriate memory location

---

The composited monitor image is seen in a mirror.  It appears
to move when the player moves the handle, due to motors which
tilt the mirror up and down, and the monitor left and right.

---

As if this wasn't enough, there are also "3D" fire beams made
of 120 green LED's which are on a mechanism in front of the mirror.
Along with a single red "sight" LED.  I am reading in the sequence
ROMS and building up a character set to simulate the LEDS with
conventional character graphics.

Finally, there is a score display made of 7-segment LEDS, along
with Credits, Barriers, and Rounds displays made of some other
type of LED bar graphs.  I'm displaying them the best I can on the
bottom line of the screen

***************************************************************************/

#include "driver.h"

/* These are defined in machine/stactics.c */
extern int stactics_vert_pos;
extern int stactics_horiz_pos;
extern UINT8 *stactics_motor_on;

/* These are needed by machine/stactics.c  */
int stactics_vblank_count;
int stactics_shot_standby;
int stactics_shot_arrive;

/* These are needed by driver/stactics.c   */
UINT8 *stactics_videoram_b;
UINT8 *stactics_videoram_d;
UINT8 *stactics_videoram_e;
UINT8 *stactics_videoram_f;
UINT8 *stactics_palette;
UINT8 *stactics_display_buffer;

static UINT8 y_scroll_d;
static UINT8 y_scroll_e;
static UINT8 y_scroll_f;

static UINT16 beam_state;
static UINT16 old_beam_state;
static UINT16 beam_states_per_frame;



/* these come via observation of the color PROM */
#define BLACK_PEN	(pens[0x00])
#define GREEN_PEN	(pens[0x12])
#define RED_PEN		(pens[0x16])
#define YELLOW_PEN	(pens[0x1a])



/* The first 16 came from the 7448 BCD to 7-segment decoder data sheet */
/* The rest are made up */

static const UINT8 char_gfx[32*8] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space */
	0x80, 0x80, 0x80, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* extras... */
	0xf0, 0x80, 0x80, 0xf0, 0x00, 0x00, 0xf0, 0x00,   /* extras... */
	0x90, 0x90, 0x90, 0xf0, 0x00, 0x00, 0x00, 0x00,   /* extras... */
	0x00, 0x00, 0x00, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* extras... */
	0x00, 0x00, 0x00, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* extras... */
	0xf0, 0x90, 0x90, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 9 */
	0xf0, 0x90, 0x90, 0xf0, 0x90, 0x90, 0xf0, 0x00,   /* 8 */
	0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,   /* 7 */
	0xf0, 0x80, 0x80, 0xf0, 0x90, 0x90, 0xf0, 0x00,   /* 6 */
	0xf0, 0x80, 0x80, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 5 */
	0x90, 0x90, 0x90, 0xf0, 0x10, 0x10, 0x10, 0x00,   /* 4 */
	0xf0, 0x10, 0x10, 0xf0, 0x10, 0x10, 0xf0, 0x00,   /* 3 */
	0xf0, 0x10, 0x10, 0xf0, 0x80, 0x80, 0xf0, 0x00,   /* 2 */
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,   /* 1 */
	0xf0, 0x90, 0x90, 0x90, 0x90, 0x90, 0xf0, 0x00,   /* 0 */

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space */
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 1 pip */
	0x60, 0x90, 0x80, 0x60, 0x10, 0x90, 0x60, 0x00,   /* S for Score */
	0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,   /* 2 pips */
	0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00,   /* 3 pips */
	0x60, 0x90, 0x80, 0x80, 0x80, 0x90, 0x60, 0x00,   /* C for Credits */
	0xe0, 0x90, 0x90, 0xe0, 0x90, 0x90, 0xe0, 0x00,   /* B for Barriers */
	0xe0, 0x90, 0x90, 0xe0, 0xc0, 0xa0, 0x90, 0x00,   /* R for Rounds */
	0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00,   /* 4 pips */
	0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,   /* Colon */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   /* Space (Unused) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    /* Space */
};


PALETTE_INIT( stactics )
{
	int i;

	for (i = 0; i < 0x400; i++)
	{
		int bit0 = (color_prom[i] >> 0) & 0x01;
		int bit1 = (color_prom[i] >> 1) & 0x01;
		int bit2 = (color_prom[i] >> 2) & 0x01;
		int bit3 = (color_prom[i] >> 3) & 0x01;

		/* red component */
		int r = 0xff * bit0;

		/* green component */
		int g = 0xff * bit1 - 0xcc * bit3;

		/* blue component */
		int b = 0xff * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( stactics )
{
	y_scroll_d = 0;
	y_scroll_e = 0;
	y_scroll_f = 0;

	stactics_vblank_count = 0;
	stactics_shot_standby = 1;
	stactics_shot_arrive = 0;
	beam_state = 0;
	old_beam_state = 0;

	stactics_vblank_count = 0;
	stactics_vert_pos = 0;
	stactics_horiz_pos = 0;
	*stactics_motor_on = 0;
}


WRITE8_HANDLER( stactics_scroll_ram_w )
{
	switch ((offset & 0x700) >> 8)
	{
		case 4:  // Page D
			if (data&0x01)
				y_scroll_d = offset&0xff;
			break;

		case 5:  // Page E
			if (data&0x01)
				y_scroll_e = offset&0xff;
			break;

		case 6:  // Page F
			if (data&0x01)
				y_scroll_f = offset&0xff;
			break;
    }
}

WRITE8_HANDLER( stactics_speed_latch_w )
{
	/* This writes to a shift register which is clocked by   */
	/* a 555 oscillator.  This value determines the speed of */
	/* the LED fire beams as follows:                        */

	/*   555_freq / bits_in_SR * edges_in_SR / states_in_PR67 / frame_rate */
	/*      = num_led_states_per_frame  */
	/*   36439 / 8 * x / 32 / 60 ~= 19/8*x */

	/* Here, we will count the number of rising edges in the shift register */

	int i;
	int num_rising_edges = 0;

	for(i=0;i<8;i++)
	{
		if ( (((data>>i)&0x01) == 1) && (((data>>((i+1)%8))&0x01) == 0))
			num_rising_edges++;
	}

	beam_states_per_frame = num_rising_edges*19/8;
}

WRITE8_HANDLER( stactics_shot_trigger_w )
{
	stactics_shot_standby = 0;
}

WRITE8_HANDLER( stactics_shot_flag_clear_w )
{
	stactics_shot_arrive = 0;
}



static void update_beam(void)
{
	/* An LED fire beam! */
	/* (There were 120 green LEDS mounted in the cabinet in the game, */
	/*  and one red one, for the sight)                               */

	/* First, update the firebeam state */

	old_beam_state = beam_state;
	if (stactics_shot_standby == 0)
		beam_state = beam_state + beam_states_per_frame;

	/* These are thresholds for the two shots from the LED fire ROM */
	/* (Note: There are two more for sound triggers, */
	/*        whenever that gets implemented)        */
	if ((old_beam_state < 0x8b) & (beam_state >= 0x8b))
		stactics_shot_arrive = 1;

	if ((old_beam_state < 0xca) & (beam_state >= 0xca))
		stactics_shot_arrive = 1;

	if (beam_state >= 0x100)
	{
		beam_state = 0;
		stactics_shot_standby = 1;
	}
}



INLINE int get_pixel_on_plane(UINT8 *videoram, UINT8 y, UINT8 x, UINT8 y_scroll)
{
	UINT8 code;
	UINT8 gfx;

	/* compute effective row */
	y = y - y_scroll;

	/* get the character code at the given pixel */
	code = videoram[((y >> 3) << 5) | (x >> 3)];

	/* get the gfx byte */
	gfx = videoram[0x800 | (code << 3) | (y & 0x07)];

	/* return the appropriate pixel within the byte */
	return (gfx >> (7 - (x & 0x07))) & 0x01;
}


static void draw_background(mame_bitmap *bitmap, const rectangle *cliprect, const pen_t *pens)
{
	int y;

	fillbitmap(bitmap, BLACK_PEN, cliprect);

	/* for every row */
	for (y = 0; y < 0x100; y++)
	{
		int x;

		/* for every pixel on the row */
		for (x = 0; x < 0x100; x++)
		{
			/* get the pixels for the four planes */
			int pixel_b = get_pixel_on_plane(stactics_videoram_b, y, x, 0);
			int pixel_d = get_pixel_on_plane(stactics_videoram_d, y, x, y_scroll_d);
			int pixel_e = get_pixel_on_plane(stactics_videoram_e, y, x, y_scroll_e);
			int pixel_f = get_pixel_on_plane(stactics_videoram_f, y, x, y_scroll_f);

			/* get the color for this pixel */
			UINT8 color = stactics_videoram_b[((y >> 3) << 5) | (x >> 3)] >> 4;

			/* assemble the pen index */
			int pen = color |
					  (pixel_b << 4) |
					  (pixel_f << 5) |
					  (pixel_e << 6) |
					  (pixel_d << 7) |
					  ((stactics_palette[0] & 0x01) << 8) |
					  ((stactics_palette[1] & 0x01) << 9);

			/* compute the effective pixel coordinate after adjusting for the
               mirror movement - this is mechanical on the real machine */
			int sy = y + stactics_vert_pos;
			int sx = x - stactics_horiz_pos;

			/* plot if visible */
			if ((sy >= 0) && (sy < 0xf0) && (sx >= 0) && (sx < 0x100))
				*BITMAP_ADDR16(bitmap, sy, sx) = pens[pen];
		}
	}
}


static void draw_character(mame_bitmap *bitmap, const rectangle *cliprect, int code, int x, pen_t pen)
{
	int y;

	for (y = 0; y < 8; y++)
	{
		int i;
		UINT8 gfx_data = char_gfx[(code << 3) | (y & 0x07)];

		for (i = 0; i < 6; i++)
			if ((gfx_data << i) & 0x80)
				*BITMAP_ADDR16(bitmap, y + 248, 255 - (x + i)) = pen;
	}
}


static void update_artwork(mame_bitmap *bitmap, const rectangle *cliprect, const pen_t *pens)
{
	int i;
	UINT8 *beam_region = memory_region(REGION_USER1);

	int x = 11;
	int y = 170;

	/* for each LED */
	for (i = 0; i < 0x40; i++)
	{
		offs_t beam_data_offs;
		UINT8 beam_data;

		/* every 15th LED is not present on the real machine */
		if ((i & 0x0f) == 0x0f)  continue;

		beam_data_offs = ((i & 0x08) << 7) | ((i & 0x30) << 4) | beam_state;
		beam_data = beam_region[beam_data_offs];

		/* if the LED is on, draw */
		if ((beam_data >> (i & 0x07)) & 0x01)
		{
			*BITMAP_ADDR16(bitmap, y,     x    ) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y,     x + 1) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y + 1, x    ) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y + 1, x + 1) = GREEN_PEN;

			*BITMAP_ADDR16(bitmap, y,     256 - x) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y,     255 - x) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y + 1, 256 - x) = GREEN_PEN;
			*BITMAP_ADDR16(bitmap, y + 1, 255 - x) = GREEN_PEN;
		}

		x = x + 2;
		y = y - 1;
	}

	/* draw the sight LED, if on */
	if (*stactics_motor_on & 0x01)
	{
		x = 127;
		y = 112;

		*BITMAP_ADDR16(bitmap, y,     x + 1) = RED_PEN;
		*BITMAP_ADDR16(bitmap, y + 1, x    ) = RED_PEN;
		*BITMAP_ADDR16(bitmap, y + 1, x + 1) = RED_PEN;
		*BITMAP_ADDR16(bitmap, y + 1, x + 2) = RED_PEN;
		*BITMAP_ADDR16(bitmap, y + 2, x + 1) = RED_PEN;
	}

	/* score display */
	draw_character(bitmap, cliprect, 0x12, 16, YELLOW_PEN);	/* S */
	draw_character(bitmap, cliprect, 0x19, 22, YELLOW_PEN);	/* : */

	for (i = 0x01; i < 0x07; i++)
	{
		int code = stactics_display_buffer[i] & 0x0f;
		draw_character(bitmap, cliprect, code, 28 + ((i - 0x01) * 6), RED_PEN);
	}


	/* credits indicator */
	draw_character(bitmap, cliprect, 0x15, 80, YELLOW_PEN);	/* C */
	draw_character(bitmap, cliprect, 0x19, 86, YELLOW_PEN);	/* : */

	for (i = 0x07; i < 0x09; i++)
	{
		int code = 0x10 | (~stactics_display_buffer[i] & 0x0f);
		draw_character(bitmap, cliprect, code, 92 + ((i - 0x07) * 2), RED_PEN);
	}


	/* rounds indicator */
	draw_character(bitmap, cliprect, 0x16, 144, YELLOW_PEN);	/* R */
	draw_character(bitmap, cliprect, 0x19, 150, YELLOW_PEN);	/* : */

	for (i = 0x09; i < 0x0c; i++)
	{
		int code = 0x10 | (~stactics_display_buffer[i] & 0x0f);
		draw_character(bitmap, cliprect, code, 156 + ((i - 0x09) * 2), RED_PEN);
	}


	/* barriers indicator */
	draw_character(bitmap, cliprect, 0x17, 208, YELLOW_PEN);	/* B */
	draw_character(bitmap, cliprect, 0x19, 214, YELLOW_PEN);	/* : */

	for (i = 0x0c; i < 0x10; i++)
	{
		int code = 0x10 | (~stactics_display_buffer[i] & 0x0f);
		draw_character(bitmap, cliprect, code, 220 + ((i - 0x0c) * 2), RED_PEN);
	}
}


VIDEO_UPDATE( stactics )
{
	update_beam();

	/* update vblank counter */
	stactics_vblank_count++;

	draw_background(bitmap, cliprect, machine->pens);
	update_artwork(bitmap, cliprect, machine->pens);

	return 0;
}
