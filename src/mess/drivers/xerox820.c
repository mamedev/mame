/***************************************************************************

        Xerox 820

        12/05/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - Xerox 820
        - floppy (wd1772.c FM support)
    - Xerox 820-II
        - floppy (read/write to FDC triggers Z80 WAIT)
        - Winchester
            - Shugart SA1004 (chs=256,4,40 ss=256)
            - Shugart SA606 (chs=160,6, ss=256)
            - Shugart SA1403D controller
    - Xerox 16/8
    - Emerald Microware X120 board
    - type in Monitor v1.0 from manual
    - proper keyboard emulation (MCU?)

    http://users.telenet.be/lust/Xerox820/index.htm
    http://www.classiccmp.org/dunfield/img41867/system.htm
    http://www.microcodeconsulting.com/z80/plus2.htm

    Note:
    - MK-82 have same roms as original Big Board
    - MK-83 have 256K of RAM
*/


#include "includes/xerox820.h"

/* Keyboard HACK */

static const UINT8 xerox820_keycodes[3][9][8] =
{
	/* unshifted */
	{
	{ 0x1e, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
	{ 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x71, 0x77, 0x65, 0x72 },
	{ 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x61, 0x73, 0x64 },
	{ 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27 },
	{ 0x0d, 0x0a, 0x01, 0x31, 0x32, 0x33, 0x7a, 0x78 },
	{ 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f },
	{ 0x04, 0x02, 0x03, 0x30, 0x2e, 0x20, 0x00, 0x00 }
	},

	/* shifted */
	{
	{ 0x1e, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26 },
	{ 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x51, 0x57, 0x45, 0x52 },
	{ 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x41, 0x53, 0x44 },
	{ 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22 },
	{ 0x0d, 0x0a, 0x01, 0x31, 0x32, 0x33, 0x5a, 0x58 },
	{ 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3c, 0x3e, 0x3f },
	{ 0x04, 0x02, 0x03, 0x30, 0x2e, 0x20, 0x00, 0x00 }
	},

	/* control */
	{
	{ 0x9e, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
	{ 0x98, 0x99, 0x90, 0x1f, 0x9a, 0x88, 0xff, 0xad },
	{ 0xb7, 0xb8, 0xb9, 0x89, 0x11, 0x17, 0x05, 0x12 },
	{ 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d },
	{ 0x9b, 0xab, 0xb4, 0xb5, 0xb6, 0x01, 0x13, 0x04 },
	{ 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x7e, 0x60 },
	{ 0x8d, 0x8a, 0x81, 0xb1, 0xb2, 0xb3, 0x1a, 0x18 },
	{ 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x1c, 0x7c, 0x5c },
	{ 0x84, 0x82, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 }
	}
};

void xerox820_state::scan_keyboard()
{
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };
	int table = 0, row, col;
	int keydata = -1;

	if (ioport("ROW9")->read() & 0x07)
	{
		/* shift, upper case */
		table = 1;
	}

	if (ioport("ROW9")->read() & 0x18)
	{
		/* ctrl */
		table = 2;
	}

	/* scan keyboard */
	for (row = 0; row < 9; row++)
	{
		UINT8 data = ioport(keynames[row])->read();

		for (col = 0; col < 8; col++)
		{
			if (!BIT(data, col))
			{
				/* latch key data */
				keydata = ~xerox820_keycodes[table][row][col];

				if (m_keydata != keydata)
				{
					m_keydata = keydata;

					/* strobe in keyboard data */
					m_kbpio->strobe_b(0);
					m_kbpio->strobe_b(1);
				}
			}
		}
	}

	m_keydata = keydata;
}

TIMER_DEVICE_CALLBACK_MEMBER(xerox820_state::xerox820_keyboard_tick)
{
	scan_keyboard();
}

/* Read/Write Handlers */

void xerox820_state::bankswitch(int bank)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (bank)
	{
		/* ROM */
		program.install_rom(0x0000, 0x0fff, memregion("monitor")->base());
		program.unmap_readwrite(0x1000, 0x1fff);
		program.install_ram(0x3000, 0x3fff, m_video_ram);
	}
	else
	{
		/* RAM */
		program.install_ram(0x0000, 0x3fff, ram);
	}
}

void xerox820ii_state::bankswitch(int bank)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	if (bank)
	{
		/* ROM */
		program.install_rom(0x0000, 0x17ff, memregion("monitor")->base());
		program.unmap_readwrite(0x1800, 0x2fff);
		program.install_ram(0x3000, 0x3fff, m_video_ram);
		program.unmap_readwrite(0x4000, 0xbfff);
	}
	else
	{
		/* RAM */
		program.install_ram(0x0000, 0xbfff, ram);
	}
}

WRITE8_MEMBER( xerox820_state::scroll_w )
{
	m_scroll = (offset >> 8) & 0x1f;
}

#ifdef UNUSED_CODE
WRITE8_MEMBER( xerox820_state::x120_system_w )
{
	/*

        bit     signal      description

        0       DSEL0       drive select bit 0 (01=A, 10=B, 00=C, 11=D)
        1       DSEL1       drive select bit 1
        2       SIDE        side select
        3       VATT        video attribute (0=inverse, 1=blinking)
        4       BELL        bell trigger
        5       DENSITY     density (0=double, 1=single)
        6       _MOTOR      disk motor (0=on, 1=off)
        7       BANK        memory bank switch (0=RAM, 1=ROM/video)

    */
}
#endif

WRITE8_MEMBER( xerox820ii_state::bell_w )
{
	speaker_level_w(m_speaker, offset);
}

WRITE8_MEMBER( xerox820ii_state::slden_w )
{
	m_fdc->dden_w(offset);
}

WRITE8_MEMBER( xerox820ii_state::chrom_w )
{
	m_chrom = offset;
}

WRITE8_MEMBER( xerox820ii_state::lowlite_w )
{
	m_lowlite = data;
}

WRITE8_MEMBER( xerox820ii_state::sync_w )
{
	if (offset)
	{
		/* set external clocks for synchronous sio A */
	}
	else
	{
		/* set internal clocks for asynchronous sio A */
	}
}

/* Memory Maps */

static ADDRESS_MAP_START( xerox820_mem, AS_PROGRAM, 8, xerox820_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820_io, AS_IO, 8, xerox820_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff03) AM_DEVWRITE(COM8116_TAG, com8116_device, str_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0xff02) AM_DEVREADWRITE_LEGACY(Z80SIO_TAG, z80dart_d_r, z80dart_d_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0xff02) AM_DEVREADWRITE_LEGACY(Z80SIO_TAG, z80dart_c_r, z80dart_c_w)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80PIO_GP_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff03) AM_DEVWRITE(COM8116_TAG, com8116_device, stt_w)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_DEVREADWRITE(FD1771_TAG, wd_fdc_t, read, write)
	AM_RANGE(0x14, 0x14) AM_MIRROR(0xff03) AM_MASK(0xff00) AM_WRITE(scroll_w)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80PIO_KB_TAG, z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820ii_mem, AS_PROGRAM, 8, xerox820ii_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820ii_io, AS_IO, 8, xerox820ii_state )
	AM_IMPORT_FROM(xerox820_io)
	AM_RANGE(0x28, 0x29) AM_MIRROR(0xff00) AM_WRITE(bell_w)
	AM_RANGE(0x30, 0x31) AM_MIRROR(0xff00) AM_WRITE(slden_w)
	AM_RANGE(0x34, 0x35) AM_MIRROR(0xff00) AM_WRITE(chrom_w)
	AM_RANGE(0x36, 0x36) AM_MIRROR(0xff00) AM_WRITE(lowlite_w)
	AM_RANGE(0x68, 0x69) AM_MIRROR(0xff00) AM_WRITE(sync_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox168_mem, AS_PROGRAM, 16, xerox820ii_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
	AM_RANGE(0xff000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mk83_mem, AS_PROGRAM, 8, xerox820_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


/* Input Ports */

static INPUT_PORTS_START( xerox820 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT CTRL") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER( bigboard_state::bigboard_beepoff )
{
	beep_set_state(m_beeper, 0);
}

/* Z80 PIO */

READ8_MEMBER( xerox820_state::kbpio_pa_r )
{
	/*

        bit     signal          description

        0
        1
        2
        3       PBRDY           keyboard data available
        4       8/N5            8"/5.25" disk select (0=5.25", 1=8")
        5       400/460         double sided disk detect (only on Etch 2 PCB) (0=SS, 1=DS)
        6
        7

    */

    UINT8 data = 0;

    // keyboard
	data |= m_kbpio->rdy_b() << 3;

	// floppy
	data |= m_8n5 << 4;
	data |= m_400_460 << 5;

	return data;
};

WRITE8_MEMBER( xerox820_state::kbpio_pa_w )
{
	/*

        bit     signal          description

        0       _DVSEL1         drive select 1
        1       _DVSEL2         drive select 2
        2       SIDE            side select
        3
        4
        5
        6       NCSET2          display character set (inverted and connected to chargen A10)
        7       BANK            bank switching (0=RAM, 1=ROM/videoram)

    */

	/* drive select */
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		int _8n5 = (floppy->get_form_factor() == floppy_image::FF_8);

		if (m_8n5 != _8n5)
		{
			m_8n5 = _8n5;

			m_fdc->set_unscaled_clock(m_8n5 ? XTAL_20MHz/10 : XTAL_20MHz/20);
		}

		m_400_460 = !floppy->twosid_r();

		floppy->mon_w(0);

		floppy->ss_w(BIT(data, 2));
	}

	/* display character set */
	m_ncset2 = !BIT(data, 6);

	/* bank switching */
	bankswitch(BIT(data, 7));
}

WRITE8_MEMBER( bigboard_state::kbpio_pa_w )
{
	xerox820_state::kbpio_pa_w(space, offset, data);

	/* beeper on bigboard */
	if (BIT(data, 5) & (!m_bit5))
	{
		machine().scheduler().timer_set(attotime::from_msec(40), timer_expired_delegate(FUNC(bigboard_state::bigboard_beepoff),this));
		beep_set_state(m_beeper, 1);
	}
	m_bit5 = BIT(data, 5);
}

READ8_MEMBER( xerox820_state::kbpio_pb_r )
{
	/*

        bit     description

        0       KB0
        1       KB1
        2       KB2
        3       KB3
        4       KB4
        5       KB5
        6       KB6
        7       KB7

    */

	return m_keydata;
};

static Z80PIO_INTERFACE( xerox820_kbpio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),		/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pa_r),	/* port A read callback */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pa_w),	/* port A write callback */
	DEVCB_NULL,											/* portA ready active callback */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pb_r),	/* port B read callback */
	DEVCB_NULL,											/* port B write callback */
	DEVCB_NULL											/* portB ready active callback */
};

static Z80PIO_INTERFACE( xerox820ii_kbpio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),		/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pa_r),	/* port A read callback */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pa_w),	/* port A write callback */
	DEVCB_NULL,											/* portA ready active callback */
	DEVCB_DRIVER_MEMBER(xerox820_state, kbpio_pb_r),	/* port B read callback */
	DEVCB_NULL,											/* port B write callback */
	DEVCB_NULL											/* portB ready active callback */
};

static Z80PIO_INTERFACE( gppio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_NULL,		/* port A read callback */
	DEVCB_NULL,		/* port A write callback */
	DEVCB_NULL,		/* portA ready active callback */
	DEVCB_NULL,		/* port B read callback */
	DEVCB_NULL,		/* port B write callback */
	DEVCB_NULL		/* portB ready active callback */
};

READ8_MEMBER( xerox820ii_state::rdpio_pb_r )
{
	/*

        bit     description

        0       NBSY
        1       NMSG
        2       NC/D
        3       NREQ
        4       NI/O
        5
        6       LS74 Q
        7

    */

	UINT8 data = 0;

	data |= !m_sasibus->scsi_bsy_r();
	data |= !m_sasibus->scsi_msg_r() << 1;
	data |= !m_sasibus->scsi_cd_r() << 2;
	data |= !m_sasibus->scsi_req_r() << 3;
	data |= !m_sasibus->scsi_io_r() << 4;

	return data;
}

WRITE8_MEMBER( xerox820ii_state::rdpio_pb_w )
{
	/*

        bit     description

        0
        1
        2
        3
        4
        5       NSEL
        6
        7       NRST

    */

	m_sasibus->scsi_sel_w(!BIT(data, 5));
	m_sasibus->scsi_rst_w(!BIT(data, 7));
}

WRITE_LINE_MEMBER( xerox820ii_state::rdpio_pardy_w )
{
	// TODO
}

static Z80PIO_INTERFACE( rdpio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_DEVICE_MEMBER(SASIBUS_TAG ":host", scsicb_device, scsi_data_r),	/* port A read callback */
	DEVCB_DEVICE_MEMBER(SASIBUS_TAG ":host", scsicb_device, scsi_data_w),	/* port A write callback */
	DEVCB_DRIVER_LINE_MEMBER(xerox820ii_state, rdpio_pardy_w),		/* portA ready active callback */
	DEVCB_DRIVER_MEMBER(xerox820ii_state, rdpio_pb_r),		/* port B read callback */
	DEVCB_DRIVER_MEMBER(xerox820ii_state, rdpio_pb_w),		/* port B write callback */
	DEVCB_NULL		/* portB ready active callback */
};

/* Z80 SIO */

static Z80DART_INTERFACE( sio_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0)
};

/* Z80 CTC */

TIMER_DEVICE_CALLBACK_MEMBER( xerox820_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* interrupt handler */
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg1),	/* ZC/TO0 callback */
	DEVCB_NULL,	/* ZC/TO1 callback */
	DEVCB_DEVICE_LINE_MEMBER(Z80CTC_TAG, z80ctc_device, trg3)	/* ZC/TO2 callback */
};

/* Z80 Daisy Chain */

static const z80_daisy_config xerox820_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80CTC_TAG },
	{ NULL }
};

/* WD1771 Interface */

static SLOT_INTERFACE_START( xerox820_floppies )
	SLOT_INTERFACE( "sa400", FLOPPY_525_SSSD_35T ) // Shugart SA-400
	SLOT_INTERFACE( "sa450", FLOPPY_525_DD ) // Shugart SA-450
	SLOT_INTERFACE( "sa800", FLOPPY_8_SSDD ) // Shugart SA-800
	SLOT_INTERFACE( "sa850", FLOPPY_8_DSDD ) // Shugart SA-850
SLOT_INTERFACE_END

void xerox820_state::fdc_intrq_w(bool state)
{
	m_fdc_irq = state;

	int halt = m_maincpu->state_int(Z80_HALT);

	if (halt && (m_fdc_irq || m_fdc_drq))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void xerox820_state::fdc_drq_w(bool state)
{
	m_fdc_drq = state;

	int halt = m_maincpu->state_int(Z80_HALT);

	if (halt && (m_fdc_irq || m_fdc_drq))
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

/* COM8116 Interface */

static COM8116_INTERFACE( com8116_intf )
{
	DEVCB_NULL,		/* fX/4 output */
	DEVCB_NULL,		/* fR output */
	DEVCB_NULL,		/* fT output */
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* receiver divisor ROM */
	{ 101376, 67584, 46080, 37686, 33792, 16896, 8448, 4224, 2816, 2534, 2112, 1408, 1056, 704, 528, 264 },			/* transmitter divisor ROM */
};

/* Video */

void xerox820_state::video_start()
{
	/* find memory regions */
	m_char_rom = memregion("chargen")->base();
}


UINT32 xerox820_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=(m_scroll + 1) * 0x80,x;

	m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		if (ma > 0xb80) ma = 0;

		for (ra = 0; ra < 10; ra++)
		{
			UINT32 *p = &bitmap.pix32(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				if (ra < 8)
				{
					chr = m_video_ram[x & XEROX820_VIDEORAM_MASK] ^ 0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_char_rom[(m_ncset2 << 10) | (chr<<3) | ra ];
				}
				else
					gfx = 0xff;

			/* Display a scanline of a character (7 pixels) */
			*p++ = RGB_MONOCHROME_WHITE[0];
			*p++ = RGB_MONOCHROME_WHITE[BIT(gfx, 4) ^ 1];
			*p++ = RGB_MONOCHROME_WHITE[BIT(gfx, 3) ^ 1];
			*p++ = RGB_MONOCHROME_WHITE[BIT(gfx, 2) ^ 1];
			*p++ = RGB_MONOCHROME_WHITE[BIT(gfx, 1) ^ 1];
			*p++ = RGB_MONOCHROME_WHITE[BIT(gfx, 0) ^ 1];
			*p++ = RGB_MONOCHROME_WHITE[0];
			}
		}
		ma+=128;
	}
	return 0;
}

/* Machine Initialization */

void xerox820_state::machine_start()
{
	// floppy callbacks
	m_fdc->setup_intrq_cb(wd_fdc_t::line_cb(FUNC(xerox820_state::fdc_intrq_w), this));
	m_fdc->setup_drq_cb(wd_fdc_t::line_cb(FUNC(xerox820_state::fdc_drq_w), this));
	m_fdc->dden_w(1);

	// state saving
	save_item(NAME(m_keydata));
	save_item(NAME(m_scroll));
	save_item(NAME(m_ncset2));
	save_item(NAME(m_vatt));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_8n5));
	save_item(NAME(m_400_460));
}

void xerox820_state::machine_reset()
{
	bankswitch(1);

	m_fdc->reset();
}

void bigboard_state::machine_reset()
{
	bankswitch(1);

	/* bigboard has a one-pulse output to drive a user-supplied beeper */
	beep_set_state(m_beeper, 0);
	beep_set_frequency(m_beeper, 950);

	m_fdc->reset();
}

void xerox820ii_state::machine_reset()
{
	bankswitch(1);

	m_fdc->reset();
}
/*
static LEGACY_FLOPPY_OPTIONS_START( xerox820 )
    LEGACY_FLOPPY_OPTION( sssd8, "dsk", "8\" SSSD", basicdsk_identify_default, basicdsk_construct_default, NULL,
        HEADS([1])
        TRACKS([77])
        SECTORS([26])
        SECTOR_LENGTH([128])
        FIRST_SECTOR_ID([1]))
    LEGACY_FLOPPY_OPTION( ssdd8, "dsk", "8\" SSDD", basicdsk_identify_default, basicdsk_construct_default, NULL,
        HEADS([1])
        TRACKS([77])
        SECTORS([26])
        SECTOR_LENGTH([256])
        FIRST_SECTOR_ID([1]))
    LEGACY_FLOPPY_OPTION( sssd5, "dsk", "5.25\" SSSD", basicdsk_identify_default, basicdsk_construct_default, NULL,
        HEADS([1])
        TRACKS([40])
        SECTORS([18])
        SECTOR_LENGTH([128])
        FIRST_SECTOR_ID([1]))
    LEGACY_FLOPPY_OPTION( ssdd5, "dsk", "5.25\" SSDD", basicdsk_identify_default, basicdsk_construct_default, NULL,
        HEADS([2])
        TRACKS([40])
        SECTORS([18])
        SECTOR_LENGTH([128])
        FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END
*/

/* F4 Character Displayer */
static const gfx_layout xerox820_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8					/* every char takes 8 bytes */
};

static const gfx_layout xerox820_gfxlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8					/* every char takes 8 bytes */
};

static GFXDECODE_START( xerox820 )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( xerox820ii )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, xerox820_gfxlayout, 0, 1 )
GFXDECODE_END

/* Machine Drivers */

static MACHINE_CONFIG_START( xerox820, xerox820_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_20MHz/8)
	MCFG_CPU_PROGRAM_MAP(xerox820_mem)
	MCFG_CPU_IO_MAP(xerox820_io)
	MCFG_CPU_CONFIG(xerox820_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(xerox820_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_69425MHz, 700, 0, 560, 260, 0, 240)
	MCFG_GFXDECODE(xerox820)

	/* keyboard */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", xerox820_state, xerox820_keyboard_tick, attotime::from_hz(60))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", xerox820_state, ctc_tick, attotime::from_hz(XTAL_20MHz/8))

	/* devices */
	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_20MHz/8, sio_intf)
	MCFG_Z80PIO_ADD(Z80PIO_KB_TAG, XTAL_20MHz/8, xerox820_kbpio_intf)
	MCFG_Z80PIO_ADD(Z80PIO_GP_TAG, XTAL_20MHz/8, gppio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_20MHz/8, ctc_intf)
	MCFG_FD1771x_ADD(FD1771_TAG, XTAL_20MHz/20)
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":0", xerox820_floppies, "sa400", NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":1", xerox820_floppies, "sa400", NULL, floppy_image_device::default_floppy_formats)
	MCFG_COM8116_ADD(COM8116_TAG, XTAL_5_0688MHz, com8116_intf)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "xerox820")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( bigboard, xerox820, bigboard_state )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(BEEPER_TAG, BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00) /* bigboard only */
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( xerox820ii, xerox820ii_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(xerox820ii_mem)
	MCFG_CPU_IO_MAP(xerox820ii_io)
	MCFG_CPU_CONFIG(xerox820_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(xerox820ii_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_69425MHz, 700, 0, 560, 260, 0, 240)
	MCFG_GFXDECODE(xerox820ii)

	/* keyboard */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", xerox820_state, xerox820_keyboard_tick, attotime::from_hz(60))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", xerox820_state, ctc_tick, attotime::from_hz(XTAL_16MHz/4))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_16MHz/4, sio_intf)
	MCFG_Z80PIO_ADD(Z80PIO_KB_TAG, XTAL_16MHz/4, xerox820ii_kbpio_intf)
	MCFG_Z80PIO_ADD(Z80PIO_GP_TAG, XTAL_16MHz/4, gppio_intf)
	MCFG_Z80PIO_ADD(Z80PIO_RD_TAG, XTAL_20MHz/8, rdpio_intf)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_16MHz/4, ctc_intf)
	MCFG_FD1797x_ADD(FD1797_TAG, XTAL_16MHz/8)
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":0", xerox820_floppies, "sa450", NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":1", xerox820_floppies, "sa450", NULL, floppy_image_device::default_floppy_formats)
	MCFG_COM8116_ADD(COM8116_TAG, XTAL_5_0688MHz, com8116_intf)

	// SASI bus
	MCFG_SCSIBUS_ADD(SASIBUS_TAG)
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":harddisk0", SA1403D, SCSI_ID_0)
	MCFG_SCSICB_ADD(SASIBUS_TAG ":host")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "xerox820ii")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xerox168, xerox820ii )
	MCFG_CPU_ADD(I8086_TAG, I8086, 4770000)
	MCFG_CPU_PROGRAM_MAP(xerox168_mem)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("192K")
	MCFG_RAM_EXTRA_OPTIONS("320K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mk83, xerox820 )
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_PROGRAM_MAP(mk83_mem)
MACHINE_CONFIG_END

/* ROMs */

ROM_START( bigboard )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "bigboard.u67", 0x0000, 0x0800, CRC(5a85a228) SHA1(d51a2cbd0aae80315bda9530275aabfe8305364e))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "bigboard.u73", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

#define rom_mk82 rom_bigboard

ROM_START( x820 )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_DEFAULT_BIOS( "v20" )
	ROM_SYSTEM_BIOS( 0, "v10", "Xerox Monitor v1.0" )
	ROMX_LOAD( "x820v10.u64", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "x820v10.u63", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v20", "Xerox Monitor v2.0" )
	ROMX_LOAD( "x820v20.u64", 0x0000, 0x0800, CRC(2fc227e2) SHA1(b4ea0ae23d281a687956e8a514cb364a1372678e), ROM_BIOS(2) )
	ROMX_LOAD( "x820v20.u63", 0x0800, 0x0800, CRC(bc11f834) SHA1(4fd2b209a6e6ff9b0c41800eb5228c34a0d7f7ef), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "smart23", "MICROCode SmartROM v2.3" )
	ROMX_LOAD( "mxkx25a.u64", 0x0000, 0x0800, CRC(7ec5f100) SHA1(5d0ff35a51aa18afc0d9c20ef99ff5d9d3f2075b), ROM_BIOS(3) )
	ROMX_LOAD( "mxkx25b.u63", 0x0800, 0x0800, CRC(a7543798) SHA1(886e617e1003d13f86f33085cbd49391b77291a3), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "plus2", "MICROCode Plus2 v0.2a" )
	ROMX_LOAD( "p2x25a.u64",  0x0000, 0x0800, CRC(3ccd7a8f) SHA1(6e46c88f03fc7289595dd6bec95e23bb13969525), ROM_BIOS(4) )
	ROMX_LOAD( "p2x25b.u63",  0x0800, 0x0800, CRC(1e580391) SHA1(e91f8ce82586df33c0d6d02eb005e8079f4de67d), ROM_BIOS(4) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "x820.u92", 0x0000, 0x0800, CRC(b823fa98) SHA1(ad0ea346aa257a53ad5701f4201896a2b3a0f928) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "keyboard", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( x820ii )
	ROM_REGION( 0x1800, "monitor", 0 )
	ROM_DEFAULT_BIOS( "v404" )
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" )
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(1) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(1) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9) )
	ROM_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "keyboard", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( x168 )
	ROM_REGION( 0x1800, "monitor", 0 )
	ROM_DEFAULT_BIOS( "v404" )
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" )
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(1) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(1) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(1) )

	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9) )
	ROM_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "keyboard", 0x0000, 0x0800, NO_DUMP )
ROM_END

ROM_START( mk83 )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "2732mk83.bin", 0x0000, 0x1000, CRC(a845c7e1) SHA1(3ccf629c5cd384953794ac4a1d2b45678bd40e92))
	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "2716mk83.bin", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c))
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                         FULLNAME        FLAGS */
COMP( 1980, bigboard,   0,          0,      bigboard,   xerox820, driver_device,   0,      "Digital Research Computers",   "Big Board", 	GAME_IMPERFECT_KEYBOARD )
COMP( 1981, x820,   	bigboard,   0,      xerox820,   xerox820, driver_device,   0,      "Xerox",                        "Xerox 820", 	GAME_IMPERFECT_KEYBOARD | GAME_NO_SOUND_HW )
COMP( 1982, mk82,   	bigboard,   0,      bigboard,   xerox820, driver_device,   0,      "Scomar",                       "MK-82", 		GAME_IMPERFECT_KEYBOARD )
COMP( 1983, x820ii, 	0,  		0,      xerox820ii, xerox820, driver_device,   0,      "Xerox",                        "Xerox 820-II",	GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
COMP( 1983, x168,   	x820ii, 	0,      xerox168,   xerox820, driver_device,   0,      "Xerox",                        "Xerox 16/8",	GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
COMP( 1983, mk83,       x820ii, 	0,      mk83,       xerox820, driver_device,   0,      "Scomar",                       "MK-83",     	GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD | GAME_NO_SOUND_HW )
