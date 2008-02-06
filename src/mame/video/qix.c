/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "qix.h"
#include "video/crtc6845.h"


/* Constants */
#define SCANLINE_INCREMENT	1


/* Globals */
UINT8 *qix_videoaddress;
UINT8 qix_cocktail_flip;


/* Local variables */
static UINT8 vram_mask;
static UINT8 qix_palettebank;
static UINT8 leds;
static emu_timer *scanline_timer;
static UINT8 scanline_latch;



/*************************************
 *
 *  Static function prototypes
 *
 *************************************/

static void qix_display_enable_changed(int display_enabled);

static TIMER_CALLBACK( scanline_callback );

static void *qix_begin_update(running_machine *machine,
							  int screen,
							  mame_bitmap *bitmap,
							  const rectangle *cliprect);

static void qix_update_row(mame_bitmap *bitmap,
						   const rectangle *cliprect,
						   UINT16 ma,
						   UINT8 ra,
						   UINT16 y,
						   UINT8 x_count,
						   void *param);



/*************************************
 *
 *  Video startup
 *
 *************************************/

static const crtc6845_interface crtc6845_intf =
{
	0,						/* screen we are acting on */
	QIX_CHARACTER_CLOCK, 	/* the clock (pin 21) of the chip */
	8,						/* number of pixels per video memory address */
	qix_begin_update,		/* before pixel update callback */
	qix_update_row,			/* row update callback */
	0,						/* after pixel update callback */
	qix_display_enable_changed	/* call back for display state changes */
};


VIDEO_START( qix )
{
	/* configure the CRT controller */
	crtc6845_config(0, &crtc6845_intf);

	/* allocate memory for the full video RAM */
	videoram = auto_malloc(256 * 256);

	/* initialize the mask for games that don't use it */
	vram_mask = 0xff;

	/* allocate a timer */
	scanline_timer = timer_alloc(scanline_callback, NULL);
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(0, 1, 0), 1);

	/* set up save states */
	state_save_register_global_pointer(videoram, 256 * 256);
	state_save_register_global(qix_cocktail_flip);
	state_save_register_global(vram_mask);
	state_save_register_global(qix_palettebank);
	state_save_register_global(leds);
}



/*************************************
 *
 *  Scanline caching
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	int scanline = param;

	/* force a partial update */
	video_screen_update_partial(0, scanline - 1);

	/* set a timer for the next increment */
	scanline += SCANLINE_INCREMENT;
	if (scanline > machine->screen[0].visarea.max_y)
		scanline = SCANLINE_INCREMENT;
	timer_adjust_oneshot(scanline_timer, video_screen_get_time_until_pos(0, scanline, 0), scanline);
}



/*************************************
 *
 *  Current scanline read
 *
 *************************************/

static void qix_display_enable_changed(int display_enabled)
{
	/* on the rising edge, latch the scanline */
	if (display_enabled)
	{
		UINT16 ma = crtc6845_get_ma(0);
		UINT8 ra = crtc6845_get_ra(0);

		/* RA0-RA2 goes to D0-D2 and MA5-MA9 goes to D3-D7 */
		scanline_latch = ((ma >> 2) & 0xf8) | (ra & 0x07);
	}
}


READ8_HANDLER( qix_scanline_r )
{
	return scanline_latch;
}



/*************************************
 *
 *  Video RAM mask
 *
 *************************************/

WRITE8_HANDLER( slither_vram_mask_w )
{
	/* Slither appears to extend the basic hardware by providing */
	/* a mask register which controls which data bits get written */
	/* to video RAM */
	vram_mask = data;
}



/*************************************
 *
 *  Direct video RAM read/write
 *
 *  The screen is 256x256 with eight
 *  bit pixels (64K).  The screen is
 *  divided into two halves each half
 *  mapped by the video CPU at
 *  $0000-$7FFF.  The high order bit
 *  of the address latch at $9402
 *  specifies which half of the screen
 *  is being accessed.
 *
 *************************************/

READ8_HANDLER( qix_videoram_r )
{
	/* add in the upper bit of the address latch */
	offset += (qix_videoaddress[0] & 0x80) << 8;
	return videoram[offset];
}


WRITE8_HANDLER( qix_videoram_w )
{
	/* add in the upper bit of the address latch */
	offset += (qix_videoaddress[0] & 0x80) << 8;

	/* blend the data */
	videoram[offset] = (videoram[offset] & ~vram_mask) | (data & vram_mask);
}



/*************************************
 *
 *  Latched video RAM read/write
 *
 *  The address latch works as follows.
 *  When the video CPU accesses $9400,
 *  the screen address is computed by
 *  using the values at $9402 (high
 *  byte) and $9403 (low byte) to get
 *  a value between $0000-$FFFF.  The
 *  value at that location is either
 *  returned or written.
 *
 *************************************/

READ8_HANDLER( qix_addresslatch_r )
{
	/* compute the value at the address latch */
	offset = (qix_videoaddress[0] << 8) | qix_videoaddress[1];
	return videoram[offset];
}



WRITE8_HANDLER( qix_addresslatch_w )
{
	/* compute the value at the address latch */
	offset = (qix_videoaddress[0] << 8) | qix_videoaddress[1];

	/* blend the data */
	videoram[offset] = (videoram[offset] & ~vram_mask) | (data & vram_mask);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

WRITE8_HANDLER( qix_paletteram_w )
{
	/* this conversion table should be about right. It gives a reasonable */
	/* gray scale in the test screen, and the red, green and blue squares */
	/* in the same screen are barely visible, as the manual requires. */
	static const UINT8 table[16] =
	{
		0x00,	/* value = 0, intensity = 0 */
		0x12,	/* value = 0, intensity = 1 */
		0x24,	/* value = 0, intensity = 2 */
		0x49,	/* value = 0, intensity = 3 */
		0x12,	/* value = 1, intensity = 0 */
		0x24,	/* value = 1, intensity = 1 */
		0x49,	/* value = 1, intensity = 2 */
		0x92,	/* value = 1, intensity = 3 */
		0x5b,	/* value = 2, intensity = 0 */
		0x6d,	/* value = 2, intensity = 1 */
		0x92,	/* value = 2, intensity = 2 */
		0xdb,	/* value = 2, intensity = 3 */
		0x7f,	/* value = 3, intensity = 0 */
		0x91,	/* value = 3, intensity = 1 */
		0xb6,	/* value = 3, intensity = 2 */
		0xff	/* value = 3, intensity = 3 */
	};
	int bits, intensity, red, green, blue;

	/* set the palette RAM value */
	paletteram[offset] = data;

	/* compute R, G, B from the table */
	intensity = (data >> 0) & 0x03;
	bits = (data >> 6) & 0x03;
	red = table[(bits << 2) | intensity];
	bits = (data >> 4) & 0x03;
	green = table[(bits << 2) | intensity];
	bits = (data >> 2) & 0x03;
	blue = table[(bits << 2) | intensity];

	/* update the palette */
	palette_set_color(Machine, offset, MAKE_RGB(red, green, blue));
}


WRITE8_HANDLER( qix_palettebank_w )
{
	/* set the bank value */
	if (qix_palettebank != (data & 3))
	{
		video_screen_update_partial(0, video_screen_get_vpos(0) - 1);
		qix_palettebank = data & 3;
	}

	/* LEDs are in the upper 6 bits */
	leds = ~data & 0xfc;
}



/*************************************
 *
 *  CRTC callbacks for updating
 *  the screen
 *
 *************************************/

static void *qix_begin_update(running_machine *machine, int screen,
							  mame_bitmap *bitmap, const rectangle *cliprect)
{
#if 0
	// note the confusing bit order!
	popmessage("self test leds: %d%d %d%d%d%d",BIT(leds,7),BIT(leds,5),BIT(leds,6),BIT(leds,4),BIT(leds,2),BIT(leds,3));
#endif

	/* return the pens we are going to use to update the display */
	return (void *)&machine->pens[qix_palettebank * 256];
}


static void qix_update_row(mame_bitmap *bitmap, const rectangle *cliprect,
						   UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param)
{
	UINT16 x;
	UINT8 scanline[256];

	pen_t *pens = (pen_t *)param;

	/* the memory is hooked up to the MA, RA lines this way */
	offs_t offs = ((ma << 6) & 0xf800) | ((ra << 8) & 0x0700);
	offs_t offs_xor = qix_cocktail_flip ? 0xffff : 0;

	for (x = 0; x < x_count * 8; x++)
		scanline[x] = videoram[(offs + x) ^ offs_xor];

	draw_scanline8(bitmap, 0, y, x_count * 8, scanline, pens, -1);
}
