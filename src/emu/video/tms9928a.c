/*
** File: tms9928a.c -- software implementation of the Texas Instruments
**                     TMS9918(A), TMS9928(A) and TMS9929(A), used by the Coleco, MSX and
**                     TI99/4(A).
**
** All undocumented features as described in the following file
** should be emulated.
**
** http://bifi.msxnet.org/msxnet/tech/tms9918a.txt
**
** By Sean Young 1999 (sean@msxnet.org).
** Based on code by Mike Balfour.
** Improved over the years by MESS and MAME teams.
**
** Todo:
** - Colours are incorrect. [fixed by R Nabet ?]
** - Sprites 8-31 are ghosted/cloned in mode 3 when using less than
**   three pattern tables. Exact behaviour is not known.
*/

#include "emu.h"
#include "tms9928a.h"
#include "machine/devhelpr.h"


const device_type TMS9928A = &device_creator<tms9928a_device>;
const device_type TMS9918  = &device_creator<tms9918_device>;
const device_type TMS9918A = &device_creator<tms9918a_device>;
const device_type TMS9118  = &device_creator<tms9118_device>;
const device_type TMS9128  = &device_creator<tms9128_device>;
const device_type TMS9929  = &device_creator<tms9929_device>;
const device_type TMS9929A = &device_creator<tms9929a_device>;
const device_type TMS9129  = &device_creator<tms9129_device>;


/*
    New palette (R. Nabet).

    First 3 columns from TI datasheet (in volts).
    Next 3 columns based on formula :
        Y = .299*R + .587*G + .114*B (NTSC)
    (the coefficients are likely to be slightly different with PAL, but who cares ?)
    I assumed the "zero" for R-Y and B-Y was 0.47V.
    Last 3 coeffs are the 8-bit values.

    Color            Y      R-Y     B-Y     R       G       B       R   G   B
    0 Transparent
    1 Black         0.00    0.47    0.47    0.00    0.00    0.00      0   0   0
    2 Medium green  0.53    0.07    0.20    0.13    0.79    0.26     33 200  66
    3 Light green   0.67    0.17    0.27    0.37    0.86    0.47     94 220 120
    4 Dark blue     0.40    0.40    1.00    0.33    0.33    0.93     84  85 237
    5 Light blue    0.53    0.43    0.93    0.49    0.46    0.99    125 118 252
    6 Dark red      0.47    0.83    0.30    0.83    0.32    0.30    212  82  77
    7 Cyan          0.73    0.00    0.70    0.26    0.92    0.96     66 235 245
    8 Medium red    0.53    0.93    0.27    0.99    0.33    0.33    252  85  84
    9 Light red     0.67    0.93    0.27    1.13(!) 0.47    0.47    255 121 120
    A Dark yellow   0.73    0.57    0.07    0.83    0.76    0.33    212 193  84
    B Light yellow  0.80    0.57    0.17    0.90    0.81    0.50    230 206 128
    C Dark green    0.47    0.13    0.23    0.13    0.69    0.23     33 176  59
    D Magenta       0.53    0.73    0.67    0.79    0.36    0.73    201  91 186
    E Gray          0.80    0.47    0.47    0.80    0.80    0.80    204 204 204
    F White         1.00    0.47    0.47    1.00    1.00    1.00    255 255 255
*/
static const rgb_t tms9928a_palette[16] =
{
	RGB_BLACK,
	RGB_BLACK,
	MAKE_RGB(33, 200, 66),
	MAKE_RGB(94, 220, 120),
	MAKE_RGB(84, 85, 237),
	MAKE_RGB(125, 118, 252),
	MAKE_RGB(212, 82, 77),
	MAKE_RGB(66, 235, 245),
	MAKE_RGB(252, 85, 84),
	MAKE_RGB(255, 121, 120),
	MAKE_RGB(212, 193, 84),
	MAKE_RGB(230, 206, 128),
	MAKE_RGB(33, 176, 59),
	MAKE_RGB(201, 91, 186),
	MAKE_RGB(204, 204, 204),
	RGB_WHITE
};


/*
** initialize the palette
*/
PALETTE_INIT( tms9928a )
{
	palette_set_colors(machine, 0, tms9928a_palette, TMS9928A_PALETTE_SIZE);
}


tms9928a_device::tms9928a_device( const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_50hz, bool is_reva )
	: device_t( mconfig, type, name, tag, owner, clock )
{
	m_50hz = is_50hz;
	m_reva = is_reva;
}


tms9928a_device::tms9928a_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t( mconfig, TMS9928A, "tms9928a", tag, owner, clock )
{
	m_50hz = false;
	m_reva = true;
}


READ8_MEMBER( tms9928a_device::vram_read )
{
	UINT8 data = m_ReadAhead;

	m_ReadAhead = m_vMem[ m_Addr ];
	m_Addr = (m_Addr + 1) & (m_vram_size - 1);
	m_latch = 0;

	return data;
}


WRITE8_MEMBER( tms9928a_device::vram_write )
{
	m_vMem[ m_Addr ] = data;
	m_Addr = (m_Addr + 1) & (m_vram_size - 1);
	m_ReadAhead = data;
	m_latch = 0;
}


READ8_MEMBER( tms9928a_device::register_read )
{
	UINT8 data = m_StatusReg;

	m_StatusReg = m_FifthSprite;
	check_interrupt();
	m_latch = 0;

	return data;
}


void tms9928a_device::check_interrupt()
{
	// trigger if vblank and interrupt-enable bits are set
	UINT8 b = (m_StatusReg & 0x80 && m_Regs[1] & 0x20) ? 1 : 0;

	if (b != m_INT)
	{
		m_INT = b;
		if ( !m_irq_changed.isnull() )
			m_irq_changed( m_INT );
	}
}


void tms9928a_device::change_register(UINT8 reg, UINT8 val)
{
	static const UINT8 Mask[8] =
		{ 0x03, 0xfb, 0x0f, 0xff, 0x07, 0x7f, 0x07, 0xff };
	static const char *const modes[] =
	{
		"Mode 0 (GRAPHIC 1)", "Mode 1 (TEXT 1)", "Mode 2 (GRAPHIC 2)",
		"Mode 1+2 (TEXT 1 variation)", "Mode 3 (MULTICOLOR)",
		"Mode 1+3 (BOGUS)", "Mode 2+3 (MULTICOLOR variation)",
		"Mode 1+2+3 (BOGUS)"
	};

	val &= Mask[reg];
	m_Regs[reg] = val;

	logerror("TMS9928A('%s'): Reg %d = %02xh\n", tag(), reg, (int)val);

	switch (reg)
	{
	case 0:
		/* re-calculate masks and pattern generator & colour */
		if (val & 2)
		{
			m_colour = ((m_Regs[3] & 0x80) * 64) & (m_vram_size - 1);
			m_colourmask = (m_Regs[3] & 0x7f) * 8 | 7;
			m_pattern = ((m_Regs[4] & 4) * 2048) & (m_vram_size - 1);
			m_patternmask = ( (m_Regs[4] & 3) << 8 ) | (m_colourmask & 0xff);
		}
		else
		{
			m_colour = (m_Regs[3] * 64) & (m_vram_size - 1);
			m_pattern = (m_Regs[4] * 2048) & (m_vram_size - 1);
		}
		m_mode = ( (m_reva ? (m_Regs[0] & 2) : 0) | ((m_Regs[1] & 0x10)>>4) | ((m_Regs[1] & 8)>>1));
		logerror("TMS9928A('%s'): %s\n", tag(), modes[m_mode]);
		break;
	case 1:
		check_interrupt();
		m_mode = ( (m_reva ? (m_Regs[0] & 2) : 0) | ((m_Regs[1] & 0x10)>>4) | ((m_Regs[1] & 8)>>1));
		logerror("TMS9928A('%s'): %s\n", tag(), modes[m_mode]);
		break;
	case 2:
		m_nametbl = (val * 1024) & (m_vram_size - 1);
		break;
	case 3:
		if (m_Regs[0] & 2)
		{
			m_colour = ((val & 0x80) * 64) & (m_vram_size - 1);
			m_colourmask = ( (val & 0x7f) * 8 ) | 7;
		}
		else
		{
			m_colour = (val * 64) & (m_vram_size - 1);
		}
		m_patternmask = ( (m_Regs[4] & 3) * 256 ) | (m_colourmask & 255);
		break;
	case 4:
		if (m_Regs[0] & 2)
		{
			m_pattern = ((val & 4) * 2048) & (m_vram_size - 1);
			m_patternmask = ( (val & 3) * 256 ) | 255;
		}
		else
		{
			m_pattern = (val * 2048) & (m_vram_size - 1);
		}
		break;
	case 5:
		m_spriteattribute = (val * 128) & (m_vram_size - 1);
		break;
	case 6:
		m_spritepattern = (val * 2048) & (m_vram_size - 1);
		break;
	case 7:
		/* The backdrop is updated at TMS9928A_refresh() */
		break;
	}
}


WRITE8_MEMBER( tms9928a_device::register_write )
{
	if (m_latch) {
		/* set high part of read/write address */
		m_Addr = ((data << 8) | (m_Addr & 0xff)) & (m_vram_size - 1);

		if (data & 0x80) {
			/* register write */
			change_register (data & 7, m_Addr & 0xff);
		} else {
			if ( !(data & 0x40) ) {
				/* read ahead */
				vram_read(space, 0);
			}
		}
		m_latch = 0;
	} else {
		/* set low part of read/write address */
		m_Addr = ((m_Addr & 0xff00) | data) & (m_vram_size - 1);
		m_latch = 1;
	}
}


void tms9928a_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int vpos = m_screen->vpos();
	UINT16 BackColour = m_Regs[7] & 15;
	UINT16 *p = BITMAP_ADDR16( m_tmpbmp, vpos, 0 );

	if (!BackColour)
		BackColour=1;

	if ( vpos == 0 )
	{
		rgb_t oldcolor = palette_get_color(machine(), 0);

		/* note we preserve the alpha here; this is so that it can be controlled independently */
		/* see cliffhgr.c for an example */
		palette_set_color(machine(), 0, (tms9928a_palette[BackColour] & MAKE_ARGB(0,255,255,255)) | (oldcolor & MAKE_ARGB(255,0,0,0)));
	}

	int y = vpos - m_top_border;

	if ( y < 0 || y >= 192 || ! (m_Regs[1] & 0x40) )
	{
		/* Draw backdrop colour */
		for ( int i = 0; i < TMS9928A_TOTAL_HORZ; i++ )
			p[i] = BackColour;

		/* Check for end of active display */
		if ( y == 192 )
		{
			m_StatusReg |= 0x80;
			check_interrupt();
		}
	}
	else
	{
		/* Draw regular line */

		/* Left border */
		for ( int i = 0; i < TMS9928A_HORZ_DISPLAY_START; i++ )
			p[i] = BackColour;

		/* Active display */

		switch( m_mode )
		{
		case 0:				/* MODE 0 */
			// if (vpos==100 ) popmessage("TMS9928A MODE 0");
			{
				UINT16 addr = m_nametbl + ( ( y & 0xF8 ) << 2 );

				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 256; x+= 8, addr++ )
				{
					UINT8 charcode = m_vMem[ addr ];
					UINT8 pattern = m_vMem[ m_pattern + ( charcode << 3 ) + ( y & 7 ) ];
					UINT8 colour = m_vMem[ m_colour + ( charcode >> 3 ) ];
					UINT16 fg = colour >> 4;
					UINT16 bg = colour & 15;

					for ( int i = 0; i < 8; pattern <<= 1, i++ )
						p[x+i] = ( pattern & 0x80 ) ? fg : bg;
				}
			}
			break;

		case 1:				/* MODE 1 */
			//if (vpos==100 ) popmessage("TMS9928A MODE 1");
			{
				UINT16 addr = m_nametbl + ( ( y >> 3 ) * 40 );
				UINT16 fg = m_Regs[7] >> 4;
				UINT16 bg = m_Regs[7] & 15;

				/* Extra 6 pixels left border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 6; x++ )
					p[x] = bg;

				for ( int x = TMS9928A_HORZ_DISPLAY_START + 6; x < TMS9928A_HORZ_DISPLAY_START + 246; x+= 6, addr++ )
				{
					UINT16 charcode = m_vMem[ addr ];
					UINT8 pattern = m_vMem[ m_pattern + ( charcode << 3 ) + ( y & 7 ) ];

					for ( int i = 0; i < 6; pattern <<= 1, i++ )
						p[x+i] = ( pattern & 0x80 ) ? fg : bg;
				}

				/* Extra 10 pixels right border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START + 246; x < TMS9928A_HORZ_DISPLAY_START + 256; x++ )
					p[x] = bg;
			}
			break;

		case 2:				/* MODE 2 */
			//if (vpos==100 ) popmessage("TMS9928A MODE 2");
			{
				UINT16 addr = m_nametbl + ( ( y >> 3 ) * 32 );

				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 256; x+= 8, addr++ )
				{
					UINT16 charcode = m_vMem[ addr ] + ( ( y >> 6 ) << 8 );
					UINT8 pattern = m_vMem[ m_pattern + ( ( charcode & m_patternmask ) << 3 ) + ( y & 7 ) ];
					UINT8 colour = m_vMem[ m_colour + ( ( charcode & m_colourmask ) << 3 ) + ( y & 7 ) ];
					UINT16 fg = colour >> 4;
					UINT16 bg = colour & 15;

					for ( int i = 0; i < 8; pattern <<= 1, i++ )
						p[x+i] = ( pattern & 0x80 ) ? fg : bg;
				}
			}
			break;

		case 3:				/* MODE 1+2 */
			//if (vpos==100) popmessage("TMS9928A MODE1+2");
			{
				UINT16 addr = m_nametbl + ( ( y >> 3 ) * 40 );
				UINT16 fg = m_Regs[7] >> 4;
				UINT16 bg = m_Regs[7] & 15;

				/* Extra 6 pixels left border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 6; x++ )
					p[x] = bg;

				for ( int x = TMS9928A_HORZ_DISPLAY_START + 6; x < TMS9928A_HORZ_DISPLAY_START + 246; x+= 6, addr++ )
				{
					UINT16 charcode = ( m_vMem[ addr ] + ( ( y >> 6 ) << 8 ) ) & m_patternmask;
					UINT8 pattern = m_vMem[ m_pattern + ( charcode << 3 ) + ( y & 7 ) ];

					for ( int i = 0; i < 6; pattern <<= 1, i++ )
						p[x+i] = ( pattern & 0x80 ) ? fg : bg;
				}

				/* Extra 10 pixels right border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START + 246; x < TMS9928A_HORZ_DISPLAY_START + 256; x++ )
					p[x] = bg;
			}
			break;

		case 4:				/* MODE 3 */
			//if (vpos==100 ) popmessage("TMS9928A MODE 3");
			{
				UINT16 addr = m_nametbl + ( ( y >> 3 ) * 32 );

				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 256; x+= 8, addr++ )
				{
					UINT8 charcode = m_vMem[ addr ];
					UINT8 colour = m_vMem[ m_pattern + ( charcode << 3 ) + ( ( y >> 2 ) & 7 ) ];
					UINT16 fg = colour >> 4;
					UINT16 bg = colour & 15;

					p[x+0] = p[x+1] = p[x+2] = p[x+3] = fg;
					p[x+4] = p[x+5] = p[x+6] = p[x+7] = bg;
				}
			}
			break;

		case 5:	case 7:		/* MODE bogus */
			//if (vpos==100 ) popmessage("TMS9928A MODE bogus");
			{
				UINT16 fg = m_Regs[7] >> 4;
				UINT16 bg = m_Regs[7] & 15;

				/* Extra 6 pixels left border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 6; x++ )
					p[x] = bg;

				for ( int x = TMS9928A_HORZ_DISPLAY_START + 6; x < TMS9928A_HORZ_DISPLAY_START + 246; x+= 6 )
				{
					p[x+0] = p[x+1] = p[x+2] = p[x+3] = fg;
					p[x+4] = p[x+5] = bg;
				}

				/* Extra 10 pixels right border */
				for ( int x = TMS9928A_HORZ_DISPLAY_START + 246; x < TMS9928A_HORZ_DISPLAY_START + 256; x++ )
					p[x] = bg;
			}
			break;

		case 6:				/* MODE 2+3 */
			//if (vpos==100 ) popmessage("TMS9928A MODE 2+3");
			{
				UINT16 addr = m_nametbl + ( ( y >> 3 ) * 32 );

				for ( int x = TMS9928A_HORZ_DISPLAY_START; x < TMS9928A_HORZ_DISPLAY_START + 256; x+= 8, addr++ )
				{
					UINT8 charcode = m_vMem[ addr ];
					UINT8 colour = m_vMem[ m_pattern + ( ( ( charcode + ( ( y >> 2 ) & 7 ) + ( ( y >> 6 ) << 8 ) ) & m_patternmask ) << 3 ) ];
					UINT16 fg = colour >> 4;
					UINT16 bg = colour & 15;

					p[x+0] = p[x+1] = p[x+2] = p[x+3] = fg;
					p[x+4] = p[x+5] = p[x+6] = p[x+7] = bg;
				}
			}
			break;
		}

		/* Draw sprites */
		if ( ( m_Regs[1] & 0x50 ) != 0x40 )
		{
			/* sprites are disabled */
			m_FifthSprite = 31;
		}
		else
		{
			UINT8 sprite_size = ( m_Regs[1] & 0x02 ) ? 16 : 8;
			UINT8 sprite_mag = m_Regs[1] & 0x01;
			UINT8 sprite_height = sprite_size * ( sprite_mag + 1 );
			UINT8 spr_drawn[32+256+32] = { 0 };
			UINT8 num_sprites = 0;
			bool fifth_encountered = false;

			for ( UINT16 sprattr = 0; sprattr < 128; sprattr += 4 )
			{
				int spr_y = m_vMem[ m_spriteattribute + sprattr + 0 ];

				m_FifthSprite = sprattr / 4;

				/* Stop processing sprites */
				if ( spr_y == 208 )
					break;

				if ( spr_y > 0xE0 )
					spr_y -= 256;

				/* vert pos 255 is displayed on the first line of the screen */
				spr_y++;

				/* is sprite enabled on this line? */
				if ( spr_y <= y && y < spr_y + sprite_height )
				{
					int spr_x = m_vMem[ m_spriteattribute + sprattr + 1 ];
					UINT8 sprcode = m_vMem[ m_spriteattribute + sprattr + 2 ];
					UINT8 sprcol = m_vMem[ m_spriteattribute + sprattr + 3 ];
					UINT16 pataddr = m_spritepattern + ( ( sprite_size == 16 ) ? sprcode & ~0x03 : sprcode ) * 8;

					num_sprites++;

					/* Fifth sprite encountered? */
					if ( num_sprites == 5 )
					{
						fifth_encountered = true;
						break;
					}

					if ( sprite_mag )
						pataddr += ( ( ( y - spr_y ) & 0x1F ) >> 1 );
					else
						pataddr += ( ( y - spr_y ) & 0x0F );

					UINT8 pattern = m_vMem[ pataddr ];

					if ( sprcol & 0x80 )
						spr_x -= 32;

					sprcol &= 0x0f;

					for ( int s = 0; s < sprite_size; s += 8 )
					{
						for ( int i = 0; i < 8; pattern <<= 1, i++ )
						{
							int colission_index = spr_x + ( sprite_mag ? i * 2 : i ) + 32;

							for ( int z = 0; z <= sprite_mag; colission_index++, z++ )
							{
								/* Check if pixel should be drawn */
								if ( pattern & 0x80 )
								{
									if ( colission_index >= 32 && colission_index < 32 + 256 )
									{
										/* Check for colission */
										if ( spr_drawn[ colission_index ] )
											m_StatusReg |= 0x20;
										spr_drawn[ colission_index ] |= 0x01;

										if ( sprcol )
										{
											/* Has another sprite already drawn here? */
											if ( ! ( spr_drawn[ colission_index ] & 0x02 ) )
											{
												spr_drawn[ colission_index ] |= 0x02;
												p[ TMS9928A_HORZ_DISPLAY_START + colission_index - 32 ] = sprcol;
											}
										}
									}
								}
							}
						}

						pattern = m_vMem[ pataddr + 16 ];
						spr_x += sprite_mag ? 16 : 8;
					}
				}
			}

			/* Update sprite overflow bits */
			if (~m_StatusReg & 0x40)
			{
				m_StatusReg = (m_StatusReg & 0xe0) | m_FifthSprite;
				if (fifth_encountered && ~m_StatusReg & 0x80)
					m_StatusReg |= 0x40;
			}
		}

		/* Right border */
		for ( int i = TMS9928A_HORZ_DISPLAY_START + 256; i < TMS9928A_TOTAL_HORZ; i++ )
			p[i] = BackColour;
	}

	/* Schedule next callback */
	m_line_timer->adjust( m_screen->time_until_pos( ( vpos + 1 ) % m_screen->height() , 0 ) );
}


void tms9928a_device::update( bitmap_t *bitmap, const rectangle *cliprect )
{
	copybitmap( bitmap, m_tmpbmp, 0, 0, 0, 0, cliprect );
}


void tms9928a_device::device_config_complete()
{
	const tms9928a_interface *intf = reinterpret_cast<const tms9928a_interface *>(static_config());

	if ( intf != NULL )
	{
		*static_cast<tms9928a_interface *>(this) = *intf;
	}
	else
	{
		m_vram_size = 0;
		memset(&m_out_int_line, 0, sizeof(m_out_int_line));
	}
}


void tms9928a_device::device_start()
{
	assert_always(((m_vram_size == 0x1000) || (m_vram_size == 0x2000) || (m_vram_size == 0x4000)), "4, 8 or 16 kB vram please");

	m_screen = machine().device<screen_device>( m_screen_tag );
	assert( m_screen != NULL );

	m_top_border = m_50hz ? TMS9928A_VERT_DISPLAY_START_PAL : TMS9928A_VERT_DISPLAY_START_NTSC;

	m_irq_changed.resolve( m_out_int_line, *this );

	/* Video RAM */
	m_vMem = auto_alloc_array_clear(machine(), UINT8, m_vram_size);

	/* back bitmap */
	m_tmpbmp = auto_bitmap_alloc(machine(), TMS9928A_TOTAL_HORZ, TMS9928A_TOTAL_VERT_PAL, m_screen->format());

	m_line_timer = timer_alloc(TIMER_LINE);

	save_item(NAME(m_Regs[0]));
	save_item(NAME(m_Regs[1]));
	save_item(NAME(m_Regs[2]));
	save_item(NAME(m_Regs[3]));
	save_item(NAME(m_Regs[4]));
	save_item(NAME(m_Regs[5]));
	save_item(NAME(m_Regs[6]));
	save_item(NAME(m_Regs[7]));
	save_item(NAME(m_StatusReg));
	save_item(NAME(m_FifthSprite));
	save_item(NAME(m_ReadAhead));
	save_item(NAME(m_latch));
	save_item(NAME(m_Addr));
	save_item(NAME(m_INT));
	save_pointer(NAME(m_vMem), m_vram_size);
	save_item(NAME(m_colour));
	save_item(NAME(m_colourmask));
	save_item(NAME(m_pattern));
	save_item(NAME(m_patternmask));
	save_item(NAME(m_nametbl));
	save_item(NAME(m_spriteattribute));
	save_item(NAME(m_spritepattern));
	save_item(NAME(m_mode));
}


void tms9928a_device::device_reset()
{
	for ( int i = 0; i < 8; i++ )
		m_Regs[i] = 0;

	m_StatusReg = 0;
	m_FifthSprite = 31;
	m_nametbl = 0;
	m_pattern = 0;
	m_colour = 0;
	m_spritepattern = 0;
	m_spriteattribute = 0;
	m_colourmask = 0;
	m_patternmask = 0;
	m_Addr = 0;
	m_ReadAhead = 0;
	m_INT = 0;
	m_latch = 0;
	m_mode = 0;

	m_line_timer->adjust( m_screen->time_until_pos( 0, 0 ) );
}

