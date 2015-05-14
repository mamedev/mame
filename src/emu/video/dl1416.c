// license:GPL-2.0+
// copyright-holders:Dirk Best
/*****************************************************************************
 *
 *  DL1416
 *
 *
 * 4-Digit 16-Segment Alphanumeric Intelligent Display
 * with Memory/Decoder/Driver
 *
 * Notes:
 *   - Currently supports the DL1416T and by virtue of it being nearly the same, the DL1414.
 *   - Partial support for DL1416B is available, it just needs the right
 *     character set and MAME core support for its display.
 *   - Cursor support is implemented but not tested, as the AIM65 does not
 *     seem to use it.
 *
 * Todo:
 *   - Is the DL1416A identical to the DL1416T? If not, we need to add
 *     support for it.
 *   - Add proper support for DL1414 (pretty much DL1416T without the cursor)
 *
 * Changes:
 *   - 2007-07-30: Initial version.  [Dirk Best]
 *   - 2008-02-25: Converted to the new device interface.  [Dirk Best]
 *   - 2008-12-18: Cleanups.  [Dirk Best]
 *   - 2011-10-08: Changed the ram to store character rather than segment data. [Lord Nightmare]
 *
 *
 * We use the following order for the segments:
 *
 *   000 111
 *  7D  A  E2
 *  7 D A E 2
 *  7  DAE  2
 *   888 999
 *  6  CBF  3
 *  6 C B F 3
 *  6C  B  F3
 *   555 444
 *
 ****************************************************************************/

#include "emu.h"
#include "dl1416.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SEG_UNDEF  (0xfffe)
#define SEG_BLANK  (0)
#define SEG_CURSOR (0xffff)
#define CURSOR_ON  (1)
#define CURSOR_OFF (0)

/* character set DL1416T */
static const UINT16 dl1416t_segments[128] = {
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	0x0000,    0x2421,    0x0480,    0x0f3c,    /*   ! " # */
	0x0fbb,    0x5f99,    0xa579,    0x4000,    /* $ % & ' */
	0xc000,    0x3000,    0xff00,    0x0f00,    /* ( ) * + */
	0x1000,    0x0300,    0x0020,    0x5000,    /* , - . / */
	0x0ce1,    0x0c00,    0x0561,    0x0d21,    /* 0 1 2 3 */
	0x0d80,    0x09a1,    0x09e1,    0x0c01,    /* 4 5 6 7 */
	0x0de1,    0x0da1,    0x0021,    0x1001,    /* 8 9 : ; */
	0x5030,    0x0330,    0xa030,    0x0a07,    /* < = > ? */
	0x097f,    0x03cf,    0x0e3f,    0x00f3,    /* @ A B C */
	0x0c3f,    0x01f3,    0x01c3,    0x02fb,    /* D E F G */
	0x03cc,    0x0c33,    0x0c63,    0xc1c0,    /* H I J K */
	0x00f0,    0x60cc,    0xa0cc,    0x00ff,    /* L M N O */
	0x03c7,    0x80ff,    0x83c7,    0x03bb,    /* P Q R S */
	0x0c03,    0x00fc,    0x50c0,    0x90cc,    /* T U V W */
	0xf000,    0x6800,    0x5033,    0x00e1,    /* X Y Z [ */
	0xa000,    0x001e,    0x9000,    0x0030,    /* \ ] ^ _ */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, /* undefined */
	SEG_UNDEF, SEG_UNDEF, SEG_UNDEF, SEG_UNDEF  /* undefined */
};


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

dl1416_device::dl1416_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_write_enable(FALSE),
	m_chip_enable(FALSE),
	m_cursor_enable(FALSE),
	m_update(*this)
{
	for (int i = 0; i < 4; i++)
	{
		m_digit_ram[i] = 0;
		m_cursor_state[i] = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dl1416_device::device_start()
{
	/* register for state saving */
	save_item(NAME(m_chip_enable));
	save_item(NAME(m_cursor_enable));
	save_item(NAME(m_write_enable));
	save_item(NAME(m_digit_ram));

	m_update.resolve();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dl1416_device::device_reset()
{
	int i;
	UINT16 pattern;
	/* disable all lines */
	m_chip_enable = FALSE;
	m_write_enable = FALSE;
	m_cursor_enable = FALSE;

	/* randomize digit and cursor memory */
	for (i = 0; i < 4; i++)
	{
		m_digit_ram[i] = machine().rand()&0x3F;
		// TODO: only enable the following line if the device actually has a cursor (DL1416T and DL1416B), if DL1414 then cursor is always 0!
		//m_cursor_state[i] = ((device->machine().rand()&0xFF) >= 0x80) ? CURSOR_ON : CURSOR_OFF;
		m_cursor_state[i] = CURSOR_OFF;
		pattern = dl1416t_segments[m_digit_ram[i]];

		/* If cursor for this digit position is enabled and segment is not */
		/* undefined, replace digit with cursor */
		if ((m_cursor_state[i] == CURSOR_ON) && (pattern != SEG_UNDEF))
			pattern = SEG_CURSOR;

		/* Undefined characters are replaced by blanks */
		if (pattern == SEG_UNDEF)
			pattern = SEG_BLANK;

		/* Call update function */
		if (!m_update.isnull())
			m_update((offs_t)i, pattern);
	}
}


const device_type DL1416B = &device_creator<dl1416b_device>;

dl1416b_device::dl1416b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dl1416_device(mconfig, DL1416B, "DL1416B", tag, owner, clock, "dl1416b", __FILE__)
{
}


const device_type DL1416T = &device_creator<dl1416t_device>;

dl1416t_device::dl1416t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dl1416_device(mconfig, DL1416T, "DL1416T", tag, owner, clock, "dl1416t", __FILE__)
{
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

/* write enable, active low */
WRITE_LINE_MEMBER( dl1416_device::wr_w )
{
	m_write_enable = !state;
}

/* chip enable, active low */
WRITE_LINE_MEMBER( dl1416_device::ce_w )
{
	m_chip_enable = !state;
}

/* cursor enable, active low */
WRITE_LINE_MEMBER( dl1416_device::cu_w )
{
	m_cursor_enable = !state;
}

/* data */
WRITE8_MEMBER( dl1416_device::data_w )
{
	offset &= 0x03; /* A0-A1 */
	data &= 0x7f;   /* D0-D6 */

	/* Only try to update the data if we are enabled and write is enabled */
	if (m_chip_enable && m_write_enable)
	{
		/* fprintf(stderr,"DL1416 Write: Cursor: %d, Offset: %d, Data: %02X\n (%c)", m_cursor_enable, offset, data, data); */
		int i, pattern, previous_state;

		if (m_cursor_enable) /* cursor enable is set */
		{
			if (type() == DL1416B)
			{
				/* DL1416B uses offset to decide cursor pos to change and D0 to hold new state */

				/* The cursor will be set if D0 is high and the original */
				/* character restored otherwise */
				previous_state = m_cursor_state[offset];
				m_cursor_state[offset] = data & 1 ? CURSOR_ON : CURSOR_OFF;

				if (previous_state != m_cursor_state[offset])
				{
					pattern = dl1416t_segments[m_digit_ram[offset]];

					/* If cursor for this digit position is enabled and segment is not */
					/* undefined, replace digit with cursor */
					if ((m_cursor_state[offset] == CURSOR_ON) && (pattern != SEG_UNDEF))
						pattern = SEG_CURSOR;

					/* Undefined characters are replaced by blanks */
					if (pattern == SEG_UNDEF)
						pattern = SEG_BLANK;

					/* Call update function */
					if (!m_update.isnull())
						m_update(offset, pattern, mem_mask);
				}
			}
			else {
			/* DL1416T uses a bitmap of 4 data bits D0,D1,D2,D3 to decide cursor pos to change and new state */

				for (i = 0; i < 4; i++)
				{
					/* The cursor will be set if D0-D3 is high and the original */
					/* character at the appropriate position restored otherwise */
					previous_state = m_cursor_state[i];
					m_cursor_state[i] = data & (1<<i) ? CURSOR_ON : CURSOR_OFF;

					if (previous_state != m_cursor_state[i])
					{
						pattern = dl1416t_segments[m_digit_ram[i]];

						/* If cursor for this digit position is enabled and segment is not */
						/* undefined, replace digit with cursor */
						if ((m_cursor_state[i] == CURSOR_ON) && (pattern != SEG_UNDEF))
							pattern = SEG_CURSOR;

						/* Undefined characters are replaced by blanks */
						if (pattern == SEG_UNDEF)
							pattern = SEG_BLANK;

						/* Call update function */
						if (!m_update.isnull())
							m_update(i, pattern, mem_mask);
					}
				}
			}
		}
		else /* cursor enable is not set, so standard write */
		{
			/* Save written value */
			m_digit_ram[offset] = data&0x3f;

			/* Load segment pattern from ROM */
			pattern = dl1416t_segments[data]; /** TODO: handle DL1416T vs DL1416B vs DL1414 here */

			/* If cursor for this digit position is enabled and segment is not */
			/* undefined, replace digit with cursor */
			if ((m_cursor_state[offset] == CURSOR_ON) && (pattern != SEG_UNDEF))
				pattern = SEG_CURSOR;

			/* Undefined characters are replaced by blanks */
			if (pattern == SEG_UNDEF)
				pattern = SEG_BLANK;

			/* Call update function */
			if (!m_update.isnull())
				m_update(offset, pattern, mem_mask);
		}
	}
}
