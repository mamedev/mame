// license:BSD-3-Clause
// copyright-holders:Peter Trauner
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


/*****************************************************************************
    PARAMETERS
*****************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", machine().time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)


/* 2008-05 FP: lightpen code needs to read input port from vc20.c */

#define LIGHTPEN_BUTTON     ((!m_lightpen_button_cb.isnull()) ? m_lightpen_button_cb(0) : 0)
#define LIGHTPEN_X_VALUE    ((!m_lightpen_x_cb.isnull()) ? m_lightpen_x_cb(0) : 0)
#define LIGHTPEN_Y_VALUE    ((!m_lightpen_y_cb.isnull()) ? m_lightpen_y_cb(0) : 0)

/* lightpen delivers values from internal counters
 * they do not start with the visual area or frame area */
#define MOS6560_X_BEGIN 38
#define MOS6560_Y_BEGIN -6             /* first 6 lines after retrace not for lightpen! */
#define MOS6561_X_BEGIN 38
#define MOS6561_Y_BEGIN -6
#define MOS656X_X_BEGIN ((m_variant == TYPE_6561) ? MOS6561_X_BEGIN : MOS6560_X_BEGIN)
#define MOS656X_Y_BEGIN ((m_variant == TYPE_6561) ? MOS6561_Y_BEGIN : MOS6560_Y_BEGIN)

#define MOS656X_MAME_XPOS ((m_variant == TYPE_6561) ? MOS6561_MAME_XPOS : MOS6560_MAME_XPOS)
#define MOS656X_MAME_YPOS ((m_variant == TYPE_6561) ? MOS6561_MAME_YPOS : MOS6560_MAME_YPOS)

/* lightpen behaviour in pal or mono multicolor not tested */
#define MOS656X_X_VALUE ((LIGHTPEN_X_VALUE + MOS656X_X_BEGIN + MOS656X_MAME_XPOS)/2)
#define MOS656X_Y_VALUE ((LIGHTPEN_Y_VALUE + MOS656X_Y_BEGIN + MOS656X_MAME_YPOS)/2)

#define MOS656X_VRETRACERATE ((m_variant == TYPE_6561) ? MOS6561_VRETRACERATE : MOS6560_VRETRACERATE)

/* ntsc 1 - 8 */
/* pal 5 - 19 */
#define XPOS (((int)m_reg[0] & 0x7f) * 4)
#define YPOS ((int)m_reg[1] * 2)

/* ntsc values >= 31 behave like 31 */
/* pal value >= 32 behave like 32 */
#define CHARS_X ((int)m_reg[2] & 0x7f)
#define CHARS_Y (((int)m_reg[3] & 0x7e) >> 1)

/* colorram and backgroundcolor are changed */
#define INVERTED (!(m_reg[0x0f] & 8))

#define CHARGENADDR (((int)m_reg[5] & 0x0f) << 10)
#define VIDEOADDR ((((int)m_reg[5] & 0xf0) << (10 - 4)) | (((int)m_reg[2] & 0x80) << (9-7)))
#define VIDEORAMSIZE (YSIZE * XSIZE)
#define CHARGENSIZE (256 * HEIGHTPIXEL)

#define HELPERCOLOR (m_reg[0x0e] >> 4)
#define BACKGROUNDCOLOR (m_reg[0x0f] >> 4)
#define FRAMECOLOR (m_reg[0x0f] & 0x07)


// VICE palette
static const rgb_t PALETTE_MOS[] =
{
	rgb_t(0x00, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0xff),
	rgb_t(0xf0, 0x00, 0x00),
	rgb_t(0x00, 0xf0, 0xf0),

	rgb_t(0x60, 0x00, 0x60),
	rgb_t(0x00, 0xa0, 0x00),
	rgb_t(0x00, 0x00, 0xf0),
	rgb_t(0xd0, 0xd0, 0x00),

	rgb_t(0xc0, 0xa0, 0x00),
	rgb_t(0xff, 0xa0, 0x00),
	rgb_t(0xf0, 0x80, 0x80),
	rgb_t(0x00, 0xff, 0xff),

	rgb_t(0xff, 0x00, 0xff),
	rgb_t(0x00, 0xff, 0x00),
	rgb_t(0x00, 0xa0, 0xff),
	rgb_t(0xff, 0xff, 0x00)
};



/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

inline UINT8 mos6560_device::read_videoram(offs_t offset)
{
	m_last_data = space(AS_0).read_byte(offset & 0x3fff);

	return m_last_data;
}

inline UINT8 mos6560_device::read_colorram(offs_t offset)
{
	return space(AS_1).read_byte(offset & 0x3ff);
}

/*-------------------------------------------------
 draw_character
-------------------------------------------------*/

void mos6560_device::draw_character( int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color )
{
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = read_videoram((m_chargenaddr + ch * m_charheight + y) & 0x3fff);

		m_bitmap.pix32(y + yoff, xoff + 0) = PALETTE_MOS[color[code >> 7]];
		m_bitmap.pix32(y + yoff, xoff + 1) = PALETTE_MOS[color[(code >> 6) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 2) = PALETTE_MOS[color[(code >> 5) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 3) = PALETTE_MOS[color[(code >> 4) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 4) = PALETTE_MOS[color[(code >> 3) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 5) = PALETTE_MOS[color[(code >> 2) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 6) = PALETTE_MOS[color[(code >> 1) & 1]];
		m_bitmap.pix32(y + yoff, xoff + 7) = PALETTE_MOS[color[code & 1]];
	}
}


/*-------------------------------------------------
 draw_character_multi
-------------------------------------------------*/

void mos6560_device::draw_character_multi( int ybegin, int yend, int ch, int yoff, int xoff, UINT16 *color )
{
	int y, code;

	for (y = ybegin; y <= yend; y++)
	{
		code = read_videoram((m_chargenaddr + ch * m_charheight + y) & 0x3fff);

		m_bitmap.pix32(y + yoff, xoff + 0) =
			m_bitmap.pix32(y + yoff, xoff + 1) = PALETTE_MOS[color[code >> 6]];
		m_bitmap.pix32(y + yoff, xoff + 2) =
			m_bitmap.pix32(y + yoff, xoff + 3) = PALETTE_MOS[color[(code >> 4) & 3]];
		m_bitmap.pix32(y + yoff, xoff + 4) =
			m_bitmap.pix32(y + yoff, xoff + 5) = PALETTE_MOS[color[(code >> 2) & 3]];
		m_bitmap.pix32(y + yoff, xoff + 6) =
			m_bitmap.pix32(y + yoff, xoff + 7) = PALETTE_MOS[color[code & 3]];
	}
}


/*-------------------------------------------------
 drawlines - draw a certain numer of lines
-------------------------------------------------*/

void mos6560_device::drawlines( int first, int last )
{
	int line, vline;
	int offs, yoff, xoff, ybegin, yend, i, j;
	int attr, ch;

	m_lastline = last;
	if (first >= last)
		return;

	for (line = first; (line < m_ypos) && (line < last); line++)
	{
		for (j = 0; j < m_total_xsize; j++)
			m_bitmap.pix32(line, j) = PALETTE_MOS[m_framecolor];
	}

	for (vline = line - m_ypos; (line < last) && (line < m_ypos + m_ysize);)
	{
		if (m_matrix8x16)
		{
			offs = (vline >> 4) * m_chars_x;
			yoff = (vline & ~0xf) + m_ypos;
			ybegin = vline & 0xf;
			yend = (vline + 0xf < last - m_ypos) ? 0xf : ((last - line) & 0xf) + ybegin;
		}
		else
		{
			offs = (vline >> 3) * m_chars_x;
			yoff = (vline & ~7) + m_ypos;
			ybegin = vline & 7;
			yend = (vline + 7 < last - m_ypos) ? 7 : ((last - line) & 7) + ybegin;
		}

		if (m_xpos > 0)
		{
			for (i = ybegin; i <= yend; i++)
				for (j = 0; j < m_xpos; j++)
					m_bitmap.pix32(yoff + i, j) = PALETTE_MOS[m_framecolor];
		}

		for (xoff = m_xpos; (xoff < m_xpos + m_xsize) && (xoff < m_total_xsize); xoff += 8, offs++)
		{
			ch = read_videoram((m_videoaddr + offs) & 0x3fff);

			attr = (read_colorram((m_videoaddr + offs) & 0x3fff)) & 0xf;

			if (m_variant == TYPE_ATTACK_UFO)
			{
				/* the mos6560 variant used in attckufo only has only one draw mode */
				m_mono[1] = attr;
				draw_character(ybegin, yend, ch, yoff, xoff, m_mono);
			}
			else if (m_inverted)
			{
				if (attr & 8)
				{
					m_multiinverted[0] = attr & 7;
					draw_character_multi(ybegin, yend, ch, yoff, xoff, m_multiinverted);
				}
				else
				{
					m_monoinverted[0] = attr;
					draw_character(ybegin, yend, ch, yoff, xoff, m_monoinverted);
				}
			}
			else
			{
				if (attr & 8)
				{
					m_multi[2] = attr & 7;
					draw_character_multi(ybegin, yend, ch, yoff, xoff, m_multi);
				}
				else
				{
					m_mono[1] = attr;
					draw_character(ybegin, yend, ch, yoff, xoff, m_mono);
				}
			}
		}

		if (xoff < m_total_xsize)
		{
			for (i = ybegin; i <= yend; i++)
				for (j = xoff; j < m_total_xsize; j++)
					m_bitmap.pix32(yoff + i, j) = PALETTE_MOS[m_framecolor];
		}

		if (m_matrix8x16)
		{
			vline = (vline + 16) & ~0xf;
			line = vline + m_ypos;
		}
		else
		{
			vline = (vline + 8) & ~7;
			line = vline + m_ypos;
		}
	}

	for (; line < last; line++)
		for (j = 0; j < m_total_xsize; j++)
			m_bitmap.pix32(line, j) = PALETTE_MOS[m_framecolor];
}


/*-------------------------------------------------
 mos6560_port_w - write to regs
-------------------------------------------------*/

WRITE8_MEMBER( mos6560_device::write )
{
	DBG_LOG(1, "mos6560_port_w", ("%.4x:%.2x\n", offset, data));

	switch (offset)
	{
	case 0xa:
	case 0xb:
	case 0xc:
	case 0xd:
	case 0xe:
		soundport_w(offset, data);
		break;
	}

	if (m_reg[offset] != data)
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
			drawlines(m_lastline, m_rasterline);
			break;
		}
		m_reg[offset] = data;

		switch (offset)
		{
		case 0:
			if ((m_variant != TYPE_ATTACK_UFO))
				m_xpos = XPOS;
			break;
		case 1:
			if ((m_variant != TYPE_ATTACK_UFO))
				m_ypos = YPOS;
			break;
		case 2:
			/* ntsc values >= 31 behave like 31 */
			/* pal value >= 32 behave like 32 */
			m_chars_x = CHARS_X;
			m_videoaddr = VIDEOADDR;
			m_xsize = CHARS_X * 8;
			break;
		case 3:
			if ((m_variant != TYPE_ATTACK_UFO))
			{
				m_matrix8x16 = data & 0x01;
				m_charheight = m_matrix8x16 ? 16 : 8;
			}
			m_chars_y = CHARS_Y;
			m_ysize = CHARS_Y * m_charheight;
			break;
		case 5:
			m_chargenaddr = CHARGENADDR;
			m_videoaddr = VIDEOADDR;
			break;
		case 0xe:
			m_multi[3] = m_multiinverted[3] = m_helpercolor = HELPERCOLOR;
			break;
		case 0xf:
			if ((m_variant != TYPE_ATTACK_UFO))
				m_inverted = INVERTED;
			m_multi[1] = m_multiinverted[1] = m_framecolor = FRAMECOLOR;
			m_mono[0] = m_monoinverted[1] = m_multi[0] = m_multiinverted[2] = m_backgroundcolor = BACKGROUNDCOLOR;
			break;
		}
	}
}

/*-------------------------------------------------
 mos6560_port_r - read from regs
-------------------------------------------------*/

READ8_MEMBER( mos6560_device::read )
{
	int val;

	switch (offset)
	{
	case 3:
		val = ((m_rasterline & 1) << 7) | (m_reg[offset] & 0x7f);
		break;
	case 4:                        /*rasterline */
		drawlines(m_lastline, m_rasterline);
		val = (m_rasterline / 2) & 0xff;
		break;
	case 6:                        /*lightpen horizontal */
	case 7:                        /*lightpen vertical */
#ifdef UNUSED_FUNCTION
		if (LIGHTPEN_BUTTON && ((machine().time().as_double() - m_lightpenreadtime) * MOS656X_VRETRACERATE >= 1))
		{
			/* only 1 update each frame */
			/* and diode must recognize light */
			if (1)
			{
				m_reg[6] = MOS656X_X_VALUE;
				m_reg[7] = MOS656X_Y_VALUE;
			}
			m_lightpenreadtime = machine().time().as_double();
		}
#endif
		val = m_reg[offset];
		break;
	case 8:                        /* poti 1 */
		val = m_read_potx(0);
		break;
	case 9:                        /* poti 2 */
		val = m_read_poty(0);
		break;
	default:
		val = m_reg[offset];
		break;
	}
	DBG_LOG(3, "mos6560_port_r", ("%.4x:%.2x\n", offset, val));
	return val;
}

WRITE_LINE_MEMBER( mos6560_device::lp_w )
{
	// TODO
}

UINT8 mos6560_device::bus_r()
{
	return m_last_data;
}

/*-------------------------------------------------
 mos6560_raster_interrupt_gen
-------------------------------------------------*/

void mos6560_device::raster_interrupt_gen()
{
	m_rasterline++;
	if (m_rasterline >= m_total_lines)
	{
		m_rasterline = 0;
		drawlines(m_lastline, m_total_lines);
		m_lastline = 0;
	}
}


/*-------------------------------------------------
 mos6560_video_update - copy the VIC bitmap to
     main screen bitmap
-------------------------------------------------*/

UINT32 mos6560_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

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

#define TONE1_ON (m_reg[0x0a] & 0x80)
#define TONE2_ON (m_reg[0x0b] & 0x80)
#define TONE3_ON (m_reg[0x0c] & 0x80)
#define NOISE_ON (m_reg[0x0d] & 0x80)
#define VOLUME (m_reg[0x0e] & 0x0f)

#define TONE_FREQUENCY_MIN  (clock()/256/128)

#define TONE1_VALUE (8 * (128 - ((m_reg[0x0a] + 1) & 0x7f)))
#define TONE1_FREQUENCY (clock()/32/TONE1_VALUE)

#define TONE2_VALUE (4 * (128 - ((m_reg[0x0b] + 1) & 0x7f)))
#define TONE2_FREQUENCY (clock()/32/TONE2_VALUE)

#define TONE3_VALUE (2 * (128 - ((m_reg[0x0c] + 1) & 0x7f)))
#define TONE3_FREQUENCY (clock()/32/TONE3_VALUE)

#define NOISE_VALUE (32 * (128 - ((m_reg[0x0d] + 1) & 0x7f)))
#define NOISE_FREQUENCY (clock()/NOISE_VALUE)

#define NOISE_FREQUENCY_MAX (clock()/32/1)


/*-------------------------------------------------
 mos6560_soundport_w - write to regs
-------------------------------------------------*/

void mos6560_device::soundport_w( int offset, int data )
{
	int old = m_reg[offset];
	m_channel->update();

	switch (offset)
	{
	case 0x0a:
		m_reg[offset] = data;
		if (!(old & 0x80) && TONE1_ON)
		{
			m_tone1pos = 0;
			m_tone1samples = machine().sample_rate() / TONE1_FREQUENCY;
			if (m_tone1samples == 0)
				m_tone1samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone1 %.2x %d\n", data, TONE1_FREQUENCY));
		break;
	case 0x0b:
		m_reg[offset] = data;
		if (!(old & 0x80) && TONE2_ON)
		{
			m_tone2pos = 0;
			m_tone2samples = machine().sample_rate() / TONE2_FREQUENCY;
			if (m_tone2samples == 0)
				m_tone2samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone2 %.2x %d\n", data, TONE2_FREQUENCY));
		break;
	case 0x0c:
		m_reg[offset] = data;
		if (!(old & 0x80) && TONE3_ON)
		{
			m_tone3pos = 0;
			m_tone3samples = machine().sample_rate() / TONE3_FREQUENCY;
			if (m_tone3samples == 0)
				m_tone3samples = 1;
		}
		DBG_LOG(1, "mos6560", ("tone3 %.2x %d\n", data, TONE3_FREQUENCY));
		break;
	case 0x0d:
		m_reg[offset] = data;
		if (NOISE_ON)
		{
			m_noisesamples = (int) ((double) NOISE_FREQUENCY_MAX * machine().sample_rate()
									* NOISE_BUFFER_SIZE_SEC / NOISE_FREQUENCY);
			DBG_LOG (1, "mos6560", ("noise %.2x %d sample:%d\n",
									data, NOISE_FREQUENCY, m_noisesamples));
			if ((double) m_noisepos / m_noisesamples >= 1.0)
			{
				m_noisepos = 0;
			}
		}
		else
		{
			m_noisepos = 0;
		}
		break;
	case 0x0e:
		m_reg[offset] = (old & ~0x0f) | (data & 0x0f);
		DBG_LOG (3, "mos6560", ("volume %d\n", data & 0x0f));
		break;
	}
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

/*-------------------------------------------------
 mos6560_sound_start - start audio emulation
     (to be called at device start)
-------------------------------------------------*/

void mos6560_device::sound_start()
{
	int i;

	m_channel = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());

	/* buffer for fastest played sample for 5 second so we have enough data for min 5 second */
	m_noisesize = NOISE_FREQUENCY_MAX * NOISE_BUFFER_SIZE_SEC;
	m_noise = auto_alloc_array(machine(), INT8, m_noisesize);
	{
		int noiseshift = 0x7ffff8;
		char data;

		for (i = 0; i < m_noisesize; i++)
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
			m_noise[i] = data;
			if (((noiseshift & 0x400000) == 0) != ((noiseshift & 0x002000) == 0))
				noiseshift = (noiseshift << 1) | 1;
			else
				noiseshift <<= 1;
		}
	}
	m_tonesize = machine().sample_rate() / TONE_FREQUENCY_MIN;

	if (m_tonesize > 0)
	{
		m_tone = auto_alloc_array(machine(), INT16, m_tonesize);

		for (i = 0; i < m_tonesize; i++)
		{
			m_tone[i] = (INT16)(sin (2 * M_PI * i / m_tonesize) * 127 + 0.5);
		}
	}
	else
	{
		m_tone = NULL;
	}
}


const device_type MOS6560 = &device_creator<mos6560_device>;
const device_type MOS6561 = &device_creator<mos6561_device>;
const device_type MOS656X_ATTACK_UFO = &device_creator<mos656x_attack_ufo_device>;

// default address maps
static ADDRESS_MAP_START( mos6560_videoram_map, AS_0, 8, mos6560_device )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mos6560_colorram_map, AS_1, 8, mos6560_device )
	AM_RANGE(0x000, 0x3ff) AM_RAM
ADDRESS_MAP_END

mos6560_device::mos6560_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_memory_interface(mconfig, *this),
		device_sound_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_variant(variant),
		m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(mos6560_videoram_map)),
		m_colorram_space_config("colorram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(mos6560_colorram_map)),
		m_read_potx(*this),
		m_read_poty(*this)
{
}

mos6560_device::mos6560_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOS6560, "MOS6560", tag, owner, clock, "mos6560", __FILE__),
		device_memory_interface(mconfig, *this),
		device_sound_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_variant(TYPE_6560),
		m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(mos6560_videoram_map)),
		m_colorram_space_config("colorram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(mos6560_colorram_map)),
		m_read_potx(*this),
		m_read_poty(*this)
{
}

mos6561_device::mos6561_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6560_device(mconfig, MOS6561, "MOS6561", tag, owner, clock, TYPE_6561, "mos6561", __FILE__) { }

mos656x_attack_ufo_device::mos656x_attack_ufo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6560_device(mconfig, MOS656X_ATTACK_UFO, "MOS656X", tag, owner, clock, TYPE_ATTACK_UFO, "mos656x_attack_ufo", __FILE__) { }


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *mos6560_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
		case AS_0: return &m_videoram_space_config;
		case AS_1: return &m_colorram_space_config;
		default: return NULL;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6560_device::device_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	// resolve callbacks
	m_read_potx.resolve_safe(0xff);
	m_read_poty.resolve_safe(0xff);

	switch (m_variant)
	{
	case TYPE_6560:
		m_total_xsize = MOS6560_XSIZE;
		m_total_ysize = MOS6560_YSIZE;
		m_total_lines = MOS6560_LINES;
		m_total_vretracerate = MOS6560_VRETRACERATE;
		break;

	case TYPE_ATTACK_UFO:
		m_total_xsize = 23 * 8;
		m_total_ysize = 22 * 8;
		m_total_lines = MOS6560_LINES;
		m_total_vretracerate = MOS6560_VRETRACERATE;
		break;

	case TYPE_6561:
		m_total_xsize = MOS6561_XSIZE;
		m_total_ysize = MOS6561_YSIZE;
		m_total_lines = MOS6561_LINES;
		m_total_vretracerate = MOS6561_VRETRACERATE;
		break;
	}

	// allocate timers
	m_line_timer = timer_alloc(TIMER_LINE);
	m_line_timer->adjust(m_screen->scan_period(), 0, m_screen->scan_period());

	// initialize sound
	sound_start();

	// state saving
	save_item(NAME(m_lightpenreadtime));
	save_item(NAME(m_rasterline));
	save_item(NAME(m_lastline));

	save_item(NAME(m_charheight));
	save_item(NAME(m_matrix8x16));
	save_item(NAME(m_inverted));
	save_item(NAME(m_chars_x));
	save_item(NAME(m_chars_y));
	save_item(NAME(m_xsize));
	save_item(NAME(m_ysize));
	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_chargenaddr));
	save_item(NAME(m_videoaddr));

	save_item(NAME(m_backgroundcolor));
	save_item(NAME(m_framecolor));
	save_item(NAME(m_helpercolor));

	save_item(NAME(m_reg));

	save_item(NAME(m_mono));
	save_item(NAME(m_monoinverted));
	save_item(NAME(m_multi));
	save_item(NAME(m_multiinverted));

	save_item(NAME(m_last_data));

	save_item(NAME(m_tone1pos));
	save_item(NAME(m_tone2pos));
	save_item(NAME(m_tone3pos));
	save_item(NAME(m_tone1samples));
	save_item(NAME(m_tone2samples));
	save_item(NAME(m_tone3samples));
	save_item(NAME(m_noisepos));
	save_item(NAME(m_noisesamples));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6560_device::device_reset()
{
	m_lightpenreadtime = 0.0;
	m_rasterline = 0;
	m_lastline = 0;

	memset(m_reg, 0, 16);

	m_charheight = 8;
	m_matrix8x16 = 0;
	m_inverted = 0;
	m_chars_x = 0;
	m_chars_y = 0;
	m_xsize = 0;
	m_ysize = 0;
	m_xpos = 0;
	m_ypos = 0;
	m_chargenaddr = 0;
	m_videoaddr = 0;

	m_backgroundcolor = 0;
	m_framecolor = 0;
	m_helpercolor = 0;

	m_mono[0] = 0;
	m_mono[1] = 0;
	m_monoinverted[0] = 0;
	m_monoinverted[1] = 0;
	m_multi[0] = 0;
	m_multi[1] = 0;
	m_multi[2] = 0;
	m_multi[3] = 0;
	m_multiinverted[0] = 0;
	m_multiinverted[1] = 0;
	m_multiinverted[2] = 0;
	m_multiinverted[3] = 0;

	m_last_data = 0;

	m_tone1pos = 0;
	m_tone2pos = 0;
	m_tone3pos = 0;
	m_tone1samples = 1;
	m_tone2samples = 1;
	m_tone3samples = 1;
	m_noisepos = 0;
	m_noisesamples = 1;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mos6560_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_LINE:
		raster_interrupt_gen();
		break;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mos6560_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i, v;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++)
	{
		v = 0;
		if (TONE1_ON /*||(m_tone1pos != 0) */ )
		{
			v += m_tone[m_tone1pos * m_tonesize / m_tone1samples];
			m_tone1pos++;
#if 0
			m_tone1pos %= m_tone1samples;
#else
			if (m_tone1pos >= m_tone1samples)
			{
				m_tone1pos = 0;
				m_tone1samples = machine().sample_rate() / TONE1_FREQUENCY;
				if (m_tone1samples == 0)
					m_tone1samples = 1;
			}
#endif
		}
		if (TONE2_ON /*||(m_tone2pos != 0) */ )
		{
			v += m_tone[m_tone2pos * m_tonesize / m_tone2samples];
			m_tone2pos++;
#if 0
			m_tone2pos %= m_tone2samples;
#else
			if (m_tone2pos >= m_tone2samples)
			{
				m_tone2pos = 0;
				m_tone2samples = machine().sample_rate() / TONE2_FREQUENCY;
				if (m_tone2samples == 0)
					m_tone2samples = 1;
			}
#endif
		}
		if (TONE3_ON /*||(m_tone3pos != 0) */ )
		{
			v += m_tone[m_tone3pos * m_tonesize / m_tone3samples];
			m_tone3pos++;
#if 0
			m_tone3pos %= m_tone3samples;
#else
			if (m_tone3pos >= m_tone3samples)
			{
				m_tone3pos = 0;
				m_tone3samples = machine().sample_rate() / TONE3_FREQUENCY;
				if (m_tone3samples == 0)
					m_tone3samples = 1;
			}
#endif
		}
		if (NOISE_ON)
		{
			v += m_noise[(int) ((double) m_noisepos * m_noisesize / m_noisesamples)];
			m_noisepos++;
			if ((double) m_noisepos / m_noisesamples >= 1.0)
			{
				m_noisepos = 0;
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
