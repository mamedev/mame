// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Dirk Best
/******************************************************************************
 *  Sharp MZ700
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 *  MZ700 memory map
 *
 *  0000-0FFF   1Z-013A ROM or RAM
 *  1000-CFFF   RAM
 *  D000-D7FF   videoram or RAM
 *  D800-DFFF   colorram or RAM
 *  E000-FFFF   memory mapped IO or RAM
 *
 *      xxx0    PPI8255 port A (output)
 *              bit 7   556RST (reset NE556)
 *              bit 6-4 unused
 *              bit 3-0 keyboard row demux (LS145)
 *
 *      xxx1    PPI8255 port B (input)
 *              bit 7-0 keyboard matrix code
 *
 *      xxx2    PPI8255 port C (input/output)
 *              bit 7 R -VBLANK input
 *              bit 6 R 556OUT (1.5Hz)
 *              bit 5 R RDATA from cassette
 *              bit 4 R MOTOR from cassette
 *              bit 3 W M-ON control
 *              bit 2 W INTMASK 1=enable 0=disabel clock interrupt
 *              bit 1 W WDATA to cassette
 *              bit 0 W unused
 *
 *      xxx3    PPI8255 control
 *
 *      xxx4    PIT8253 timer 0 (clock input 1,108800 MHz)
 *      xxx5    PIT8253 timer 1 (clock input 15,611 kHz)
 *      xxx6    PIT8253 timer 2 (clock input OUT1 1Hz (default))
 *      xxx7    PIT8253 control/status
 *
 *      xxx8    bit 7 R -HBLANK
 *              bit 6 R unused
 *              bit 5 R unused
 *              bit 4 R joystick JB2
 *              bit 3 R joystick JB1
 *              bit 2 R joystick JA2
 *              bit 1 R joystick JA1
 *              bit 0 R NE556 OUT (32Hz IC BJ)
 *                    W gate0 of PIT8253 (sound enable)
 *
 *  MZ800 memory map
 *
 *  0000-0FFF   ROM or RAM
 *  1000-1FFF   PCG ROM or RAM
 *  2000-7FFF   RAM
 *  8000-9FFF   videoram or RAM
 *  A000-BFFF   videoram or RAM
 *  C000-CFFF   PCG RAM or RAM
 *  D000-DFFF   videoram or RAM
 *  E000-FFFF   memory mapped IO or RAM
 *
 *  ToDo:
    - slows down while making sound
    - MZ800:
      - Port CF not done.
      - Dips not connected.
      - MZ800-mode display not working /Hi-res not coded.
      - The CRTC is a very complex custom device, mostly unemulated.
    - MZ1500:
      - Various ports not done.
      - Floppy disk and quick disk not done.
      - F4 display is blank.
      - Need manuals.

  Note: MZ800 hardware starts in memory map (mode A), but switches to MZ700
  compatibility mode (mode B) as soon as it starts up. We start in Mode B
  because it helps MZ1500 get started and it doesn't break anything.

*
 *****************************************************************************/

#include "emu.h"
#include "includes/mz700.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/z80pio.h"
#include "machine/74145.h"
#include "bus/centronics/ctronics.h"
#include "sound/sn76496.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "formats/mz_cas.h"
#include "softlist.h"

/***************************************************************************
    TIMER DEVICE CALLBACKS
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mz_state::ne556_cursor_callback)
{
	m_cursor_timer ^= 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(mz_state::ne556_other_callback)
{
	m_other_timer ^= 1;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( mz700_mem, AS_PROGRAM, 8, mz_state )
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAMBANK("bankd")
	AM_RANGE(0xe000, 0xffff) AM_DEVICE("banke", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz700_banke, AS_PROGRAM, 8, mz_state )
	// bank 0: ram (mz700_bank1)
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	// bank 1: devices (mz700_bank3)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x1ff0) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x2004, 0x2007) AM_MIRROR(0x1ff0) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x2008, 0x200b) AM_MIRROR(0x1ff0) AM_READWRITE(mz700_e008_r,mz700_e008_w)
	AM_RANGE(0x200c, 0x200f) AM_MIRROR(0x1ff0) AM_NOP
	// bank 2: switched out (mz700_bank5)
	AM_RANGE(0x4000, 0x5fff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz700_io, AS_IO, 8, mz_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(mz700_bank_0_w)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(mz700_bank_1_w)
	AM_RANGE(0xe2, 0xe2) AM_WRITE(mz700_bank_2_w)
	AM_RANGE(0xe3, 0xe3) AM_WRITE(mz700_bank_3_w)
	AM_RANGE(0xe4, 0xe4) AM_WRITE(mz700_bank_4_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(mz700_bank_5_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITE(mz700_bank_6_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz800_mem, AS_PROGRAM, 8, mz_state )
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("banka")
	AM_RANGE(0xc000, 0xcfff) AM_RAMBANK("bankc")
	AM_RANGE(0xd000, 0xdfff) AM_RAMBANK("bankd")
	AM_RANGE(0xe000, 0xffff) AM_DEVICE("bankf", address_map_bank_device, amap8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz800_bankf, AS_PROGRAM, 8, mz_state )
	// bank 0: ram (mz700_bank1)
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	// bank 1: devices (mz700_bank3)
	AM_RANGE(0x2000, 0x2003) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x2004, 0x2007) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x2008, 0x200b) AM_READWRITE(mz700_e008_r,mz700_e008_w)
	AM_RANGE(0x200c, 0x200f) AM_NOP
	AM_RANGE(0x2010, 0x3fff) AM_ROM AM_REGION("monitor", 0x2010)
	// bank 2: switched out (mz700_bank5)
	AM_RANGE(0x4000, 0x5fff) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( mz800_io, AS_IO, 8, mz_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xcc, 0xcc) AM_WRITE(mz800_write_format_w )
	AM_RANGE(0xcd, 0xcd) AM_WRITE(mz800_read_format_w )
	AM_RANGE(0xce, 0xce) AM_READWRITE(mz800_crtc_r, mz800_display_mode_w )
	AM_RANGE(0xcf, 0xcf) AM_WRITE(mz800_scroll_border_w )
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0xd4, 0xd7) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0xe0, 0xe0) AM_READWRITE(mz800_bank_0_r, mz800_bank_0_w)
	AM_RANGE(0xe1, 0xe1) AM_READWRITE(mz800_bank_1_r, mz700_bank_1_w)
	AM_RANGE(0xe2, 0xe2) AM_WRITE(mz700_bank_2_w)
	AM_RANGE(0xe3, 0xe3) AM_WRITE(mz700_bank_3_w)
	AM_RANGE(0xe4, 0xe4) AM_WRITE(mz700_bank_4_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(mz700_bank_5_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITE(mz700_bank_6_w)
	AM_RANGE(0xea, 0xea) AM_READWRITE(mz800_ramdisk_r, mz800_ramdisk_w )
	AM_RANGE(0xeb, 0xeb) AM_WRITE(mz800_ramaddr_w )
	AM_RANGE(0xf0, 0xf0) AM_READ_PORT("atari_joy1") AM_WRITE(mz800_palette_w)
	AM_RANGE(0xf1, 0xf1) AM_READ_PORT("atari_joy2")
	AM_RANGE(0xf2, 0xf2) AM_DEVWRITE("sn76489n", sn76489_device, write)
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/* 2008-05 FP:
Notice that there is no Backspace key, only a 'Del' one.

Small note about natural keyboard support: currently,
- "Alpha" is mapped to 'F6'
- "Graph" is mapped to 'F7'
- "Break" is mapped to 'F8'                      */

static INPUT_PORTS_START( mz700 )
	PORT_START("ROW0")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, 0x08, IPT_UNUSED )
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_NAME("Alpha") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x93  \xC2\xA3") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\xA3') // this one would be 2nd row, 3rd key after 'P'
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)          PORT_CHAR('_') // this one would be 2nd row, 4th key after 'P'

	PORT_START("ROW1")
	PORT_BIT(0x07, 0x07, IPT_UNUSED )
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW2")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW3")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("ROW4")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("ROW5")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW6")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("0  Pi")               PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x91  ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('~')
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)          PORT_CHAR('\\') PORT_CHAR('|')  // this one would be 1st row, 3rd key after '0'

	PORT_START("ROW7")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("/  \xE2\x86\x90") PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("?  \xE2\x86\x92") PORT_CODE(KEYCODE_END)      PORT_CHAR('?')  // this one would be 4th row, 4th key after 'M'
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)                                  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                                 PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                                  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                                    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)                  PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_NAME("INST") PORT_CODE(KEYCODE_INSERT)              PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("ROW8")
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x3e, 0x3e, IPT_UNUSED)
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_HOME)               PORT_CHAR(UCHAR_MAMEKEY(F8))    // this one would be at Backspace position

	PORT_START("ROW9")
	PORT_BIT(0x07, 0x07, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)            PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x10, 0x10, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)            PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x20, 0x20, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)            PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x40, 0x40, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, 0x80, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("JOY")
	PORT_BIT(0x01, 0x00, IPT_UNUSED)
	PORT_BIT(0x02, 0x00, IPT_JOYSTICK_UP)       PORT_8WAY
	PORT_BIT(0x04, 0x00, IPT_JOYSTICK_DOWN)     PORT_8WAY
	PORT_BIT(0x08, 0x00, IPT_JOYSTICK_LEFT)     PORT_8WAY
	PORT_BIT(0x10, 0x00, IPT_JOYSTICK_RIGHT)    PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( mz800 )
	PORT_INCLUDE(mz700)

	PORT_MODIFY("JOY")
	PORT_BIT(0x1f, 0x00, IPT_UNUSED)

	PORT_START("atari_joy1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(1)

	PORT_START("atari_joy2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_PLAYER(2)

	PORT_START("system_settings")
	PORT_DIPNAME(0x01, 0x00, "Mode selection")
	PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(0x01, "MZ-700")
	PORT_DIPSETTING(0x00, "MZ-800")
	PORT_DIPNAME(0x06, 0x06, "Printer selection")
	PORT_DIPLOCATION("SW:3,2")
	PORT_DIPSETTING(0x06, "MZ printer")
	PORT_DIPSETTING(0x00, "Centronics printer")
	PORT_DIPNAME(0x08, 0x08, "Cassette polarity")
	PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(0x08, DEF_STR(Unknown))
	PORT_DIPSETTING(0x00, DEF_STR(Unknown))
INPUT_PORTS_END


/***************************************************************************
    GFX LAYOUT
***************************************************************************/

static const gfx_layout mz700_layout =
{
	8, 8,       /* 8 x 8 graphics */
	512,        /* 512 codes */
	1,      /* 1 bit per pixel */
	{ 0 },      /* no bitplanes */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8       /* code takes 8 times 8 bits */
};

static GFXDECODE_START( mz700 )
	GFXDECODE_ENTRY("cgrom", 0, mz700_layout, 0, 4)
GFXDECODE_END

static GFXDECODE_START( mz800 )
	GFXDECODE_ENTRY("monitor", 0x1000, mz700_layout, 0, 4)    // for mz800 viewer only
GFXDECODE_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static MACHINE_CONFIG_START( mz700, mz_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_17_73447MHz/5)
	MCFG_CPU_PROGRAM_MAP(mz700_mem)
	MCFG_CPU_IO_MAP(mz700_io)
	MCFG_DEVICE_ADD("banke", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(mz700_banke)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_MACHINE_RESET_OVERRIDE(mz_state, mz700)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_17_73447MHz/2, 568, 0, 40*8, 312, 0, 25*8)
	MCFG_SCREEN_UPDATE_DRIVER(mz_state, screen_update_mz700)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mz700)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* ne556 timers */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("cursor", mz_state, ne556_cursor_callback, attotime::from_hz(1.5))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("other", mz_state, ne556_other_callback, attotime::from_hz(34.5))

	/* devices */
	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_17_73447MHz/20)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(mz_state, pit_out0_changed))
	MCFG_PIT8253_CLK1(15611.0)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pit8253", pit8253_device, write_clk2))
	MCFG_PIT8253_CLK2(0)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(mz_state, pit_irq_2))

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(mz_state, pio_port_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(mz_state, pio_port_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(mz_state, pio_port_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(mz_state, pio_port_c_w))

	MCFG_DEVICE_ADD("ls145", TTL74145, 0)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(mz700_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("mz_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","mz700_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mz800, mz700 )
	MCFG_DEVICE_REMOVE("banke")

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mz800_mem)
	MCFG_CPU_IO_MAP(mz800_io)
	MCFG_DEVICE_ADD("bankf", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(mz800_bankf)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x2000)

	MCFG_MACHINE_RESET_OVERRIDE(mz_state, mz800)
	MCFG_GFXDECODE_MODIFY("gfxdecode",mz800)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(mz_state, screen_update_mz800)

	MCFG_SOUND_ADD("sn76489n", SN76489, XTAL_17_73447MHz/5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_REMOVE("cass_list")
	MCFG_SOFTWARE_LIST_ADD("cass_list","mz800_cass")

	/* devices */
	MCFG_DEVICE_MODIFY("pit8253")
	MCFG_PIT8253_CLK0(XTAL_17_73447MHz/16)

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_17_73447MHz/5)
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(mz_state, mz800_z80pio_irq))
	MCFG_Z80PIO_IN_PA_CB(READ8(mz_state, mz800_z80pio_port_a_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(mz_state, mz800_z80pio_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( mz700 )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "1z-013a.rom", 0x0000, 0x1000, CRC(4c6c6b7b) SHA1(ef8f7399e86c1dc638a5cb83efdb73369c2b5735) )

	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "mz700fon.int", 0x0000, 0x1000, CRC(42b9e8fb) SHA1(5128ad179a702f8e0bd9910a58bad8fbe4c20167) )
ROM_END

ROM_START( mz700j )
	ROM_REGION( 0x1000, "monitor", 0 )
	ROM_LOAD( "1z-009b.rom", 0x0000, 0x1000, CRC(ab1fbe6f) SHA1(7b10d7965c541393e33a265bcf71a00314d2db7a))

	ROM_REGION( 0x1000, "cgrom", 0 )
	//ROM_LOAD( "mz700fon.jp", 0x0000, 0x1000, CRC(697ec121) SHA1(5eb1d42d273b1fd2cab120486279ab8ff6c85dc7))
	ROM_LOAD( "mz700fon.jpn", 0x0000, 0x1000, CRC(425eedf5) SHA1(bd2cc750f2d2f63e50a59786668509e81a276e32) )
ROM_END

ROM_START( mz800 )
	ROM_REGION( 0x4000, "monitor", 0 )
	ROM_LOAD( "mz800.rom", 0x0000, 0x4000, CRC(600d17e1) SHA1(950ce4b51429916f8036e41ba6130fac149b36e4) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 ) // ramdisk
ROM_END

ROM_START( mz1500 )
	ROM_REGION( 0x4000, "monitor", 0 )
	ROM_LOAD( "9z-502m.rom",  0x0000, 0x1000, CRC(643db428) SHA1(c2ad8af2ef00db32afde54d5741b07de5d4da16a))
	ROM_CONTINUE(0x2800, 0x1800)

	ROM_REGION( 0x1000, "cgrom", 0 )
	//ROM_LOAD( "mz700fon.jp", 0x0000, 0x1000, CRC(697ec121) SHA1(5eb1d42d273b1fd2cab120486279ab8ff6c85dc7))
	ROM_LOAD( "mz700fon.jpn", 0x0000, 0x1000, CRC(425eedf5) SHA1(bd2cc750f2d2f63e50a59786668509e81a276e32) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 ) // ramdisk
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT    COMPANY      FULLNAME */
COMP( 1982, mz700,    0,        0,      mz700,    mz700, mz_state,    mz700,  "Sharp",     "MZ-700", 0 )
COMP( 1982, mz700j,   mz700,    0,      mz700,    mz700, mz_state,    mz700,  "Sharp",     "MZ-700 (Japan)", 0 )
COMP( 1984, mz800,    0,        0,      mz800,    mz800, mz_state,    mz800,  "Sharp",     "MZ-800", MACHINE_NOT_WORKING )
COMP( 1984, mz1500,   0,        0,      mz800,    mz800, mz_state,    mz800,  "Sharp",     "MZ-1500", MACHINE_NOT_WORKING )    // Japanese version of the MZ-800
