/*****************************************************************************
 *
 * video/dl1416.c
 *
 * DL1416
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

#define SEG_UNDEF  (-2)
#define SEG_BLANK  (0)
#define SEG_CURSOR (0xffff)
#define CURSOR_ON  (1)
#define CURSOR_OFF (0)

/* character set DL1416T */
static const int dl1416t_segments[128] = {
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


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct dl1416_state
{
	int write_enable;
	int chip_enable;
	int cursor_enable;

	UINT16 digit_ram[4]; // holds the digit code for each position
	UINT8 cursor_state[4]; // holds the cursor state for each position, 0=off, 1=on
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE dl1416_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DL1416B || device->type() == DL1416T);

	return (dl1416_state *)downcast<dl1416_device *>(device)->token();
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( dl1416 )
{
	dl1416_state *dl1416 = get_safe_token(device);

	/* register for state saving */
	state_save_register_item(device->machine(), "dl1416", device->tag(), 0, dl1416->chip_enable);
	state_save_register_item(device->machine(), "dl1416", device->tag(), 0, dl1416->cursor_enable);
	state_save_register_item(device->machine(), "dl1416", device->tag(), 0, dl1416->write_enable);
	state_save_register_item_array(device->machine(), "dl1416", device->tag(), 0, dl1416->digit_ram);
}


static DEVICE_RESET( dl1416 )
{
	int i, pattern;
	dl1416_state *chip = get_safe_token(device);
	const dl1416_interface *intf = (const dl1416_interface *)device->static_config();
	/* disable all lines */
	chip->chip_enable = FALSE;
	chip->write_enable = FALSE;
	chip->cursor_enable = FALSE;

	/* randomize digit and cursor memory */
	for (i = 0; i < 4; i++)
	{
		chip->digit_ram[i] = device->machine().rand()&0x3F;
		// TODO: only enable the following line if the device actually has a cursor (DL1416T and DL1416B), if DL1414 then cursor is always 0!
		//chip->cursor_state[i] = ((device->machine().rand()&0xFF) >= 0x80) ? CURSOR_ON : CURSOR_OFF;
		chip->cursor_state[i] = CURSOR_OFF;
		pattern = dl1416t_segments[chip->digit_ram[i]];

		/* If cursor for this digit position is enabled and segment is not */
		/* undefined, replace digit with cursor */
		if ((chip->cursor_state[i] == CURSOR_ON) && (pattern != SEG_UNDEF))
			pattern = SEG_CURSOR;

		/* Undefined characters are replaced by blanks */
		if (pattern == SEG_UNDEF)
			pattern = SEG_BLANK;

		/* Call update function */
		if (intf->update)
			intf->update(device, i, pattern);
	}
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

/* write enable, active low */
WRITE_LINE_DEVICE_HANDLER( dl1416_wr_w )
{
	dl1416_state *chip = get_safe_token(device);
	chip->write_enable = !state;
}

/* chip enable, active low */
WRITE_LINE_DEVICE_HANDLER( dl1416_ce_w )
{
	dl1416_state *chip = get_safe_token(device);
	chip->chip_enable = !state;
}

/* cursor enable, active low */
WRITE_LINE_DEVICE_HANDLER( dl1416_cu_w )
{
	dl1416_state *chip = get_safe_token(device);
	chip->cursor_enable = !state;
}

/* data */
WRITE8_DEVICE_HANDLER( dl1416_data_w )
{
	dl1416_state *chip = get_safe_token(device);
	const dl1416_interface *intf = (const dl1416_interface *)device->static_config();

	offset &= 0x03; /* A0-A1 */
	data &= 0x7f;   /* D0-D6 */

	/* Only try to update the data if we are enabled and write is enabled */
	if (chip->chip_enable && chip->write_enable)
	{
		/* fprintf(stderr,"DL1416 Write: Cursor: %d, Offset: %d, Data: %02X\n (%c)", chip->cursor_enable, offset, data, data); */
		int i, pattern, previous_state;

		if (chip->cursor_enable) /* cursor enable is set */
		{
			if (device->type() == DL1416B)
			{
			 /* DL1416B uses offset to decide cursor pos to change and D0 to hold new state */

				/* The cursor will be set if D0 is high and the original */
				/* character restored otherwise */
				previous_state = chip->cursor_state[offset];
				chip->cursor_state[offset] = data & 1 ? CURSOR_ON : CURSOR_OFF;

				if (previous_state != chip->cursor_state[offset])
				{
					pattern = dl1416t_segments[chip->digit_ram[offset]];

					/* If cursor for this digit position is enabled and segment is not */
					/* undefined, replace digit with cursor */
					if ((chip->cursor_state[offset] == CURSOR_ON) && (pattern != SEG_UNDEF))
						pattern = SEG_CURSOR;

					/* Undefined characters are replaced by blanks */
					if (pattern == SEG_UNDEF)
						pattern = SEG_BLANK;

					/* Call update function */
					if (intf->update)
						intf->update(device, offset, pattern);
				}
			}
			else {
			/* DL1416T uses a bitmap of 4 data bits D0,D1,D2,D3 to decide cursor pos to change and new state */

				for (i = 0; i < 4; i++)
				{
					/* The cursor will be set if D0-D3 is high and the original */
					/* character at the appropriate position restored otherwise */
					previous_state = chip->cursor_state[i];
					chip->cursor_state[i] = data & (1<<i) ? CURSOR_ON : CURSOR_OFF;

					if (previous_state != chip->cursor_state[i])
					{
						pattern = dl1416t_segments[chip->digit_ram[i]];

						/* If cursor for this digit position is enabled and segment is not */
						/* undefined, replace digit with cursor */
						if ((chip->cursor_state[i] == CURSOR_ON) && (pattern != SEG_UNDEF))
							pattern = SEG_CURSOR;

						/* Undefined characters are replaced by blanks */
						if (pattern == SEG_UNDEF)
							pattern = SEG_BLANK;

						/* Call update function */
						if (intf->update)
							intf->update(device, i, pattern);
					}
				}			
			}
		}
		else /* cursor enable is not set, so standard write */
		{
			/* Save written value */
			chip->digit_ram[offset] = data&0x3f;

			/* Load segment pattern from ROM */
			pattern = dl1416t_segments[data]; /** TODO: handle DL1416T vs DL1416B vs DL1414 here */

			/* If cursor for this digit position is enabled and segment is not */
			/* undefined, replace digit with cursor */
			if ((chip->cursor_state[offset] == CURSOR_ON) && (pattern != SEG_UNDEF))
				pattern = SEG_CURSOR;

			/* Undefined characters are replaced by blanks */
			if (pattern == SEG_UNDEF)
				pattern = SEG_BLANK;

			/* Call update function */
			if (intf->update)
				intf->update(device, offset, pattern);
		}
	}
}

dl1416_device::dl1416_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(dl1416_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void dl1416_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dl1416_device::device_start()
{
	DEVICE_START_NAME( dl1416 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dl1416_device::device_reset()
{
	DEVICE_RESET_NAME( dl1416 )(this);
}


const device_type DL1416B = &device_creator<dl1416b_device>;

dl1416b_device::dl1416b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dl1416_device(mconfig, DL1416B, "DL1416B", tag, owner, clock)
{
}


const device_type DL1416T = &device_creator<dl1416t_device>;

dl1416t_device::dl1416t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: dl1416_device(mconfig, DL1416T, "DL1416T", tag, owner, clock)
{
}


