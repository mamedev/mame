/***************************************************************************

    MOS 6560 / 6561 Video Interface Chip


    Original code by PeT (mess@utanet.at), 1999


    2010 FP: converted to a device and merged the video & sound components

    TODO:
      - plenty of cleanups!
      - investigate attckufo chip features (no invert mode, no multicolor, 16 col chars)
      - investigate why some vic20 carts crash emulation

****************************************************************************

    Original notes:

    2 Versions
    6560 NTSC
    6561 PAL
    14 bit addr bus
    12 bit data bus
    (16 8 bit registers)
    alternates with MOS 6502 on the address bus
    fetch 8 bit characternumber and 4 bit color
    high bit of 4 bit color value determines:
    0: 2 color mode
    1: 4 color mode
    than fetch characterbitmap for characternumber
    2 color mode:
    set bit in characterbitmap gives pixel in color of the lower 3 color bits
    cleared bit gives pixel in backgroundcolor
    4 color mode:
    2 bits in the characterbitmap are viewed together
    00: backgroundcolor
    11: colorram
    01: helpercolor
    10: framecolor
    advance to next character in videorram until line is full
    repeat this 8 or 16 lines, before moving to next line in videoram
    screen ratio ntsc, pal 4/3

    pal version:
    can contain greater visible areas
    expects other sync position (so ntsc modules may be displayed at
    the upper left corner of the tv screen)
    pixel ratio seems to be different on pal and ntsc

    commodore vic20 notes
    6560 address line 13 is connected inverted to address line 15 of the board
    1 K 4 bit ram at 0x9400 is additional connected as 4 higher bits
    of the 6560 (colorram) without decoding the 6560 address line a8..a13

*****************************************************************************/


#include "emu.h"
#include "sound/mos6560.h"

/***************************************************************************
 TYPE DEFINITIONS
***************************************************************************/

typedef struct _mos6560_state  mos6560_state;
struct _mos6560_state
{
	mos6560_type  type;

	screen_device *screen;

	UINT8 reg[16];

	bitmap_ind16 *bitmap;

	int rasterline, lastline;
	double lightpenreadtime;

	int charheight, matrix8x16, inverted;
	int chars_x, chars_y;
	int xsize, ysize, xpos, ypos;
	int chargenaddr, videoaddr;

	/* values in videoformat */
	UINT16 backgroundcolor, framecolor, helpercolor;

	/* arrays for bit to color conversion without condition checking */
	UINT16 mono[2], monoinverted[2], multi[4], multiinverted[4];

	/* video chip settings */
	int total_xsize, total_ysize, total_lines, total_vretracerate;

	/* DMA */
	mos6560_dma_read          dma_read;
	mos6560_dma_read_color    dma_read_color;
	UINT8 last_data;

	/* lightpen */
	mos6560_lightpen_button_callback lightpen_button_cb;
	mos6560_lightpen_x_callback lightpen_x_cb;
	mos6560_lightpen_y_callback lightpen_y_cb;

	/* paddles */
	mos6560_paddle_callback        paddle_cb[2];

	/* sound part */
	int tone1pos, tone2pos, tone3pos,
	tonesize, tone1samples, tone2samples, tone3samples,
	noisesize,		  /* number of samples */
	noisepos,         /* pos of tone */
	noisesamples;	  /* count of samples to give out per tone */

	sound_stream *channel;
	INT16 *tone;
	INT8 *noise;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE mos6560_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == MOS656X);

	return (mos6560_state *)downcast<mos6560_device *>(device)->token();
}

INLINE const mos6560_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == MOS656X));

	return (const mos6560_interface *) device->static_config();
}

/*****************************************************************************
    PARAMETERS
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
	} while (0)


/* 2008-05 FP: lightpen code needs to read input port from vc20.c */

#define LIGHTPEN_BUTTON		((mos6560->lightpen_button_cb != NULL) ? mos6560->lightpen_button_cb(device->machine()) : 0)
#define LIGHTPEN_X_VALUE	((mos6560->lightpen_x_cb != NULL) ? mos6560->lightpen_x_cb(device->machine()) : 0)
#define LIGHTPEN_Y_VALUE	((mos6560->lightpen_y_cb != NULL) ? mos6560->lightpen_y_cb(device->machine()) : 0)

/* lightpen delivers values from internal counters
 * they do not start with the visual area or frame area */
#define MOS6560_X_BEGIN 38
#define MOS6560_Y_BEGIN -6			   /* first 6 lines after retrace not for lightpen! */
#define MOS6561_X_BEGIN 38
#define MOS6561_Y_BEGIN -6
#define MOS656X_X_BEGIN ((mos6560->type == MOS6561) ? MOS6561_X_BEGIN : MOS6560_X_BEGIN)
#define MOS656X_Y_BEGIN ((mos6560->type == MOS6561) ? MOS6561_Y_BEGIN : MOS6560_Y_BEGIN)

#define MOS656X_MAME_XPOS ((mos6560->type == MOS6561) ? MOS6561_MAME_XPOS : MOS6560_MAME_XPOS)
#define MOS656X_MAME_YPOS ((mos6560->type == MOS6561) ? MOS6561_MAME_YPOS : MOS6560_MAME_YPOS)

/* lightpen behaviour in pal or mono multicolor not tested */
#define MOS656X_X_VALUE ((LIGHTPEN_X_VALUE + MOS656X_X_BEGIN + MOS656X_MAME_XPOS)/2)
#define MOS656X_Y_VALUE ((LIGHTPEN_Y_VALUE + MOS656X_Y_BEGIN + MOS656X_MAME_YPOS)/2)

#define MOS656X_VRETRACERATE ((mos6560->type == MOS6561) ? MOS6561_VRETRACERATE : MOS6560_VRETRACERATE)

/* ntsc 1 - 8 */
/* pal 5 - 19 */
#define XPOS (((int)mos6560->reg[0] & 0x7f) * 4)
#define YPOS ((int)mos6560->reg[1] * 2)

/* ntsc values >= 31 behave like 31 */
/* pal value >= 32 behave like 32 */
#define CHARS_X ((int)mos6560->reg[2] & 0x7f)
#define CHARS_Y (((int)mos6560->reg[3] & 0x7e) >> 1)

/* colorram and backgroundcolor are changed */
#define INVERTED (!(mos6560->reg[0x0f] & 8))

#define CHARGENADDR (((int)mos6560->reg[5] & 0x0f) << 10)
#define VIDEOADDR ((((int)mos6560->reg[5] & 0xf0) << (10 - 4)) | (((int)mos6560->reg[2] & 0x80) << (9-7)))
#define VIDEORAMSIZE (YSIZE * XSIZE)
#define CHARGENSIZE (256 * HEIGHTPIXEL)

#define HELPERCOLOR (mos6560->reg[0x0e] >> 4)
#define BACKGROUNDCOLOR (mos6560->reg[0x0f] >> 4)
#define FRAMECOLOR (mos6560->reg[0x0f] & 0x07)

/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

static void mos6560_soundport_w( device_t *device, int offset, int data );

/*-------------------------------------------------
 mos6560_draw_character
-------------------------------------------------*/

static void mos6560_draw_character( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = mos6560->dma_read(device->machine(), (mos6560->chargenaddr + ch * mos6560->charheight + y) & 0x3fff);
		mos6560->last_data = code;
		mos6560->bitmap->pix16(y + yoff, xoff + 0) = color[code >> 7];
		mos6560->bitmap->pix16(y + yoff, xoff + 1) = color[(code >> 6) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 2) = color[(code >> 5) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 3) = color[(code >> 4) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 4) = color[(code >> 3) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 5) = color[(code >> 2) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 6) = color[(code >> 1) & 1];
		mos6560->bitmap->pix16(y + yoff, xoff + 7) = color[code & 1];
	}
}


/*-------------------------------------------------
 mos6560_draw_character_multi
-------------------------------------------------*/

static void mos6560_draw_character_multi( device_t *device, int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = mos6560->dma_read(device->machine(), (mos6560->chargenaddr + ch * mos6560->charheight + y) & 0x3fff);
		mos6560->last_data = code;
		mos6560->bitmap->pix16(y + yoff, xoff + 0) =
			mos6560->bitmap->pix16(y + yoff, xoff + 1) = color[code >> 6];
		mos6560->bitmap->pix16(y + yoff, xoff + 2) =
			mos6560->bitmap->pix16(y + yoff, xoff + 3) = color[(code >> 4) & 3];
		mos6560->bitmap->pix16(y + yoff, xoff + 4) =
			mos6560->bitmap->pix16(y + yoff, xoff + 5) = color[(code >> 2) & 3];
		mos6560->bitmap->pix16(y + yoff, xoff + 6) =
			mos6560->bitmap->pix16(y + yoff, xoff + 7) = color[code & 3];
	}
}


/*-------------------------------------------------
 mos6560_drawlines - draw a certain numer of lines
-------------------------------------------------*/

static void mos6560_drawlines( device_t *device, int first, int last )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int line, vline;
	int offs, yoff, xoff, ybegin, yend, i, j;
	int attr, ch;

	mos6560->lastline = last;
	if (first >= last)
		return;

	for (line = first; (line < mos6560->ypos) && (line < last); line++)
	{
		for (j = 0; j < mos6560->total_xsize; j++)
			mos6560->bitmap->pix16(line, j) = mos6560->framecolor;
	}

	for (vline = line - mos6560->ypos; (line < last) && (line < mos6560->ypos + mos6560->ysize);)
	{
		if (mos6560->matrix8x16)
		{
			offs = (vline >> 4) * mos6560->chars_x;
			yoff = (vline & ~0xf) + mos6560->ypos;
			ybegin = vline & 0xf;
			yend = (vline + 0xf < last - mos6560->ypos) ? 0xf : ((last - line) & 0xf) + ybegin;
		}
		else
		{
			offs = (vline >> 3) * mos6560->chars_x;
			yoff = (vline & ~7) + mos6560->ypos;
			ybegin = vline & 7;
			yend = (vline + 7 < last - mos6560->ypos) ? 7 : ((last - line) & 7) + ybegin;
		}

		if (mos6560->xpos > 0)
		{
			for (i = ybegin; i <= yend; i++)
				for (j = 0; j < mos6560->xpos; j++)
					mos6560->bitmap->pix16(yoff + i, j) = mos6560->framecolor;
		}

		for (xoff = mos6560->xpos; (xoff < mos6560->xpos + mos6560->xsize) && (xoff < mos6560->total_xsize); xoff += 8, offs++)
		{
			ch = mos6560->dma_read(device->machine(), (mos6560->videoaddr + offs) & 0x3fff);
			mos6560->last_data = ch;
			attr = (mos6560->dma_read_color(device->machine(), (mos6560->videoaddr + offs) & 0x3fff)) & 0xf;

			if (mos6560->type == MOS6560_ATTACKUFO)
			{
				/* the mos6560 variant used in attckufo only has only one draw mode */
				mos6560->mono[1] = attr;
				mos6560_draw_character(device, ybegin, yend, ch, yoff, xoff, mos6560->mono);
			}
			else if (mos6560->inverted)
			{
				if (attr & 8)
				{
					mos6560->multiinverted[0] = attr & 7;
					mos6560_draw_character_multi(device, ybegin, yend, ch, yoff, xoff, mos6560->multiinverted);
				}
				else
				{
					mos6560->monoinverted[0] = attr;
					mos6560_draw_character(device, ybegin, yend, ch, yoff, xoff, mos6560->monoinverted);
				}
			}
			else
			{
				if (attr & 8)
				{
					mos6560->multi[2] = attr & 7;
					mos6560_draw_character_multi(device, ybegin, yend, ch, yoff, xoff, mos6560->multi);
				}
				else
				{
					mos6560->mono[1] = attr;
					mos6560_draw_character(device, ybegin, yend, ch, yoff, xoff, mos6560->mono);
				}
			}
		}

		if (xoff < mos6560->total_xsize)
		{
			for (i = ybegin; i <= yend; i++)
				for (j = xoff; j < mos6560->total_xsize; j++)
					mos6560->bitmap->pix16(yoff + i, j) = mos6560->framecolor;
		}

		if (mos6560->matrix8x16)
		{
			vline = (vline + 16) & ~0xf;
			line = vline + mos6560->ypos;
		}
		else
		{
			vline = (vline + 8) & ~7;
			line = vline + mos6560->ypos;
		}
	}

	for (; line < last; line++)
		for (j = 0; j < mos6560->total_xsize; j++)
			mos6560->bitmap->pix16(line, j) = mos6560->framecolor;
}


/*-------------------------------------------------
 mos6560_port_w - write to regs
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( mos6560_port_w )
{
	mos6560_state *mos6560 = get_safe_token(device);

	DBG_LOG(1, "mos6560_port_w", ("%.4x:%.2x\n", offset, data));

	switch (offset)
	{
	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
	case 0xe:
		mos6560_soundport_w(device, offset, data);
		break;
	}

	if (mos6560->reg[offset] != data)
	{
		switch (offset)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
		case 0xe:
		case 0xf:
			mos6560_drawlines(device, mos6560->lastline, mos6560->rasterline);
			break;
		}
		mos6560->reg[offset] = data;

		switch (offset)
		{
		case 0:
			if ((mos6560->type != MOS6560_ATTACKUFO))
				mos6560->xpos = XPOS;
			break;
		case 1:
			if ((mos6560->type != MOS6560_ATTACKUFO))
				mos6560->ypos = YPOS;
			break;
		case 2:
			/* ntsc values >= 31 behave like 31 */
			/* pal value >= 32 behave like 32 */
			mos6560->chars_x = CHARS_X;
			mos6560->videoaddr = VIDEOADDR;
			mos6560->xsize = CHARS_X * 8;
			break;
		case 3:
			if ((mos6560->type != MOS6560_ATTACKUFO))
			{
				mos6560->matrix8x16 = data & 0x01;
				mos6560->charheight = mos6560->matrix8x16 ? 16 : 8;
			}
			mos6560->chars_y = CHARS_Y;
			mos6560->ysize = CHARS_Y * mos6560->charheight;
			break;
		case 5:
			mos6560->chargenaddr = CHARGENADDR;
			mos6560->videoaddr = VIDEOADDR;
			break;
		case 0xe:
			mos6560->multi[3] = mos6560->multiinverted[3] = mos6560->helpercolor = HELPERCOLOR;
			break;
		case 0xf:
			if ((mos6560->type != MOS6560_ATTACKUFO))
				mos6560->inverted = INVERTED;
			mos6560->multi[1] = mos6560->multiinverted[1] = mos6560->framecolor = FRAMECOLOR;
			mos6560->mono[0] = mos6560->monoinverted[1] = mos6560->multi[0] = mos6560->multiinverted[2] = mos6560->backgroundcolor = BACKGROUNDCOLOR;
			break;
		}
	}
}

/*-------------------------------------------------
 mos6560_port_r - read from regs
-------------------------------------------------*/

READ8_DEVICE_HANDLER( mos6560_port_r )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int val;

	switch (offset)
	{
	case 3:
		val = ((mos6560->rasterline & 1) << 7) | (mos6560->reg[offset] & 0x7f);
		break;
	case 4:						   /*rasterline */
		mos6560_drawlines(device, mos6560->lastline, mos6560->rasterline);
		val = (mos6560->rasterline / 2) & 0xff;
		break;
	case 6:						   /*lightpen horizontal */
	case 7:						   /*lightpen vertical */
		if (LIGHTPEN_BUTTON && ((device->machine().time().as_double() - mos6560->lightpenreadtime) * MOS656X_VRETRACERATE >= 1))
		{
			/* only 1 update each frame */
			/* and diode must recognize light */
			if (1)
			{
				mos6560->reg[6] = MOS656X_X_VALUE;
				mos6560->reg[7] = MOS656X_Y_VALUE;
			}
			mos6560->lightpenreadtime = device->machine().time().as_double();
		}
		val = mos6560->reg[offset];
		break;
	case 8:						   /* poti 1 */
	case 9:						   /* poti 2 */
		val = (mos6560->paddle_cb != NULL) ? mos6560->paddle_cb[offset - 8](device->machine()) : mos6560->reg[offset];
		break;
	default:
		val = mos6560->reg[offset];
		break;
	}
	DBG_LOG(3, "mos6560_port_r", ("%.4x:%.2x\n", offset, val));
	return val;
}

UINT8 mos6560_bus_r( device_t *device )
{
	mos6560_state *mos6560 = get_safe_token(device);

	return mos6560->last_data;
}

/*-------------------------------------------------
 mos6560_raster_interrupt_gen
-------------------------------------------------*/

void mos6560_raster_interrupt_gen( device_t *device )
{
	mos6560_state *mos6560 = get_safe_token(device);

	mos6560->rasterline++;
	if (mos6560->rasterline >= mos6560->total_lines)
	{
		mos6560->rasterline = 0;
		mos6560_drawlines(device, mos6560->lastline, mos6560->total_lines);
		mos6560->lastline = 0;
	}
}


/*-------------------------------------------------
 mos6560_video_update - copy the VIC bitmap to
     main screen bitmap
-------------------------------------------------*/

UINT32 mos6560_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	mos6560_state *mos6560 = get_safe_token(device);

	copybitmap(bitmap, *mos6560->bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*****************************************************************************
    SOUND IMPLEMENTATION
*****************************************************************************/

/*
 * assumed model:
 * each write to a ton/noise generated starts it new
 * each generator behaves like an timer
 * when it reaches 0, the next samplevalue is given out
 */

/*
 * noise channel
 * based on a document by diku0748@diku.dk (Asger Alstrup Nielsen)
 *
 * 23 bit shift register
 * initial value (0x7ffff8)
 * after shift bit 0 is set to bit 22 xor bit 17
 * dac sample bit22 bit20 bit16 bit13 bit11 bit7 bit4 bit2(lsb)
 *
 * emulation:
 * allocate buffer for 5 sec sampledata (fastest played frequency)
 * and fill this buffer in init with the required sample
 * fast turning off channel, immediate change of frequency
 */

#define NOISE_BUFFER_SIZE_SEC 5

#define TONE1_ON (mos6560->reg[0x0a] & 0x80)
#define TONE2_ON (mos6560->reg[0x0b] & 0x80)
#define TONE3_ON (mos6560->reg[0x0c] & 0x80)
#define NOISE_ON (mos6560->reg[0x0d] & 0x80)
#define VOLUME (mos6560->reg[0x0e] & 0x0f)

#define MOS656X_CLOCK ((mos6560->type == MOS6561) ? MOS6561_CLOCK : MOS6560_CLOCK)

#define TONE_FREQUENCY_MIN  (MOS656X_CLOCK/256/128)

#define TONE1_VALUE (8 * (128 - ((mos6560->reg[0x0a] + 1) & 0x7f)))
#define TONE1_FREQUENCY (MOS656X_CLOCK/32/TONE1_VALUE)

#define TONE2_VALUE (4 * (128 - ((mos6560->reg[0x0b] + 1) & 0x7f)))
#define TONE2_FREQUENCY (MOS656X_CLOCK/32/TONE2_VALUE)

#define TONE3_VALUE (2 * (128 - ((mos6560->reg[0x0c] + 1) & 0x7f)))
#define TONE3_FREQUENCY (MOS656X_CLOCK/32/TONE3_VALUE)

#define NOISE_VALUE (32 * (128 - ((mos6560->reg[0x0d] + 1) & 0x7f)))
#define NOISE_FREQUENCY (MOS656X_CLOCK/NOISE_VALUE)

#define NOISE_FREQUENCY_MAX (MOS656X_CLOCK/32/1)


/*-------------------------------------------------
 mos6560_soundport_w - write to regs
-------------------------------------------------*/

static void mos6560_soundport_w( device_t *device, int offset, int data )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int old = mos6560->reg[offset];
	mos6560->channel->update();

	switch (offset)
	{
	case 0x0a:
		mos6560->reg[offset] = data;
		if (!(old & 0x80) && TONE1_ON)
		{
			mos6560->tone1pos = 0;
			mos6560->tone1samples = device->machine().sample_rate() / TONE1_FREQUENCY;
			if (!mos6560->tone1samples == 0)
				mos6560->tone1samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone1 %.2x %d\n", data, TONE1_FREQUENCY));
		break;
	case 0x0b:
		mos6560->reg[offset] = data;
		if (!(old & 0x80) && TONE2_ON)
		{
			mos6560->tone2pos = 0;
			mos6560->tone2samples = device->machine().sample_rate() / TONE2_FREQUENCY;
			if (mos6560->tone2samples == 0)
				mos6560->tone2samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone2 %.2x %d\n", data, TONE2_FREQUENCY));
		break;
	case 0x0c:
		mos6560->reg[offset] = data;
		if (!(old & 0x80) && TONE3_ON)
		{
			mos6560->tone3pos = 0;
			mos6560->tone3samples = device->machine().sample_rate() / TONE3_FREQUENCY;
			if (mos6560->tone3samples == 0)
				mos6560->tone3samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone3 %.2x %d\n", data, TONE3_FREQUENCY));
		break;
	case 0x0d:
		mos6560->reg[offset] = data;
		if (NOISE_ON)
		{
			mos6560->noisesamples = (int) ((double) NOISE_FREQUENCY_MAX * device->machine().sample_rate()
								  * NOISE_BUFFER_SIZE_SEC / NOISE_FREQUENCY);
			DBG_LOG (1, "mos6560", ("noise %.2x %d sample:%d\n",
									data, NOISE_FREQUENCY, mos6560->noisesamples));
			if ((double) mos6560->noisepos / mos6560->noisesamples >= 1.0)
			{
				mos6560->noisepos = 0;
			}
		}
		else
		{
			mos6560->noisepos = 0;
		}
		break;
	case 0x0e:
		mos6560->reg[offset] = (old & ~0x0f) | (data & 0x0f);
		DBG_LOG (3, "mos6560", ("volume %d\n", data & 0x0f));
		break;
	}
}


/*-------------------------------------------------
 mos6560_update - update audio stream
-------------------------------------------------*/

static STREAM_UPDATE( mos6560_update )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int i, v;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++)
	{
		v = 0;
		if (TONE1_ON /*||(mos6560->tone1pos != 0) */ )
		{
			v += mos6560->tone[mos6560->tone1pos * mos6560->tonesize / mos6560->tone1samples];
			mos6560->tone1pos++;
#if 0
			mos6560->tone1pos %= mos6560->tone1samples;
#else
			if (mos6560->tone1pos >= mos6560->tone1samples)
			{
				mos6560->tone1pos = 0;
				mos6560->tone1samples = device->machine().sample_rate() / TONE1_FREQUENCY;
				if (mos6560->tone1samples == 0)
					mos6560->tone1samples = 1;
			}
#endif
		}
		if (TONE2_ON /*||(mos6560->tone2pos != 0) */ )
		{
			v += mos6560->tone[mos6560->tone2pos * mos6560->tonesize / mos6560->tone2samples];
			mos6560->tone2pos++;
#if 0
			mos6560->tone2pos %= mos6560->tone2samples;
#else
			if (mos6560->tone2pos >= mos6560->tone2samples)
			{
				mos6560->tone2pos = 0;
				mos6560->tone2samples = device->machine().sample_rate() / TONE2_FREQUENCY;
				if (mos6560->tone2samples == 0)
					mos6560->tone2samples = 1;
			}
#endif
		}
		if (TONE3_ON /*||(mos6560->tone3pos != 0) */ )
		{
			v += mos6560->tone[mos6560->tone3pos * mos6560->tonesize / mos6560->tone3samples];
			mos6560->tone3pos++;
#if 0
			mos6560->tone3pos %= mos6560->tone3samples;
#else
			if (mos6560->tone3pos >= mos6560->tone3samples)
			{
				mos6560->tone3pos = 0;
				mos6560->tone3samples = device->machine().sample_rate() / TONE3_FREQUENCY;
				if (mos6560->tone3samples == 0)
					mos6560->tone3samples = 1;
			}
#endif
		}
		if (NOISE_ON)
		{
			v += mos6560->noise[(int) ((double) mos6560->noisepos * mos6560->noisesize / mos6560->noisesamples)];
			mos6560->noisepos++;
			if ((double) mos6560->noisepos / mos6560->noisesamples >= 1.0)
			{
				mos6560->noisepos = 0;
			}
		}
		v = (v * VOLUME) << 2;
		if (v > 32767)
			buffer[i] = 32767;
		else if (v < -32767)
			buffer[i] = -32767;
		else
			buffer[i] = v;



	}
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

/*-------------------------------------------------
 mos6560_sound_start - start audio emulation
     (to be called at device start)
-------------------------------------------------*/

static void mos6560_sound_start( device_t *device )
{
	mos6560_state *mos6560 = get_safe_token(device);
	int i;

	mos6560->channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, mos6560_update);

	/* buffer for fastest played sample for 5 second so we have enough data for min 5 second */
	mos6560->noisesize = NOISE_FREQUENCY_MAX * NOISE_BUFFER_SIZE_SEC;
	mos6560->noise = auto_alloc_array(device->machine(), INT8, mos6560->noisesize);
	{
		int noiseshift = 0x7ffff8;
		char data;

		for (i = 0; i < mos6560->noisesize; i++)
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
			mos6560->noise[i] = data;
			if (((noiseshift & 0x400000) == 0) != ((noiseshift & 0x002000) == 0))
				noiseshift = (noiseshift << 1) | 1;
			else
				noiseshift <<= 1;
		}
	}
	mos6560->tonesize = device->machine().sample_rate() / TONE_FREQUENCY_MIN;

	if (mos6560->tonesize > 0)
	{
		mos6560->tone = auto_alloc_array(device->machine(), INT16, mos6560->tonesize);

		for (i = 0; i < mos6560->tonesize; i++)
		{
			mos6560->tone[i] = (INT16)(sin (2 * M_PI * i / mos6560->tonesize) * 127 + 0.5);
		}
	}
	else
	{
		mos6560->tone = NULL;
	}
}


/*-------------------------------------------------
 DEVICE_START( mos6560 )
-------------------------------------------------*/

static DEVICE_START( mos6560 )
{
	mos6560_state *mos6560 = get_safe_token(device);
	const mos6560_interface *intf = (mos6560_interface *)device->static_config();
	int width, height;

	mos6560->screen = downcast<screen_device *>(device->machine().device(intf->screen));
	width = mos6560->screen->width();
	height = mos6560->screen->height();

	mos6560->type = intf->type;

	mos6560->bitmap = auto_bitmap_ind16_alloc(device->machine(), width, height);

	assert(intf->dma_read != NULL);
	assert(intf->dma_read_color != NULL);

	mos6560->dma_read = intf->dma_read;
	mos6560->dma_read_color = intf->dma_read_color;

	mos6560->lightpen_button_cb = intf->button_cb;
	mos6560->lightpen_x_cb = intf->x_cb;
	mos6560->lightpen_y_cb = intf->y_cb;

	mos6560->paddle_cb[0] = intf->paddle0_cb;
	mos6560->paddle_cb[1] = intf->paddle1_cb;

	switch (mos6560->type)
	{
	case MOS6560:
		mos6560->total_xsize = MOS6560_XSIZE;
		mos6560->total_ysize = MOS6560_YSIZE;
		mos6560->total_lines = MOS6560_LINES;
		mos6560->total_vretracerate = MOS6560_VRETRACERATE;
		break;
	case MOS6560_ATTACKUFO:
		mos6560->total_xsize = 23 * 8;
		mos6560->total_ysize = 22 * 8;
		mos6560->total_lines = MOS6560_LINES;
		mos6560->total_vretracerate = MOS6560_VRETRACERATE;
		break;
	case MOS6561:
		mos6560->total_xsize = MOS6561_XSIZE;
		mos6560->total_ysize = MOS6561_YSIZE;
		mos6560->total_lines = MOS6561_LINES;
		mos6560->total_vretracerate = MOS6561_VRETRACERATE;
		break;
	}

	mos6560_sound_start(device);

	device->save_item(NAME(mos6560->lightpenreadtime));
	device->save_item(NAME(mos6560->rasterline));
	device->save_item(NAME(mos6560->lastline));

	device->save_item(NAME(mos6560->charheight));
	device->save_item(NAME(mos6560->matrix8x16));
	device->save_item(NAME(mos6560->inverted));
	device->save_item(NAME(mos6560->chars_x));
	device->save_item(NAME(mos6560->chars_y));
	device->save_item(NAME(mos6560->xsize));
	device->save_item(NAME(mos6560->ysize));
	device->save_item(NAME(mos6560->xpos));
	device->save_item(NAME(mos6560->ypos));
	device->save_item(NAME(mos6560->chargenaddr));
	device->save_item(NAME(mos6560->videoaddr));

	device->save_item(NAME(mos6560->backgroundcolor));
	device->save_item(NAME(mos6560->framecolor));
	device->save_item(NAME(mos6560->helpercolor));

	device->save_item(NAME(mos6560->reg));

	device->save_item(NAME(mos6560->mono));
	device->save_item(NAME(mos6560->monoinverted));
	device->save_item(NAME(mos6560->multi));
	device->save_item(NAME(mos6560->multiinverted));

	device->save_item(NAME(mos6560->last_data));

	device->save_item(NAME(*mos6560->bitmap));

	device->save_item(NAME(mos6560->tone1pos));
	device->save_item(NAME(mos6560->tone2pos));
	device->save_item(NAME(mos6560->tone3pos));
	device->save_item(NAME(mos6560->tone1samples));
	device->save_item(NAME(mos6560->tone2samples));
	device->save_item(NAME(mos6560->tone3samples));
	device->save_item(NAME(mos6560->noisepos));
	device->save_item(NAME(mos6560->noisesamples));
}

/*-------------------------------------------------
 DEVICE_RESET( mos6560 )
-------------------------------------------------*/

static DEVICE_RESET( mos6560 )
{
	mos6560_state *mos6560 = get_safe_token(device);

	mos6560->lightpenreadtime = 0.0;
	mos6560->rasterline = 0;
	mos6560->lastline = 0;

	memset(mos6560->reg, 0, 16);

	mos6560->charheight = 8;
	mos6560->matrix8x16 = 0;
	mos6560->inverted = 0;
	mos6560->chars_x = 0;
	mos6560->chars_y = 0;
	mos6560->xsize = 0;
	mos6560->ysize = 0;
	mos6560->xpos = 0;
	mos6560->ypos = 0;
	mos6560->chargenaddr = 0;
	mos6560->videoaddr = 0;

	mos6560->backgroundcolor = 0;
	mos6560->framecolor = 0;
	mos6560->helpercolor = 0;

	mos6560->mono[0] = 0;
	mos6560->mono[1] = 0;
	mos6560->monoinverted[0] = 0;
	mos6560->monoinverted[1] = 0;
	mos6560->multi[0] = 0;
	mos6560->multi[1] = 0;
	mos6560->multi[2] = 0;
	mos6560->multi[3] = 0;
	mos6560->multiinverted[0] = 0;
	mos6560->multiinverted[1] = 0;
	mos6560->multiinverted[2] = 0;
	mos6560->multiinverted[3] = 0;

	mos6560->tone1pos = 0;
	mos6560->tone2pos = 0;
	mos6560->tone3pos = 0;
	mos6560->tone1samples = 1;
	mos6560->tone2samples = 1;
	mos6560->tone3samples = 1;
	mos6560->noisepos = 0;
	mos6560->noisesamples = 1;
}

const device_type MOS656X = &device_creator<mos6560_device>;

mos6560_device::mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS656X, "MOS 6560 / 6561 VIC", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(mos6560_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6560_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6560_device::device_start()
{
	DEVICE_START_NAME( mos6560 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6560_device::device_reset()
{
	DEVICE_RESET_NAME( mos6560 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mos6560_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


