// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Osborne Executive driver file

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/speaker.h"
#include "machine/wd_fdc.h"
#include "machine/6821pia.h"
#include "machine/z80dart.h"
#include "machine/pit8253.h"
#include "machine/ram.h"


#define MAIN_CLOCK  23961600


class osbexec_state : public driver_device
{
public:
	osbexec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu( *this, "maincpu" ),
			m_mb8877( *this, "mb8877" ),
			m_messram( *this, RAM_TAG ),
			m_pia_0( *this, "pia_0" ),
			m_pia_1( *this, "pia_1" ),
			m_sio( *this, "sio" ),
			m_speaker( *this, "speaker" ),
			m_floppy0( *this, "mb8877:0:525ssdd" ),
			m_floppy1( *this, "mb8877:1:525ssdd" )
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<mb8877_t>  m_mb8877;
	required_device<ram_device> m_messram;
	required_device<pia6821_device> m_pia_0;
	required_device<pia6821_device> m_pia_1;
	required_device<z80dart_device> m_sio;
	required_device<speaker_sound_device>   m_speaker;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;

	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	memory_region   *m_fontram_region;
	memory_region *m_vram_region;
	UINT8   *m_fontram;
	UINT8   *m_vram;
	UINT8   *m_ram_0000;
	UINT8   *m_ram_c000;
	UINT8   m_temp_attr;
	emu_timer *m_video_timer;

	/* PIA 0 (UD12) */
	UINT8   m_pia0_porta;
	UINT8   m_pia0_portb;
	int     m_pia0_irq_state;
	int     m_pia0_cb2;         /* 60/50 */

	/* PIA 1 (UD8) */
	int     m_pia1_irq_state;

	/* Vblank counter ("RTC") */
	UINT8   m_rtc;

	void set_banks()
	{
		UINT8 *ram_ptr = m_messram->pointer();

		m_ram_0000 = ram_ptr;

		if ( m_pia0_porta & 0x01 )
			m_ram_0000 += 0x10000;

		membank( "0000" )->set_base( m_ram_0000 + 0x0000 );
		membank( "2000" )->set_base( m_ram_0000 + 0x2000 );
		membank( "4000" )->set_base( m_ram_0000 + 0x4000 );
		m_ram_c000 = m_ram_0000 + 0xc000;
		membank( "e000" )->set_base( m_ram_0000 + 0xe000 );

		if ( m_pia0_porta & 0x80 )
		{
			membank( "0000" )->set_base( memregion("maincpu")->base());
			/* When BIOS is enabled 2000-3FFF is set to the "ROM RAM" */
			membank( "2000" )->set_base( ram_ptr + 0x20000 );
		}

		if ( m_pia0_porta & 0x40 )
			m_ram_c000 = m_vram_region->base();
	}

	void update_irq_state()
	{
		if ( m_pia0_irq_state || m_pia1_irq_state )
			m_maincpu->set_input_line(0, ASSERT_LINE );
		else
			m_maincpu->set_input_line(0, CLEAR_LINE );
	}
	DECLARE_WRITE8_MEMBER(osbexec_0000_w);
	DECLARE_READ8_MEMBER(osbexec_c000_r);
	DECLARE_WRITE8_MEMBER(osbexec_c000_w);
	DECLARE_READ8_MEMBER(osbexec_kbd_r);
	DECLARE_READ8_MEMBER(osbexec_rtc_r);
	DECLARE_DRIVER_INIT(osbexec);
	virtual void machine_reset();
	TIMER_CALLBACK_MEMBER(osbexec_video_callback);
	DECLARE_READ8_MEMBER(osbexec_pia0_a_r);
	DECLARE_WRITE8_MEMBER(osbexec_pia0_a_w);
	DECLARE_READ8_MEMBER(osbexec_pia0_b_r);
	DECLARE_WRITE8_MEMBER(osbexec_pia0_b_w);
	DECLARE_WRITE_LINE_MEMBER(osbexec_pia0_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(osbexec_pia0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(osbexec_pia0_irq);
	DECLARE_WRITE_LINE_MEMBER(osbexec_pia1_irq);
};


WRITE8_MEMBER(osbexec_state::osbexec_0000_w)
{
	/* Font RAM writing is enabled when ROM bank is enabled */
	if ( m_pia0_porta & 0x80 )
	{
		if ( offset < 0x1000 )
			m_fontram[ offset ] = data;
	}
	else
	{
		m_ram_0000[ offset ] = data;
	}
}


READ8_MEMBER(osbexec_state::osbexec_c000_r)
{
	UINT8   data = m_ram_c000[offset];

	if ( ( m_pia0_porta & 0x40 ) && offset < 0x1000 )
	{
		m_temp_attr = m_ram_c000[ 0x1000 + offset ];
	}

	return data;
}


WRITE8_MEMBER(osbexec_state::osbexec_c000_w)
{
	m_ram_c000[offset] = data;

	if ( ( m_pia0_porta & 0x40 ) && offset < 0x1000 )
	{
		m_ram_c000[ 0x1000 + offset ] = m_temp_attr;
	}
}


READ8_MEMBER(osbexec_state::osbexec_kbd_r)
{
	UINT8 data = 0xFF;

	if ( offset & 0x0100 )
		data &= ioport( "ROW0" )->read();

	if ( offset & 0x0200 )
		data &= ioport( "ROW1" )->read();

	if ( offset & 0x0400 )
		data &= ioport( "ROW2" )->read();

	if ( offset & 0x0800 )
		data &= ioport( "ROW3" )->read();

	if ( offset & 0x1000 )
		data &= ioport( "ROW4" )->read();

	if ( offset & 0x2000 )
		data &= ioport( "ROW5" )->read();

	if ( offset & 0x4000 )
		data &= ioport( "ROW6" )->read();

	if ( offset & 0x8000 )
		data &= ioport( "ROW7" )->read();

	return data;
}


READ8_MEMBER(osbexec_state::osbexec_rtc_r)
{
	return m_rtc;
}


static ADDRESS_MAP_START( osbexec_mem, AS_PROGRAM, 8, osbexec_state )
	AM_RANGE( 0x0000, 0x1FFF ) AM_READ_BANK("0000") AM_WRITE(osbexec_0000_w )   /* ROM and maybe also banked ram */
	AM_RANGE( 0x2000, 0x3FFF ) AM_RAMBANK("2000")                               /* Banked RAM */
	AM_RANGE( 0x4000, 0xBFFF ) AM_RAMBANK("4000")                               /* Banked RAM */
	AM_RANGE( 0xC000, 0xDFFF ) AM_READWRITE(osbexec_c000_r, osbexec_c000_w )    /* Video ram / Banked RAM */
	AM_RANGE( 0xE000, 0xEFFF ) AM_RAMBANK("e000")                               /* Banked RAM */
	AM_RANGE( 0xF000, 0xFFFF ) AM_RAM                                           /* 4KB of non-banked RAM for system stack etc */
ADDRESS_MAP_END


static ADDRESS_MAP_START( osbexec_io, AS_IO, 8, osbexec_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00, 0x03 ) AM_MIRROR( 0xff00 ) AM_DEVREADWRITE( "pia_0", pia6821_device, read, write)               /* 6821 PIA @ UD12 */
	/* 0x04 - 0x07 - 8253 @UD1 */
	AM_RANGE( 0x08, 0x0B ) AM_MIRROR( 0xff00 ) AM_DEVREADWRITE("mb8877", wd_fdc_t, read, write )                /* MB8877 @ UB17 input clock = 1MHz */
	AM_RANGE( 0x0C, 0x0F ) AM_MIRROR( 0xff00 ) AM_DEVREADWRITE("sio", z80sio2_device, ba_cd_r, ba_cd_w ) /* SIO @ UD4 */
	AM_RANGE( 0x10, 0x13 ) AM_MIRROR( 0xff00 ) AM_DEVREADWRITE( "pia_1", pia6821_device, read, write)               /* 6821 PIA @ UD8 */
	AM_RANGE( 0x14, 0x17 ) AM_MIRROR( 0xff00 ) AM_MASK( 0xff00 ) AM_READ(osbexec_kbd_r )                    /* KBD */
	AM_RANGE( 0x18, 0x1b ) AM_MIRROR( 0xff00 ) AM_READ(osbexec_rtc_r )                                      /* "RTC" @ UE13/UF13 */
	/* ?? - vid ? */
ADDRESS_MAP_END


static INPUT_PORTS_START( osbexec )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)       PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)     PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x08, 0, "Alpha Lock" ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void osbexec_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
}

UINT32 osbexec_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*
  UD12 - 6821 PIA

  Port A:
  PA7 - ROM BANK ENA
  PA6 - VRAM BANK ENA
  PA5 - BANK6ENA
  PA4 - BANK5ENA
  PA3 - BANK4ENA
  PA2 - BANK3ENA
  PA1 - BANK2ENA
  PA0 - BANK1ENA
  CA1 - DMA IRQ
  CA2 - KBD STB (i/o)

  Port B:
  PB7 - MODEM RI (input)
  PB6 - MODEM DSR (input)
  PB5 - TXC SEL
  PB4 - RXC SEL
  PB3 - speaker
  PB2 - DSEL2
  PB1 - DSEL1
  PB0 - DDEN
  CB1 - VBlank (input)
  CB2 - 60/50 (?)
*/

READ8_MEMBER(osbexec_state::osbexec_pia0_a_r)
{
	return m_pia0_porta;
}


WRITE8_MEMBER(osbexec_state::osbexec_pia0_a_w)
{
	logerror("osbexec_pia0_a_w: %02x\n", data );

	m_pia0_porta = data;

	set_banks();
}


READ8_MEMBER(osbexec_state::osbexec_pia0_b_r)
{
	return m_pia0_portb;
}


WRITE8_MEMBER(osbexec_state::osbexec_pia0_b_w)
{
	m_pia0_portb = data;

	m_speaker->level_w(!BIT(data, 3));

	switch ( data & 0x06 )
	{
	case 0x02:
		m_mb8877->set_floppy(m_floppy1);
		m_floppy1->mon_w(0);
		break;
	case 0x04:
		m_mb8877->set_floppy(m_floppy0);
		m_floppy0->mon_w(0);
		break;
	default:
		m_mb8877->set_floppy(NULL);
		break;
	}

	m_mb8877->dden_w(( data & 0x01 ) ? 1 : 0 );
}


WRITE_LINE_MEMBER(osbexec_state::osbexec_pia0_ca2_w)
{
	logerror("osbexec_pia0_ca2_w: state = %d\n", state);
}


WRITE_LINE_MEMBER(osbexec_state::osbexec_pia0_cb2_w)
{
	m_pia0_cb2 = state;
}


WRITE_LINE_MEMBER(osbexec_state::osbexec_pia0_irq)
{
	m_pia0_irq_state = state;
	update_irq_state();
}


WRITE_LINE_MEMBER(osbexec_state::osbexec_pia1_irq)
{
	m_pia1_irq_state = state;
	update_irq_state();
}


/*
 * The Osborne Executive supports the following disc formats: (TODO: Verify)
 * - Osborne single density: 40 tracks, 10 sectors per track, 256-byte sectors (100 KByte)
 * - Osborne double density: 40 tracks, 5 sectors per track, 1024-byte sectors (200 KByte)
 * - IBM Personal Computer: 40 tracks, 8 sectors per track, 512-byte sectors (160 KByte)
 * - Xerox 820 Computer: 40 tracks, 18 sectors per track, 128-byte sectors (90 KByte)
 * - DEC 1820 double density: 40 tracks, 9 sectors per track, 512-byte sectors (180 KByte)
 *
 */

static SLOT_INTERFACE_START( osborne2_floppies )
	SLOT_INTERFACE( "525ssdd", FLOPPY_525_SSDD )
SLOT_INTERFACE_END


TIMER_CALLBACK_MEMBER(osbexec_state::osbexec_video_callback)
{
	int y = machine().first_screen()->vpos();

	/* Start of frame */
	if ( y == 0 )
	{
		/* Clear CB1 on PIA @ UD12 */
		m_pia_0->cb1_w(0);
	}
	else if ( y == 240 )
	{
		/* Set CB1 on PIA @ UD12 */
		m_pia_0->cb1_w(1);
		m_rtc++;
	}
	if ( y < 240 )
	{
		UINT16 row_addr = ( y / 10 ) * 128;
		UINT16 *p = &m_bitmap.pix16(y);
		UINT8 char_line = y % 10;

		for ( int x = 0; x < 80; x++ )
		{
			UINT8 ch = m_vram[ row_addr + x ];
			UINT8 attr = m_vram[ 0x1000 + row_addr + x ];
			UINT8 fg_col = ( attr & 0x80 ) ? 2 : 1;
			UINT8 font_bits = m_fontram[ ( ( attr & 0x10 ) ? 0x800 : 0 ) + ( ch & 0x7f ) * 16 + char_line ];

			/* Check for underline */
			if ( ( attr & 0x40 ) && char_line == 9 )
				font_bits = 0xFF;

			/* Check for blink */
			if ( ( attr & 0x20 ) && ( m_rtc & 0x10 ) )
				font_bits = 0;

			/* Check for inverse video */
			if ( ( ch & 0x80 ) && ! ( attr & 0x10 ) )
				font_bits ^= 0xFF;

			for ( int b = 0; b < 8; b++ )
			{
				p[ x * 8 + b ] = ( font_bits & 0x80 ) ? fg_col : 0;
				font_bits <<= 1;
			}
		}
	}

	m_video_timer->adjust( machine().first_screen()->time_until_pos( y + 1, 0 ) );
}


DRIVER_INIT_MEMBER(osbexec_state,osbexec)
{
	m_fontram_region = machine().memory().region_alloc( "fontram", 0x1000, 1, ENDIANNESS_LITTLE);
	m_vram_region = machine().memory().region_alloc( "vram", 0x2000, 1, ENDIANNESS_LITTLE );
	m_vram = m_vram_region->base();
	m_fontram = m_fontram_region->base();


	memset( m_fontram, 0x00, 0x1000 );
	memset( m_vram, 0x00, 0x2000 );

	m_video_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(osbexec_state::osbexec_video_callback),this));
}


void osbexec_state::machine_reset()
{
	m_pia0_porta = 0xC0;        /* Enable ROM and VRAM on reset */

	set_banks();

	m_video_timer->adjust( machine().first_screen()->time_until_pos( 0, 0 ) );

	m_rtc = 0;
}


static const z80_daisy_config osbexec_daisy_config[] =
{
	{ "sio" },
	{ NULL }
};


static MACHINE_CONFIG_START( osbexec, osbexec_state )
	MCFG_CPU_ADD( "maincpu", Z80, MAIN_CLOCK/6 )
	MCFG_CPU_PROGRAM_MAP( osbexec_mem)
	MCFG_CPU_IO_MAP( osbexec_io)
	MCFG_CPU_CONFIG( osbexec_daisy_config )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(osbexec_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS( MAIN_CLOCK/2, 768, 0, 640, 260, 0, 240 )    /* May not be correct */
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")

	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

//  MCFG_PIT8253_ADD("pit", osbexec_pit_config)

	MCFG_DEVICE_ADD("pia_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(osbexec_state, osbexec_pia0_a_r))
	MCFG_PIA_READPB_HANDLER(READ8(osbexec_state, osbexec_pia0_b_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(osbexec_state, osbexec_pia0_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(osbexec_state, osbexec_pia0_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(osbexec_state, osbexec_pia0_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(osbexec_state, osbexec_pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(osbexec_state, osbexec_pia0_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(osbexec_state, osbexec_pia0_irq))

	MCFG_DEVICE_ADD("pia_1", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(WRITELINE(osbexec_state, osbexec_pia1_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(osbexec_state, osbexec_pia1_irq))

	MCFG_Z80SIO2_ADD("sio", MAIN_CLOCK/6, 0, 0, 0, 0)

	MCFG_DEVICE_ADD("mb8877", MB8877, MAIN_CLOCK/24)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pia_1", pia6821_device, cb1_w))
	MCFG_FLOPPY_DRIVE_ADD("mb8877:0", osborne2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("mb8877:1", osborne2_floppies, "525ssdd", floppy_image_device::default_floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("136K")   /* 128KB Main RAM + RAM in ROM bank (8) */

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("flop_list", "osborne2")
MACHINE_CONFIG_END


ROM_START( osbexec )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "execv12.ud18", 0x0000, 0x2000, CRC(70798c2f) SHA1(2145a72da563bed1d6d455c77e48cc011a5f1153) )    /* Checksum C6B2 */
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME        FLAGS */
COMP( 1982, osbexec,    0,      0,      osbexec,    osbexec, osbexec_state,    osbexec,    "Osborne",  "Executive",    MACHINE_NOT_WORKING )
