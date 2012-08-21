/***************************************************************************

    TExt Display 7360
    PeT mess@utanet.at


    2010-02: Converted to be a device and merged video/audio emulation

    TODO:
      - Plenty of clean-ups

***************************************************************************/
/*
Metal Oxid Semicontuctor MOS
Commodore Business Machine CBM
Textured Display Ted7360
------------------------

7360/8360 (NTSC-M, PAL-B by same chip ?)
8365 PAL-N
8366 PAL-M

Ted7360 used in Commodore C16
Ted8360 used in Commodore C116/Plus4
 should be 100% software compatible with Ted7360

video unit (without sprites), dram controller seams to be much like vic6567

PAL/NTSC variants

functions of this chip:
 memory controller
 dram controller
 cpu clock generation with single and double clock switching
 3 timer
 1 8bit latched input
 2 channel sound/noise generator
 video controller
 interrupt controller for timer and video

Reasons for this document:
 document state in mess c16/plus4 emulator
 base to discuss ted7360 with others (i have no c16 computer anymore)

pal:
 clock 17734470
 divided by 5 for base clock (pixel clock changing edges?)
 312 lines
 50 hertz vertical refresh

ntsc:
 clock 14318180
 divided by 4 for base clock (pixel clock changing edges?)
 262 lines
 60 hertz vertical refresh

registers:
 0xff00 timer 1 low byte
 0xff01 timer 1 high byte
 0xff02 timer 2 low byte
 0xff03 timer 2 high byte
 0xff04 timer 3 low byte
 0xff05 timer 3 high byte
 0xff06
  bit 7 test?
  bit 6 ecm on
  bit 5 hires on
  bit 4 screen on
  bit 3 25 raws (0 24 raws)
  bit 2-0 vertical position
 0xff07
  bit 7 reverse off (0 on)
  bit 6 read only NTSC (0 PAL)
  bit 5 freeze horicontal position?
  bit 4 multicolor on
  bit 3 40 columns (0 38 columns)
  bit 2-0 horicontal pos
 0xff08 input latch (0 input low), write reloads latch
 0xff09 interrupt request
  7: interrupt
  6: timer 3
  5: ?
  4: timer 2
  3: timer 1
  2: lightpen
  1: rasterline
  0: 1 (asl quitting)
 0xff0a interrupt enable
  7: ?
  6: timer 3
  5: ?
  4: timer 2
  3: timer 1
  2: lightpen
  1: rasterline
  0: irq rasterline bit 8
 0xff0b
  7-0: irq rasterline 7-0
 0xff0c
  7-2: 1?
  1,0: cursorpos bit 9,8
 0xff0d cursorpos bit 7-0
 0xff0e tone channel 1: frequency 7-0
 0xff0f tone channel 2: frequency 7-0
 0xff10
  7-2: ?
  1,0: tone channel 2 bit 9,8
 0xff11
  7: sound reload
  6: tone 2 noise on (tone 2 must be off for noise)
  5: tone 2 tone on
  4: tone 1 on
  3-0: volume 8?-0
 0xff12
  7,6: ?
  5-3: bitmap address bit 15-13
  2: bitmapmemory?, charactermemory in rom (0 ram)
  1,0: tone 1 frequency bit 9,8
 0xff13
  7-2: chargen address bit 15-11
  2: chargen address bit 10 (only used when reverse is off)
  1: single clock in overscan area (0 double clock)
  0: ram (0)/rom status
 0xff14
  7-2: video address bit 15-10
  1,0: ?
 0xff15
  7: ?
  6-0: backgroundcolor
 0xff16
  7: ?
  6-0: color1
 0xff17
  7: ?
  6-0: color2
 0xff18
  7: ?
  6-0: color3
 0xff19
  7: ?
  6-0: framecolor
 0xff1a
  7-2: ?
  1,0: bitmap reload(cursorpos2) bit 9-8
 0xff1b bitmap reload bit 7-0
 0xff1c
  7-1: 1? (matrix expects this)
  0: current rasterline bit 8
 0xff1d current rasterline bit 7-0
 0xff1e
  7-0: current rastercolumn /2 (0 begin visible area?)
 0xff1f cursorblink?
  7: ?
  6-3: blink counter
  2-0: vsub
 0xff3e write switches to rom
 0xff3f write switches to dram

memory controller
 0x8000-0xfcff, 0xff20-0xffff
  read selection between dram and rom
 0xff3e-0xff3f write nothing (would change main memory in 16k ram c16)

clock generator
 in displayed area ted need 3 memory cycles to get data for video chip
 memory system runs at approximately 4 megahertz
 so only 1 megahertz is available for the cpu
 in the other area the clock can be doubled for the cpu

dram controller
 generator of refresh cycles
 when are these occuring?
 vic6567 in each rasterline reads 5 refresh addresses

sound generator
 2 channels
 channel 2 can be switched to generate noise
 volume control (0-8?)
 tone reload bit?
 samples ton/noise?
 frequency=(base clock/32)/(1024-value)

timer
 timer count down
 when reaching 0
  they restart from 0xffff
  set the interrupt request flag
 writing to low byte stops timer
 writing to high byte restarts timer with current value

interrupt controller
 interrupt sources must be enabled in the enable register to generate
 an interrupt
 request flag are set also when not enabled
 interrupt is generated when rising edge on request and enable
 quitting of interrupt is done via writing 1 to the request bit
 what happens with request interrupts when the are enabled?

Video part
 colors
  bit 3-0 color
   black, white, red, cyan
   purple, green, blue, yellow
   orange, light orange, pink, light cyan,
   light violett, light green, light blue, light yellow
  bit 6-4 luminance (0 dark)
  r,g,b values from 0 to 127
   taken from digitized tv screen shot
   0x06,0x01,0x03, 0x2b,0x2b,0x2b, 0x67,0x0e,0x0f, 0x00,0x3f,0x42,
   0x57,0x00,0x6d, 0x00,0x4e,0x00, 0x19,0x1c,0x94, 0x38,0x38,0x00,
   0x56,0x20,0x00, 0x4b,0x28,0x00, 0x16,0x48,0x00, 0x69,0x07,0x2f,
   0x00,0x46,0x26, 0x06,0x2a,0x80, 0x2a,0x14,0x9b, 0x0b,0x49,0x00,

   0x00,0x03,0x02, 0x3d,0x3d,0x3d, 0x75,0x1e,0x20, 0x00,0x50,0x4f,
   0x6a,0x10,0x78, 0x04,0x5c,0x00, 0x2a,0x2a,0xa3, 0x4c,0x47,0x00,
   0x69,0x2f,0x00, 0x59,0x38,0x00, 0x26,0x56,0x00, 0x75,0x15,0x41,
   0x00,0x58,0x3d, 0x15,0x3d,0x8f, 0x39,0x22,0xae, 0x19,0x59,0x00,

   0x00,0x03,0x04, 0x42,0x42,0x42, 0x7b,0x28,0x20, 0x02,0x56,0x59,
   0x6f,0x1a,0x82, 0x0a,0x65,0x09, 0x30,0x34,0xa7, 0x50,0x51,0x00,
   0x6e,0x36,0x00, 0x65,0x40,0x00, 0x2c,0x5c,0x00, 0x7d,0x1e,0x45,
   0x01,0x61,0x45, 0x1c,0x45,0x99, 0x42,0x2d,0xad, 0x1d,0x62,0x00,

   0x05,0x00,0x02, 0x56,0x55,0x5a, 0x90,0x3c,0x3b, 0x17,0x6d,0x72,
   0x87,0x2d,0x99, 0x1f,0x7b,0x15, 0x46,0x49,0xc1, 0x66,0x63,0x00,
   0x84,0x4c,0x0d, 0x73,0x55,0x00, 0x40,0x72,0x00, 0x91,0x33,0x5e,
   0x19,0x74,0x5c, 0x32,0x59,0xae, 0x59,0x3f,0xc3, 0x32,0x76,0x00,

   0x02,0x01,0x06, 0x84,0x7e,0x85, 0xbb,0x67,0x68, 0x45,0x96,0x96,
   0xaf,0x58,0xc3, 0x4a,0xa7,0x3e, 0x73,0x73,0xec, 0x92,0x8d,0x11,
   0xaf,0x78,0x32, 0xa1,0x80,0x20, 0x6c,0x9e,0x12, 0xba,0x5f,0x89,
   0x46,0x9f,0x83, 0x61,0x85,0xdd, 0x84,0x6c,0xef, 0x5d,0xa3,0x29,

   0x02,0x00,0x0a, 0xb2,0xac,0xb3, 0xe9,0x92,0x92, 0x6c,0xc3,0xc1,
   0xd9,0x86,0xf0, 0x79,0xd1,0x76, 0x9d,0xa1,0xff, 0xbd,0xbe,0x40,
   0xdc,0xa2,0x61, 0xd1,0xa9,0x4c, 0x93,0xc8,0x3d, 0xe9,0x8a,0xb1,
   0x6f,0xcd,0xab, 0x8a,0xb4,0xff, 0xb2,0x9a,0xff, 0x88,0xcb,0x59,

   0x02,0x00,0x0a, 0xc7,0xca,0xc9, 0xff,0xac,0xac, 0x85,0xd8,0xe0,
   0xf3,0x9c,0xff, 0x92,0xea,0x8a, 0xb7,0xba,0xff, 0xd6,0xd3,0x5b,
   0xf3,0xbe,0x79, 0xe6,0xc5,0x65, 0xb0,0xe0,0x57, 0xff,0xa4,0xcf,
   0x89,0xe5,0xc8, 0xa4,0xca,0xff, 0xca,0xb3,0xff, 0xa2,0xe5,0x7a,

   0x01,0x01,0x01, 0xff,0xff,0xff, 0xff,0xf6,0xf2, 0xd1,0xff,0xff,
   0xff,0xe9,0xff, 0xdb,0xff,0xd3, 0xfd,0xff,0xff, 0xff,0xff,0xa3,
   0xff,0xff,0xc1, 0xff,0xff,0xb2, 0xfc,0xff,0xa2, 0xff,0xee,0xff,
   0xd1,0xff,0xff, 0xeb,0xff,0xff, 0xff,0xf8,0xff, 0xed,0xff,0xbc

 video generation
  column counter from 0 to 39
  line counter from 0 to 199

  character pointer position/hires: color 0-15 position
   videoaddr+0x400+(line/8)*40+column
  color position/hires: color luminance position
   videoaddr+(line/8)*40+column
  character bitmap position
   chargenaddr+(character pointer)+(line&7)
  bitmap position
   bitmapaddr+(line/8)*40+column+(line&7)

 normal text mode (reverse off, ecm off, multicolor off, hires off)
  character pointer based
   fetch character pointer, fetch character bitmap, fetch color
  pixel on color (color)&0x7f
  pixel off color backgroundcolor
  blinking (synchron to cursor) (color&0x80)
 reverse text mode (reverse on, ecm off, multicolor off, hires off)
  character pointer &0x7f
  character pointer bit 7
   set
    pixel off color attribut &0x7f
    pixel on color backgroundcolor
   clear
    like normal text mode
  blinking (synchron to cursor) (color&0x80)
 multicolor text mode (ecm off, multicolor on, hires off)
  reverse ?
  attribut bit 3 character in multicolor (else monocolor character)
  pixel on color attribut 0x77
  0 backgroundcolor
  1 color1
  2 color2
  3 attribut &0x77
  (color&0x80) ?
 ecm text mode (ecm on)
  reverse ?
  multicolor ? (c64/vic6567 black display?)
  hires ? (c64/vic6567 black display?)
  character pointer &0x3f
  character pointer bit 7,6 select pixel off color
   0 backgroundcolor ?
   1 color1?
   2 color2?
   3 color3?
  (color&0x80)?
 hires (ecm off, multicolor off, hires on)
  bitmap based
   fetch bitmap, fetch color, fetch color luminance
  reverse ?
  pixel on color
   6-4: (color luminance) bit 2-0
   3-0: (color) bit 7-4
  pixel off color
   6-4: (color luminance) bit 6-4
   3-0: (color) bit 3-0
  (color) bit 7?
  (color luminance) bit 7?
 hires multicolor (ecm off, multicolor on, hires on)
  reverse ?
  0 backgroundcolor
  1
   6-4: (color luminance) bit 2-0
   3-0: (color) bit 7-4
  2
   6-4: (color luminance) bit 6-4
   3-0: (color) bit 3-0
  3 color1
  (color) bit 7?
  (color luminance) bit 7?
 scrolling support
  visible part of the display in fixed position
  columns 38 mode left 7 and right 9 pixel less displayed
   but line generation start at the same position as in columns 40 mode
  lines25 flag top and bottom 4 pixel less displayed ?
  horicontal scrolling support
   in columns 38 mode
    horicontal position 1-7: pixel later started line generation
    horicontal position 0: 8 pixels later started line generation
   in columns 40 mode ?
  vertical scrolling support
   in lines 24 mode
    vertical position 1-7: lines later started line generation
    vertical position 0: 8 lines later started line generation
   in lines 25 mode?
    vertial position 3: for correct display

 hardware cursor
  full 8x8 in (color) color
  flashing when in multicolor, hires ?
  flashes with about 1 hertz
  bitmap reload?

 rasterline/rastercolumn
  values in registers begin visible area 0
  vic 6567
   pal line 0 beginning of vertical refresh
   ntsc line 0 in bottom frame
    beginning of 25 lines screen 0x33 (24 lines screen 0x37)
    beginning of 40 columns line 0x18 (38 columns 0x1f)

 lightpen
  (where to store values?)
  i found a lightpen hardware description with a demo and drawing program
  lightpen must be connected to joy0 (button)
  demo program:
   disables interrupt, polles joy0 button (0xfd0d? plus 4 related???)
   and reads the rasterline value!
   so i no exact column value reachable!
*/
#include "emu.h"
#include "audio/ted7360.h"


typedef struct _ted7360_state ted7360_state;
struct _ted7360_state
{
	ted_type  type;

	screen_device *screen;			// screen which sets bitmap properties

	UINT8 reg[0x20];

	bitmap_ind16 *bitmap;

	int rom;

	int frame_count;

	int lines;
	int timer1_active, timer2_active, timer3_active;
	emu_timer *timer1, *timer2, *timer3;
	int cursor1;

	ted7360_dma_read         dma_read;
	ted7360_dma_read_rom     dma_read_rom;
	ted7360_irq              interrupt;		// c16_interrupt
	ted7360_key_cb           keyboard_cb;	// c16_read_keyboard

	int chargenaddr, bitmapaddr, videoaddr;

	int x_begin, x_end;
	int y_begin, y_end;

	UINT16 c16_bitmap[2], bitmapmulti[4], mono[2], monoinversed[2], multi[4], ecmcolor[2], colors[5];

	int rasterline, lastline;
	double rastertime;


	/* sound part */
	int tone1pos, tone2pos,
	tone1samples, tone2samples,
	noisesize,		  /* number of samples */
	noisepos,         /* pos of tone */
	noisesamples;	  /* count of samples to give out per tone */

	sound_stream *channel;
	UINT8 *noise;
};

/*****************************************************************************
    CONSTANTS
*****************************************************************************/


#define VERBOSE_LEVEL 0
#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", device->machine().time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while(0)


#define VREFRESHINLINES 28

#define TIMER1HELPER (ted7360->reg[0] | (ted7360->reg[1] << 8))
#define TIMER2HELPER (ted7360->reg[2] | (ted7360->reg[3] << 8))
#define TIMER3HELPER (ted7360->reg[4] | (ted7360->reg[5] << 8))
#define TIMER1 (TIMER1HELPER ? TIMER1HELPER : 0x10000)
#define TIMER2 (TIMER2HELPER ? TIMER2HELPER : 0x10000)
#define TIMER3 (TIMER3HELPER ? TIMER3HELPER : 0x10000)

#define TED7360_YPOS            40
#define RASTERLINE_2_C16(a)    ((a + ted7360->lines - TED7360_YPOS - 5) % ted7360->lines)
#define C16_2_RASTERLINE(a)    ((a + TED7360_YPOS + 5) % ted7360->lines)
#define XPOS 8
#define YPOS 8

#define SCREENON               (ted7360->reg[6] & 0x10)
#define TEST                   (ted7360->reg[6] & 0x80)
#define VERTICALPOS            (ted7360->reg[6] & 0x07)
#define HORICONTALPOS          (ted7360->reg[7] & 0x07)
#define ECMON                  (ted7360->reg[6] & 0x40)
#define HIRESON                (ted7360->reg[6] & 0x20)
#define MULTICOLORON           (ted7360->reg[7] & 0x10)
#define REVERSEON              (!(ted7360->reg[7] & 0x80))

/* hardware inverts character when bit 7 set (character taken &0x7f) */
/* instead of fetching character with higher number! */
#define LINES25     (ted7360->reg[6] & 0x08)     /* else 24 Lines */
#define LINES       (LINES25 ? 25 : 24)
#define YSIZE       (LINES * 8)
#define COLUMNS40   (ted7360->reg[7] & 0x08)     /* else 38 Columns */
#define COLUMNS     (COLUMNS40 ? 40 : 38)
#define XSIZE       (COLUMNS * 8)

#define INROM       (ted7360->reg[0x12] & 0x04)
#define CHARGENADDR (REVERSEON && !HIRESON && !MULTICOLORON ? ((ted7360->reg[0x13] & 0xfc) << 8) : ((ted7360->reg[0x13] & 0xf8) << 8))
#define BITMAPADDR  ((ted7360->reg[0x12] & 0x38) << 10)
#define VIDEOADDR   ((ted7360->reg[0x14] & 0xf8) << 8)

#define RASTERLINE  (((ted7360->reg[0xa] & 0x01) << 8) | ted7360->reg[0xb])
#define CURSOR1POS  (ted7360->reg[0xd] | ((ted7360->reg[0xc] & 0x03) << 8))
#define CURSOR2POS  (ted7360->reg[0x1b] | ((ted7360->reg[0x1a] & 0x03) << 8))
#define CURSORRATE  ((ted7360->reg[0x1f] & 0x7c) >> 2)

#define BACKGROUNDCOLOR (ted7360->reg[0x15] & 0x7f)
#define FOREGROUNDCOLOR (ted7360->reg[0x16] & 0x7f)
#define MULTICOLOR1     (ted7360->reg[0x17] & 0x7f)
#define MULTICOLOR2     (ted7360->reg[0x18] & 0x7f)
#define FRAMECOLOR      (ted7360->reg[0x19] & 0x7f)

#define TED7360_VRETRACERATE ((ted7360->type == TED7360_PAL) ? TED7360PAL_VRETRACERATE : TED7360NTSC_VRETRACERATE)
#define TED7360_CLOCK        (((ted7360->type == TED7360_PAL) ? TED7360PAL_CLOCK : TED7360NTSC_CLOCK) / 4)
#define TED7360_LINES        ((ted7360->type == TED7360_PAL) ? TED7360PAL_LINES : TED7360NTSC_LINES)

static attotime TEDTIME_IN_CYCLES(int pal, int cycles)
{
	double d = (double)(cycles) / (pal ? TED7360PAL_CLOCK : TED7360NTSC_CLOCK);
	return attotime::from_double(d);
}

static int TEDTIME_TO_CYCLES(int pal, attotime t)
{
	double d = t.as_double();
	return (int)((d) * (pal ? TED7360PAL_CLOCK : TED7360NTSC_CLOCK));
}

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ted7360_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TED7360);

	return (ted7360_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const ted7360_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TED7360);

	return (const ted7360_interface *) device->static_config();
}

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static void ted7360_soundport_w( device_t *device, int offset, int data );

static void ted7360_set_interrupt( running_machine &machine, int mask, ted7360_state *ted7360 )
{
	/* kernel itself polls for timer 2 shot (interrupt disabled!) when cassette loading */
	ted7360->reg[9] |= mask;
	if ((ted7360->reg[0xa] & ted7360->reg[9] & 0x5e))
	{
		if (!(ted7360->reg[9] & 0x80))
		{
			//DBG_LOG(1, "ted7360", ("irq start %.2x\n", mask));
			ted7360->reg[9] |= 0x80;
			ted7360->interrupt(machine, 1);
		}
	}
	ted7360->reg[9] |= mask;
}

static void ted7360_clear_interrupt( device_t *device, int mask )
{
	ted7360_state *ted7360 = get_safe_token(device);

	ted7360->reg[9] &= ~mask;
	if ((ted7360->reg[9] & 0x80) && !(ted7360->reg[9] & ted7360->reg[0xa] & 0x5e))
	{
		DBG_LOG(1, "ted7360", ("irq end %.2x\n", mask));
		ted7360->reg[9] &= ~0x80;
		ted7360->interrupt(device->machine(), 0);
	}
}

static int ted7360_rastercolumn( device_t *device )
{
	ted7360_state *ted7360 = get_safe_token(device);
	return (int) ((device->machine().time().as_double() - ted7360->rastertime) * TED7360_VRETRACERATE * ted7360->lines * 57 * 8 + 0.5);
}

static TIMER_CALLBACK(ted7360_timer_timeout)
{
	ted7360_state *ted7360 = (ted7360_state *)ptr;
	int which = param;

	//DBG_LOG(3, "ted7360 ", ("timer %d timeout\n", which));
	switch (which)
	{
	case 1:
		// proved by digisound of several intros like eoroidpro
		ted7360->timer1->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), TIMER1), 1);
		ted7360->timer1_active = 1;
		ted7360_set_interrupt(machine, 0x08, ted7360);
		break;
	case 2:
		ted7360->timer2->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), 0x10000), 2);
		ted7360->timer2_active = 1;
		ted7360_set_interrupt(machine, 0x10, ted7360);
		break;
	case 3:
		ted7360->timer3->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), 0x10000), 3);
		ted7360->timer3_active = 1;
		ted7360_set_interrupt(machine, 0x40, ted7360);
		break;
	}
}

static void ted7360_draw_character( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		if (INROM)
			code = ted7360->dma_read_rom(device->machine(), ted7360->chargenaddr + ch * 8 + y);
		else
			code = ted7360->dma_read(device->machine(), ted7360->chargenaddr + ch * 8 + y);

		ted7360->bitmap->pix16(y + yoff, 0 + xoff) = color[code >> 7];
		ted7360->bitmap->pix16(y + yoff, 1 + xoff) = color[(code >> 6) & 1];
		ted7360->bitmap->pix16(y + yoff, 2 + xoff) = color[(code >> 5) & 1];
		ted7360->bitmap->pix16(y + yoff, 3 + xoff) = color[(code >> 4) & 1];
		ted7360->bitmap->pix16(y + yoff, 4 + xoff) = color[(code >> 3) & 1];
		ted7360->bitmap->pix16(y + yoff, 5 + xoff) = color[(code >> 2) & 1];
		ted7360->bitmap->pix16(y + yoff, 6 + xoff) = color[(code >> 1) & 1];
		ted7360->bitmap->pix16(y + yoff, 7 + xoff) = color[code & 1];
	}
}

static void ted7360_draw_character_multi( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		if (INROM)
			code = ted7360->dma_read_rom(device->machine(), ted7360->chargenaddr + ch * 8 + y);
		else
			code = ted7360->dma_read(device->machine(), ted7360->chargenaddr + ch * 8 + y);

		ted7360->bitmap->pix16(y + yoff, 0 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 1 + xoff) = ted7360->multi[code >> 6];
		ted7360->bitmap->pix16(y + yoff, 2 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 3 + xoff) = ted7360->multi[(code >> 4) & 3];
		ted7360->bitmap->pix16(y + yoff, 4 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 5 + xoff) = ted7360->multi[(code >> 2) & 3];
		ted7360->bitmap->pix16(y + yoff, 6 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 7 + xoff) = ted7360->multi[code & 3];
	}
}

static void ted7360_draw_bitmap( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = ted7360->dma_read(device->machine(), ted7360->bitmapaddr + ch * 8 + y);
		ted7360->bitmap->pix16(y + yoff, 0 + xoff) = ted7360->c16_bitmap[code >> 7];
		ted7360->bitmap->pix16(y + yoff, 1 + xoff) = ted7360->c16_bitmap[(code >> 6) & 1];
		ted7360->bitmap->pix16(y + yoff, 2 + xoff) = ted7360->c16_bitmap[(code >> 5) & 1];
		ted7360->bitmap->pix16(y + yoff, 3 + xoff) = ted7360->c16_bitmap[(code >> 4) & 1];
		ted7360->bitmap->pix16(y + yoff, 4 + xoff) = ted7360->c16_bitmap[(code >> 3) & 1];
		ted7360->bitmap->pix16(y + yoff, 5 + xoff) = ted7360->c16_bitmap[(code >> 2) & 1];
		ted7360->bitmap->pix16(y + yoff, 6 + xoff) = ted7360->c16_bitmap[(code >> 1) & 1];
		ted7360->bitmap->pix16(y + yoff, 7 + xoff) = ted7360->c16_bitmap[code & 1];
	}
}

static void ted7360_draw_bitmap_multi( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = ted7360->dma_read(device->machine(), ted7360->bitmapaddr + ch * 8 + y);

		ted7360->bitmap->pix16(y + yoff, 0 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 1 + xoff) = ted7360->bitmapmulti[code >> 6];
		ted7360->bitmap->pix16(y + yoff, 2 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 3 + xoff) = ted7360->bitmapmulti[(code >> 4) & 3];
		ted7360->bitmap->pix16(y + yoff, 4 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 5 + xoff) = ted7360->bitmapmulti[(code >> 2) & 3];
		ted7360->bitmap->pix16(y + yoff, 6 + xoff) =
			ted7360->bitmap->pix16(y + yoff, 7 + xoff) = ted7360->bitmapmulti[code & 3];
	}
}

#ifndef memset16
static void *memset16 (void *dest, int value, size_t size)
{
	register int i;

	for (i = 0; i < size; i++)
		((short *) dest)[i] = value;
	return dest;
}
#endif

static void ted7360_draw_cursor( device_t *device, int ybegin, int yend, int yoff, int xoff, int color )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int y;

	for (y = ybegin; y <= yend; y++)
	{
		memset16(&ted7360->bitmap->pix16(yoff + y, xoff), color, 8);
	}
}

static void ted7360_drawlines( device_t *device, int first, int last )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int line, vline, end;
	int attr, ch, c1, c2, ecm;
	int offs, yoff, xoff, ybegin, yend, xbegin, xend;
	int i;

	ted7360->lastline = last;

	/* top part of display not rastered */
	first -= TED7360_YPOS;
	last -= TED7360_YPOS;
	if ((first >= last) || (last <= 0))
		return;
	if (first < 0)
		first = 0;

	if (!SCREENON)
	{
		for (line = first; (line < last) && (line < ted7360->bitmap->height()); line++)
			memset16(&ted7360->bitmap->pix16(line), 0, ted7360->bitmap->width());
		return;
	}

	if (COLUMNS40)
		xbegin = XPOS, xend = xbegin + 320;
	else
		xbegin = XPOS + 7, xend = xbegin + 304;

	if (last < ted7360->y_begin)
		end = last;
	else
		end = ted7360->y_begin + YPOS;
	{
		for (line = first; line < end; line++)
			memset16(&ted7360->bitmap->pix16(line), FRAMECOLOR, ted7360->bitmap->width());
	}
	if (LINES25)
		vline = line - ted7360->y_begin - YPOS;
	else
		vline = line - ted7360->y_begin - YPOS + 8 - VERTICALPOS;

	if (last < ted7360->y_end + YPOS)
		end = last;
	else
		end = ted7360->y_end + YPOS;

	for (; line < end; vline = (vline + 8) & ~7, line = line + 1 + yend - ybegin)
	{
		offs = (vline >> 3) * 40;
		ybegin = vline & 7;
		yoff = line - ybegin;
		yend = (yoff + 7 < end) ? 7 : (end - yoff - 1);
		/* rendering 39 characters */
		/* left and right borders are overwritten later */

		for (xoff = ted7360->x_begin + XPOS; xoff < ted7360->x_end + XPOS; xoff += 8, offs++)
		{
			if (HIRESON)
			{
				ch = ted7360->dma_read(device->machine(), (ted7360->videoaddr | 0x400) + offs);
				attr = ted7360->dma_read(device->machine(), ted7360->videoaddr + offs);
				c1 = ((ch >> 4) & 0xf) | (attr << 4);
				c2 = (ch & 0xf) | (attr & 0x70);
				ted7360->bitmapmulti[1] = ted7360->c16_bitmap[1] = c1 & 0x7f;
				ted7360->bitmapmulti[2] = ted7360->c16_bitmap[0] = c2 & 0x7f;
				if (MULTICOLORON)
				{
					ted7360_draw_bitmap_multi(device, ybegin, yend, offs, yoff, xoff);
				}
				else
				{
					ted7360_draw_bitmap(device, ybegin, yend, offs, yoff, xoff);
				}
			}
			else
			{
				ch = ted7360->dma_read(device->machine(), (ted7360->videoaddr | 0x400) + offs);
				attr = ted7360->dma_read(device->machine(), ted7360->videoaddr + offs);
				// levente harsfalvi's docu says cursor off in ecm and multicolor
				if (ECMON)
				{
					// hardware reverse off
					ecm = ch >> 6;
					ted7360->ecmcolor[0] = ted7360->colors[ecm];
					ted7360->ecmcolor[1] = attr & 0x7f;
					ted7360_draw_character(device, ybegin, yend, ch & ~0xc0, yoff, xoff, ted7360->ecmcolor);
				}
				else if (MULTICOLORON)
				{
					// hardware reverse off
					if (attr & 8)
					{
						ted7360->multi[3] = attr & 0x77;
						ted7360_draw_character_multi(device, ybegin, yend, ch, yoff, xoff);
					}
					else
					{
						ted7360->mono[1] = attr & 0x7f;
						ted7360_draw_character(device, ybegin, yend, ch, yoff, xoff, ted7360->mono);
					}
				}
				else if (ted7360->cursor1 && (offs == CURSOR1POS))
				{
					ted7360_draw_cursor(device, ybegin, yend, yoff, xoff, attr & 0x7f);
				}
				else if (REVERSEON && (ch & 0x80))
				{
					ted7360->monoinversed[0] = attr & 0x7f;
					if (ted7360->cursor1 && (attr & 0x80))
						ted7360_draw_cursor(device, ybegin, yend, yoff, xoff, ted7360->monoinversed[0]);
					else
						ted7360_draw_character(device, ybegin, yend, ch & ~0x80, yoff, xoff, ted7360->monoinversed);
				}
				else
				{
					ted7360->mono[1] = attr & 0x7f;
					if (ted7360->cursor1 && (attr & 0x80))
						ted7360_draw_cursor(device, ybegin, yend, yoff, xoff, ted7360->mono[0]);
					else
						ted7360_draw_character(device, ybegin, yend, ch, yoff, xoff, ted7360->mono);
				}
			}
		}

		for (i = ybegin; i <= yend; i++)
		{
			memset16(&ted7360->bitmap->pix16(yoff + i), FRAMECOLOR, xbegin);
			memset16(&ted7360->bitmap->pix16(yoff + i, xend), FRAMECOLOR, ted7360->bitmap->width() - xend);
		}
	}

	if (last < ted7360->bitmap->height())
		end = last;
	else
		end = ted7360->bitmap->height();

	for (; line < end; line++)
	{
		memset16(&ted7360->bitmap->pix16(line), FRAMECOLOR, ted7360->bitmap->width());
	}
}


WRITE8_DEVICE_HANDLER( ted7360_port_w )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int old;

	if ((offset != 8) && ((offset < 0x15) || (offset > 0x19)))
	{
		DBG_LOG(1, "ted7360_port_w", ("%.2x:%.2x\n", offset, data));
	}

	switch (offset)
	{
	case 0xe:
	case 0xf:
	case 0x10:
	case 0x11:
	case 0x12:
		ted7360_soundport_w(device, offset, data);
		break;
	}
	switch (offset)
	{
	case 0:						   /* stop timer 1 */
		ted7360->reg[offset] = data;

		if (ted7360->timer1_active)
		{
			ted7360->reg[1] = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer1->remaining()) >> 8;
			ted7360->timer1->reset();
			ted7360->timer1_active = 0;
		}
		break;
	case 1:						   /* start timer 1 */
		ted7360->reg[offset] = data;
		ted7360->timer1->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), TIMER1), 1);
		ted7360->timer1_active = 1;
		break;
	case 2:						   /* stop timer 2 */
		ted7360->reg[offset] = data;
		if (ted7360->timer2_active)
		{
			ted7360->reg[3] = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer2->remaining()) >> 8;
			ted7360->timer2->reset();
			ted7360->timer2_active = 0;
		}
		break;
	case 3:						   /* start timer 2 */
		ted7360->reg[offset] = data;
		ted7360->timer2->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), TIMER2), 2);
		ted7360->timer2_active = 1;
		break;
	case 4:						   /* stop timer 3 */
		ted7360->reg[offset] = data;
		if (ted7360->timer3_active)
		{
			ted7360->reg[5] = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer3->remaining()) >> 8;
			ted7360->timer3->reset();
			ted7360->timer3_active = 0;
		}
		break;
	case 5:						   /* start timer 3 */
		ted7360->reg[offset] = data;
		ted7360->timer3->adjust(TEDTIME_IN_CYCLES((ted7360->type == TED7360_PAL), TIMER3), 3);
		ted7360->timer3_active = 1;
		break;
	case 6:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			if (LINES25)
			{
				ted7360->y_begin = 0;
				ted7360->y_end = ted7360->y_begin + 200;
			}
			else
			{
				ted7360->y_begin = 4;
				ted7360->y_end = ted7360->y_begin + 192;
			}
			ted7360->chargenaddr = CHARGENADDR;
		}
		break;
	case 7:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			if (COLUMNS40)
			{
				ted7360->x_begin = 0;
				ted7360->x_end = ted7360->x_begin + 320;
			}
			else
			{
				ted7360->x_begin = HORICONTALPOS;
				ted7360->x_end = ted7360->x_begin + 320;
			}
			DBG_LOG(3, "ted7360_port_w", ("%s %s\n", data & 0x40 ? "ntsc" : "pal", data & 0x20 ? "hori freeze" : ""));
			ted7360->chargenaddr = CHARGENADDR;
		}
		break;
	case 8:
		ted7360->reg[offset] = ted7360->keyboard_cb(device->machine(), data);
		break;
	case 9:
		if (data & 0x08)
			ted7360_clear_interrupt(device, 8);
		if (data & 0x10)
			ted7360_clear_interrupt(device, 0x10);
		if (data & 0x40)
			ted7360_clear_interrupt(device, 0x40);
		if (data & 0x02)
			ted7360_clear_interrupt(device, 2);
		break;
	case 0xa:
		old = data;
		ted7360->reg[offset] = data | 0xa0;
#if 0
		ted7360->reg[9] = (ted7360->reg[9] & 0xa1) | (ted7360->reg[9] & data & 0x5e);
		if (ted7360->reg[9] & 0x80)
			ted7360_clear_interrupt(device, 0);
#endif
		if ((data ^ old) & 1)
		{
			/* DBG_LOG(1,"set rasterline hi",("soll:%d\n",RASTERLINE)); */
		}
		break;
	case 0xb:
		if (data != ted7360->reg[offset])
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			/*  DBG_LOG(1,"set rasterline lo",("soll:%d\n",RASTERLINE)); */
		}
		break;
	case 0xc:
	case 0xd:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
		}
		break;
	case 0x12:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->bitmapaddr = BITMAPADDR;
			ted7360->chargenaddr = CHARGENADDR;
			DBG_LOG(3, "ted7360_port_w", ("bitmap %.4x %s\n",  BITMAPADDR, INROM ? "rom" : "ram"));
		}
		break;
	case 0x13:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->chargenaddr = CHARGENADDR;
			DBG_LOG(3, "ted7360_port_w", ("chargen %.4x %s %d\n", CHARGENADDR, data & 2 ? "" : "doubleclock", data & 1));
		}
		break;
	case 0x14:
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->videoaddr = VIDEOADDR;
			DBG_LOG(3, "ted7360_port_w", ("videoram %.4x\n", VIDEOADDR));
		}
		break;
	case 0x15:						   /* backgroundcolor */
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->monoinversed[1] = ted7360->mono[0] = ted7360->bitmapmulti[0] = ted7360->multi[0] = ted7360->colors[0] = BACKGROUNDCOLOR;
		}
		break;
	case 0x16:						   /* foregroundcolor */
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->bitmapmulti[3] = ted7360->multi[1] = ted7360->colors[1] = FOREGROUNDCOLOR;
		}
		break;
	case 0x17:						   /* multicolor 1 */
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->multi[2] = ted7360->colors[2] = MULTICOLOR1;
		}
		break;
	case 0x18:						   /* multicolor 2 */
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->colors[3] = MULTICOLOR2;
		}
		break;
	case 0x19:						   /* framecolor */
		if (ted7360->reg[offset] != data)
		{
			ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
			ted7360->reg[offset] = data;
			ted7360->colors[4] = FRAMECOLOR;
		}
		break;
	case 0x1c:
		ted7360->reg[offset] = data;		   /*? */
		DBG_LOG(1, "ted7360_port_w", ("write to rasterline high %.2x\n",
									   data));
		break;
	case 0x1f:
		ted7360->reg[offset] = data;
		DBG_LOG(1, "ted7360_port_w", ("write to cursorblink %.2x\n", data));
		break;
	default:
		ted7360->reg[offset] = data;
		break;
	}
}

READ8_DEVICE_HANDLER( ted7360_port_r )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int val = 0;

	switch (offset)
	{
	case 0:
		if (ted7360->timer1)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer1->remaining()) & 0xff;
		else
			val = ted7360->reg[offset];
		break;
	case 1:
		if (ted7360->timer1)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer1->remaining()) >> 8;
		else
			val = ted7360->reg[offset];
		break;
	case 2:
		if (ted7360->timer2)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer2->remaining()) & 0xff;
		else
			val = ted7360->reg[offset];
		break;
	case 3:
		if (ted7360->timer2)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer2->remaining()) >> 8;
		else
			val = ted7360->reg[offset];
		break;
	case 4:
		if (ted7360->timer3)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer3->remaining()) & 0xff;
		else
			val = ted7360->reg[offset];
		break;
	case 5:
		if (ted7360->timer3)
			val = TEDTIME_TO_CYCLES((ted7360->type == TED7360_PAL), ted7360->timer3->remaining()) >> 8;
		else
			val = ted7360->reg[offset];
		break;
	case 7:
		val = (ted7360->reg[offset] & ~0x40);
		if (ted7360->type == TED7360_NTSC)
			val |= 0x40;
		break;
	case 9:
		val = ted7360->reg[offset] | 1;
		break;
	case 0xa:
		val = ted7360->reg[offset];
		break;
	case 0xb:
		val = ted7360->reg[offset];
		break;
	case 0x0c:
		val = ted7360->reg[offset] |= 0xfc;
		break;
	case 0x13:
		val = ted7360->reg[offset] & ~1;
		if (ted7360->rom)
			val |= 1;
		break;
	case 0x1c:						   /*rasterline */
		ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
		val = ((RASTERLINE_2_C16(ted7360->rasterline) & 0x100) >> 8) | 0xfe;	/* expected by matrix */
		break;
	case 0x1d:						   /*rasterline */
		ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
		val = RASTERLINE_2_C16(ted7360->rasterline) & 0xff;
		break;
	case 0x1e:						   /*rastercolumn */
		val = ted7360_rastercolumn(device) / 2;	/* pengo >=0x99 */
		break;
	case 0x1f:
		val = ((ted7360->rasterline & 7) << 4) | (ted7360->reg[offset] & 0x0f);
		DBG_LOG(1, "ted7360_port_w", ("read from cursorblink %.2x\n", val));
		break;
	default:
		val = ted7360->reg[offset];
		break;
	}

	if ((offset != 8) && (offset >= 6) && (offset != 0x1c) && (offset != 0x1d) && (offset != 9) && ((offset < 0x15) || (offset > 0x19)))
	{
		DBG_LOG(1, "ted7360_port_r", ("%.2x:%.2x\n", offset, val));
	}

	return val;
}

WRITE_LINE_DEVICE_HANDLER( ted7360_rom_switch_w )
{
	ted7360_state *ted7360 = get_safe_token(device);
	ted7360->rom = state;
}

READ_LINE_DEVICE_HANDLER( ted7360_rom_switch_r )
{
	ted7360_state *ted7360 = get_safe_token(device);
	return ted7360->rom;
}

void ted7360_frame_interrupt_gen( device_t *device )
{
	ted7360_state *ted7360 = get_safe_token(device);

	if ((ted7360->reg[0x1f] & 0xf) >= 0x0f)
	{
		/*  if (ted7360->frame_count >= CURSORRATE) */
		ted7360->cursor1 ^= 1;
		ted7360->reg[0x1f] &= ~0xf;
		ted7360->frame_count = 0;
	}
	else
		ted7360->reg[0x1f]++;
}

void ted7360_raster_interrupt_gen( device_t *device )
{
	ted7360_state *ted7360 = get_safe_token(device);

	ted7360->rasterline++;
	ted7360->rastertime = device->machine().time().as_double();
	if (ted7360->rasterline >= ted7360->lines)
	{
		ted7360->rasterline = 0;
		ted7360_drawlines(device, ted7360->lastline, TED7360_LINES);
		ted7360->lastline = 0;
	}

	if (ted7360->rasterline == C16_2_RASTERLINE(RASTERLINE))
	{
		ted7360_drawlines(device, ted7360->lastline, ted7360->rasterline);
		ted7360_set_interrupt(device->machine(), 2, ted7360);
	}
}

UINT32 ted7360_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	ted7360_state *ted7360 = get_safe_token(device);

	copybitmap(bitmap, *ted7360->bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


/* Sound emulation */

#define NOISE_BUFFER_SIZE_SEC 5

#define TONE_ON         (!(ted7360->reg[0x11] & 0x80))		/* or tone update!? */
#define TONE1_ON        ((ted7360->reg[0x11] & 0x10))
#define TONE1_VALUE     (ted7360->reg[0x0e] | ((ted7360->reg[0x12] & 3) << 8))
#define TONE2_ON        ((ted7360->reg[0x11] & 0x20))
#define TONE2_VALUE     (ted7360->reg[0x0f] | ((ted7360->reg[0x10] & 3) << 8))
#define VOLUME          (ted7360->reg[0x11] & 0x0f)
#define NOISE_ON        (ted7360->reg[0x11] & 0x40)

/*
 * pal 111860.781
 * ntsc 111840.45
 */
#define TONE_FREQUENCY(reg)         ((TED7360_CLOCK >> 3) / (1024 - reg))
#define TONE_FREQUENCY_MIN          (TONE_FREQUENCY(0))
#define NOISE_FREQUENCY             (TED7360_CLOCK / 8 / (1024 - TONE2_VALUE))
#define NOISE_FREQUENCY_MAX         (TED7360_CLOCK / 8)


static void ted7360_soundport_w( device_t *device, int offset, int data )
{
	ted7360_state *ted7360 = get_safe_token(device);

	// int old = ted7360->reg[offset];
	ted7360->channel->update();

	switch (offset)
	{
	case 0x0e:
	case 0x12:
		if (offset == 0x12)
			ted7360->reg[offset] = (ted7360->reg[offset] & ~3) | (data & 3);
		else
			ted7360->reg[offset] = data;

		ted7360->tone1samples = device->machine().sample_rate() / TONE_FREQUENCY (TONE1_VALUE);
		DBG_LOG(1, "ted7360", ("tone1 %d %d sample:%d\n", TONE1_VALUE, TONE_FREQUENCY(TONE1_VALUE), ted7360->tone1samples));
		break;

	case 0xf:
	case 0x10:
		ted7360->reg[offset] = data;

		ted7360->tone2samples = device->machine().sample_rate() / TONE_FREQUENCY (TONE2_VALUE);
		DBG_LOG (1, "ted7360", ("tone2 %d %d sample:%d\n", TONE2_VALUE, TONE_FREQUENCY(TONE2_VALUE), ted7360->tone2samples));

		ted7360->noisesamples = (int) ((double) NOISE_FREQUENCY_MAX * device->machine().sample_rate() * NOISE_BUFFER_SIZE_SEC / NOISE_FREQUENCY);
		DBG_LOG (1, "ted7360", ("noise %d sample:%d\n", NOISE_FREQUENCY, ted7360->noisesamples));

		if (!NOISE_ON || ((double) ted7360->noisepos / ted7360->noisesamples >= 1.0))
			ted7360->noisepos = 0;
		break;

	case 0x11:
		ted7360->reg[offset] = data;
		DBG_LOG(1, "ted7360", ("%s volume %d, %s %s %s\n", TONE_ON?"on":"off",
				       VOLUME, TONE1_ON?"tone1":"", TONE2_ON?"tone2":"", NOISE_ON?"noise":""));

		if (!TONE_ON||!TONE1_ON) ted7360->tone1pos = 0;
		if (!TONE_ON||!TONE2_ON) ted7360->tone2pos = 0;
		if (!TONE_ON||!NOISE_ON) ted7360->noisepos = 0;
		break;
	}
}


/************************************/
/* Sound handler update             */
/************************************/

static STREAM_UPDATE( ted7360_update )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int i, v, a;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++)
	{
		v = 0;

		if (TONE1_ON)
		{
			if (ted7360->tone1pos <= ted7360->tone1samples / 2 || !TONE_ON)
				v += 0x2ff; // depends on the volume between sound and noise

			ted7360->tone1pos++;

			if (ted7360->tone1pos > ted7360->tone1samples)
				ted7360->tone1pos = 0;
		}

		if (TONE2_ON || NOISE_ON )
		{
			if (TONE2_ON)
			{						   /*higher priority ?! */
				if (ted7360->tone2pos <= ted7360->tone2samples / 2 || !TONE_ON)
					v += 0x2ff;

				ted7360->tone2pos++;

				if (ted7360->tone2pos > ted7360->tone2samples)
					ted7360->tone2pos = 0;
			}
			else
			{
				v += ted7360->noise[(int) ((double) ted7360->noisepos * ted7360->noisesize / ted7360->noisesamples)];
				ted7360->noisepos++;

				if ((double) ted7360->noisepos / ted7360->noisesamples >= 1.0)
					ted7360->noisepos = 0;
			}
		}

		a = VOLUME;
		if (a > 8)
			a = 8;

		v = v * a;

		buffer[i] = v;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

/************************************/
/* Sound handler start              */
/************************************/

static void ted7360_sound_start( device_t *device )
{
	ted7360_state *ted7360 = get_safe_token(device);
	int i;

	ted7360->channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, ted7360_update);

	/* buffer for fastest played sample for 5 second so we have enough data for min 5 second */
	ted7360->noisesize = NOISE_FREQUENCY_MAX * NOISE_BUFFER_SIZE_SEC;
	ted7360->noise = auto_alloc_array(device->machine(), UINT8, ted7360->noisesize);

	{
		int noiseshift = 0x7ffff8;
		UINT8 data;

		for (i = 0; i < ted7360->noisesize; i++)
		{
			data = 0;
			if (noiseshift & 0x400000)
				data |= 0x80;
			if (noiseshift & 0x100000)
				data |= 0x40;
			if (noiseshift & 0x010000)
				data |= 0x20;
			if (noiseshift & 0x002000)
				data |= 0x10;
			if (noiseshift & 0x000800)
				data |= 0x08;
			if (noiseshift & 0x000080)
				data |= 0x04;
			if (noiseshift & 0x000010)
				data |= 0x02;
			if (noiseshift & 0x000004)
				data |= 0x01;
			ted7360->noise[i] = data;
			if (((noiseshift & 0x400000) == 0) != ((noiseshift & 0x002000) == 0))
				noiseshift = (noiseshift << 1) | 1;
			else
				noiseshift <<= 1;
		}
	}
}

static DEVICE_START( ted7360 )
{
	ted7360_state *ted7360 = get_safe_token(device);
	const ted7360_interface *intf = (ted7360_interface *)device->static_config();
	int width, height;

	ted7360->screen = device->machine().device<screen_device>(intf->screen);
	width = ted7360->screen->width();
	height = ted7360->screen->height();

	ted7360->bitmap = auto_bitmap_ind16_alloc(device->machine(), width, height);

	ted7360->type = intf->type;

	ted7360->dma_read = intf->dma_read;
	ted7360->dma_read_rom = intf->dma_read_rom;
	ted7360->interrupt = intf->irq;

	ted7360->keyboard_cb = intf->keyb_cb;

	ted7360->timer1 = device->machine().scheduler().timer_alloc(FUNC(ted7360_timer_timeout), ted7360);
	ted7360->timer2 = device->machine().scheduler().timer_alloc(FUNC(ted7360_timer_timeout), ted7360);
	ted7360->timer3 = device->machine().scheduler().timer_alloc(FUNC(ted7360_timer_timeout), ted7360);

	ted7360_sound_start(device);

	device->save_item(NAME(ted7360->reg));

	device->save_item(NAME(*ted7360->bitmap));

	device->save_item(NAME(ted7360->rom));
	device->save_item(NAME(ted7360->lines));
	device->save_item(NAME(ted7360->chargenaddr));
	device->save_item(NAME(ted7360->bitmapaddr));
	device->save_item(NAME(ted7360->videoaddr));
	device->save_item(NAME(ted7360->timer1_active));
	device->save_item(NAME(ted7360->timer2_active));
	device->save_item(NAME(ted7360->timer3_active));
	device->save_item(NAME(ted7360->cursor1));
	device->save_item(NAME(ted7360->rasterline));
	device->save_item(NAME(ted7360->lastline));
	device->save_item(NAME(ted7360->rastertime));
	device->save_item(NAME(ted7360->frame_count));
	device->save_item(NAME(ted7360->x_begin));
	device->save_item(NAME(ted7360->x_end));
	device->save_item(NAME(ted7360->y_begin));
	device->save_item(NAME(ted7360->y_end));

	device->save_item(NAME(ted7360->c16_bitmap));
	device->save_item(NAME(ted7360->bitmapmulti));
	device->save_item(NAME(ted7360->mono));
	device->save_item(NAME(ted7360->monoinversed));
	device->save_item(NAME(ted7360->multi));
	device->save_item(NAME(ted7360->ecmcolor));
	device->save_item(NAME(ted7360->colors));

	device->save_item(NAME(ted7360->tone1pos));
	device->save_item(NAME(ted7360->tone2pos));
	device->save_item(NAME(ted7360->tone1samples));
	device->save_item(NAME(ted7360->tone2samples));
	device->save_item(NAME(ted7360->noisepos));
	device->save_item(NAME(ted7360->noisesamples));
}

static DEVICE_RESET( ted7360 )
{
	ted7360_state *ted7360 = get_safe_token(device);

	memset(ted7360->reg, 0, ARRAY_LENGTH(ted7360->reg));

	ted7360->rom = 1;	// FIXME: at start should be RAM or ROM? old c16 code set it to ROM at init: is it correct?

	ted7360->lines = TED7360_LINES;
	ted7360->chargenaddr = ted7360->bitmapaddr = ted7360->videoaddr = 0;
	ted7360->timer1_active = ted7360->timer2_active = ted7360->timer3_active = 0;
	ted7360->cursor1 = 0;

	ted7360->rasterline = 0;
	ted7360->lastline = 0;

	ted7360->rastertime = 0.0;

	ted7360->frame_count = 0;

	ted7360->x_begin = 0;
	ted7360->x_end = 0;
	ted7360->y_begin = 0;
	ted7360->y_end = 0;

	memset(ted7360->c16_bitmap, 0, ARRAY_LENGTH(ted7360->c16_bitmap));
	memset(ted7360->bitmapmulti, 0, ARRAY_LENGTH(ted7360->bitmapmulti));
	memset(ted7360->mono, 0, ARRAY_LENGTH(ted7360->mono));
	memset(ted7360->monoinversed, 0, ARRAY_LENGTH(ted7360->monoinversed));
	memset(ted7360->multi, 0, ARRAY_LENGTH(ted7360->multi));
	memset(ted7360->ecmcolor, 0, ARRAY_LENGTH(ted7360->ecmcolor));
	memset(ted7360->colors, 0, ARRAY_LENGTH(ted7360->colors));

	ted7360->tone1pos = 0;
	ted7360->tone2pos = 0;
	ted7360->tone1samples = 1;
	ted7360->tone2samples = 1;
	ted7360->noisepos = 0;
	ted7360->noisesamples = 1;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##ted7360##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"CBM TED 7360"
#define DEVTEMPLATE_FAMILY				"CBM Text Display Video Chip"
#include "devtempl.h"

DEFINE_LEGACY_SOUND_DEVICE(TED7360, ted7360);
