// license:???
// copyright-holders:Paul Daniels, Colin Howell, R. Belmont
/**********************************************************************

Apple I

CPU:        6502 @ 1.023 MHz
            (Effective speed with RAM refresh waits is 0.960 MHz.)

RAM:        4-8 KB on main board (4 KB standard)

            Additional memory could be added via the expansion
            connector, but the user was responsible for making sure
            the extra memory was properly interfaced.

            Some users replaced the onboard 4-kilobit RAM chips with
            16-kilobit RAM chips, increasing on-board memory to 32 KB,
            but this required modifying the RAM interface circuitry.

ROM:        256 bytes for Monitor program

            Optional cassette interface included 256 bytes for
            cassette routines.

Interrupts: None.
            (The system board had jumpers to allow interrupts, but
            these were not connected in a standard system.)

Video:      Dumb terminal, based on 7 1K-bit shift registers

Sound:      None

Hardware:   Motorola 6820 PIA for keyboard and display interface

Memory map:

$0000-$1FFF:    RAM address space
    $0000-$00FF:    6502 zero page
        $0024-$002B:    Zero page locations used by the Monitor
    $0100-$01FF:    6502 processor stack
    $0200-$027F:    Keyboard input buffer storage used by the Monitor
    $0280-$0FFF:    RAM space available for a program in a 4 KB system
    $1000-$1FFF:    Extra RAM space available for a program in an 8 KB system
                    not using cassette BASIC

$2000-$BFFF:    Unused address space, available for RAM in systems larger
                than 8 KB.

$C000-$CFFF:    Address space for optional cassette interface
    $C000-$C0FF:    Cassette interface I/O range
    $C100-$C1FF:    Cassette interface ROM

$D000-$DFFF:    I/O address space
    $D010-$D013:    Motorola 6820 PIA registers.
        $D010:          Keyboard input port
        $D011:          Control register for keyboard input port, with
                        key-available flag.
        $D012:          Display output port (bit 7 is a status input)
        $D013:          Control register for display output port
    (PIA registers also mirrored at $D014-$D017, $D018-$D01B, $D01C-$D01F,
    $D030-$D03F, $D050-$D05F, ... , $DFD0-$DFDF, $DFF0-$DFFF.)

$E000-$EFFF:    Extra RAM space available for a program in an 8 KB system
                modified to use cassette BASIC
                (The system simulated here always includes this RAM.)

If you wanted to load the BASIC as rom, here are the details:
ROM_LOAD("basic.bin", 0xE000, 0x1000, CRC(d5e86efc) SHA1(04269c1c66e7d5b4aa5035462c6e612bf2ae9b91) )


$F000-$FFFF:    ROM address space
    $FF00-$FFFF:    Apple Monitor ROM


How to use cassettes:
The system has no error checking or checksums, and the cassette
has no header.
Therefore, you must know the details, and pass these to the
interface yourself.
BASIC has no cassette handling. You must enter the monitor
with: CALL -151
then when finished, re-enter BASIC with: E2B3R


Examples:

A machine-language program will typically be like this:
C100R    (enter the interface)
0300.0FFFR  (enter the load and end addresses, then load the tape)
You start the tape.
When the prompt returns you stop the tape.
0300R  (run your program)


To Load Tape Basic:
C100R
E000.EFFFR
You start the tape.
When the prompt returns you stop the tape.
E000R  (It must say 4C - if not, your tape is no good).
The BASIC prompt will appear
>@


A BASIC program is split into two areas, one for the scratch pad,
and one for the program proper.
In BASIC you may have to adjust the allowed memory area, such as
LOMEM = 768
Then, go to the monitor: CALL -151
C100R    (enter the interface)
00A4.00FFR 0300.0FFFR   (load the 2 parts)
You start the tape.
When the prompt returns you stop the tape.
E2B3R    (back to BASIC)
You can LIST or RUN now.


Saving is almost the same, when you specify the address range, enter
W instead of R. The difficulty is finding out how long your program is.

Insert a blank tape
C100R
0300.0FFFW
Quickly press Record.
When the prompt returns, press Stop.

**********************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "includes/apple1.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"

#include "bus/a1bus/a1bus.h"
#include "bus/a1bus/a1cassette.h"
#include "bus/a1bus/a1cffa.h"

/* port i/o functions */

/* memory w/r functions */

static ADDRESS_MAP_START( apple1_map, AS_PROGRAM, 8, apple1_state )
	/* In $D000-$DFFF, PIA is selected by address bit 4 being high,
	   and PIA registers are addressed with address bits 0-1.  All
	   other address bits are ignored.  Thus $D010-$D013 is mirrored
	   at all $Dxxx addresses with bit 4 high. */
	AM_RANGE(0xd010, 0xd013) AM_MIRROR(0x0fec) AM_DEVREADWRITE("pia",pia6821_device, read, write)

	/* We always include the remapped RAM for cassette BASIC, both for
	   simplicity and to allow the running of BASIC programs. */
	AM_RANGE(0xe000, 0xefff) AM_RAM

	AM_RANGE(0xf000, 0xfeff) AM_NOP

	/* Monitor ROM: */
	AM_RANGE(0xff00, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

/* graphics output */

const gfx_layout apple1_charlayout =
{
	7, 8,               /* character cell is 7 pixels wide by 8 pixels high */
	64,                 /* 64 characters in 2513 character generator ROM */
	1,                  /* 1 bitplane */
	{ 0 },
	/* 5 visible pixels per row, starting at bit 3, with MSB being 0: */
	{ 3, 4, 5, 6, 7 },
	/* pixel rows stored from top to bottom: */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8 * 8               /* 8 8-bit pixel rows per character */
};

static GFXDECODE_START( apple1 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, apple1_charlayout, 0, 1 )
GFXDECODE_END

/* keyboard input */
/*
   It's very likely that the keyboard assgnments are totally wrong: the code in machine/apple1.c
   makes arbitrary assumptions about the mapping of the keys. The schematics that are available
   on the web can help revealing the real layout.
   The large picture of Woz's Apple I at http://home.earthlink.net/~judgementcall/apple1.jpg
   show probably how the real keyboard was meant to be: note how the shifted symbols on the digits
   and on some letters are different from the ones produced by current emulation and the presence
   of the gray keys.
*/

static INPUT_PORTS_START( apple1 )
	PORT_START("KEY0")  /* first sixteen keys */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)        PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)        PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)        PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)        PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)        PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)        PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)    PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)    PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)    PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("KEY1")  /* second sixteen keys */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)        PORT_CHAR('A')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)        PORT_CHAR('B')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)        PORT_CHAR('C')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)        PORT_CHAR('D')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)        PORT_CHAR('E')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)        PORT_CHAR('F')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)        PORT_CHAR('G')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)        PORT_CHAR('H')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)        PORT_CHAR('I')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)        PORT_CHAR('J')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)        PORT_CHAR('K')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)        PORT_CHAR('L')

	PORT_START("KEY2")  /* third sixteen keys */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)        PORT_CHAR('M')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)        PORT_CHAR('N')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)        PORT_CHAR('O')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)        PORT_CHAR('P')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)        PORT_CHAR('Q')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)        PORT_CHAR('R')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)        PORT_CHAR('S')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)        PORT_CHAR('T')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)        PORT_CHAR('U')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)        PORT_CHAR('V')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)        PORT_CHAR('W')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)        PORT_CHAR('X')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)        PORT_CHAR('Y')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)        PORT_CHAR('Z')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backarrow") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('_')

	PORT_START("KEY3")  /* fourth sixteen keys */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KEY4")  /* shift keys */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control (Left)") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control (Right)") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("KEY5")  /* RESET and CLEAR SCREEN pushbutton switches */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
INPUT_PORTS_END

static SLOT_INTERFACE_START(apple1_cards)
	SLOT_INTERFACE("cassette", A1BUS_CASSETTE)
	SLOT_INTERFACE("cffa", A1BUS_CFFA)
SLOT_INTERFACE_END

/* machine definition */
static MACHINE_CONFIG_START( apple1, apple1_state )
	/* basic machine hardware */
	/* Actual CPU speed is 1.023 MHz, but RAM refresh effectively
	   slows it to 960 kHz. */
	MCFG_CPU_ADD("maincpu", M6502, 960000)        /* 1.023 MHz */
	MCFG_CPU_PROGRAM_MAP(apple1_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	/* Video is blanked for 70 out of 262 scanlines per refresh cycle.
	   Each scanline is composed of 65 character times, 40 of which
	   are visible, and each character time is 7 dot times; a dot time
	   is 2 cycles of the fundamental 14.31818 MHz oscillator.  The
	   total blanking time is about 4450 microseconds. */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC((int) (70 * 65 * 7 * 2 / 14.31818)))
	/* It would be nice if we could implement some sort of display
	   overscan here. */
	MCFG_SCREEN_SIZE(40 * 7, 24 * 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 7 - 1, 0, 24 * 8 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(apple1_state, screen_update_apple1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", apple1)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD( "pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(apple1_state,apple1_pia0_kbdin))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(apple1_state,apple1_pia0_dspout))
	MCFG_PIA_CB2_HANDLER(WRITELINE(apple1_state,apple1_pia0_dsp_write_signal))

	MCFG_DEVICE_ADD("a1bus", A1BUS, 0)
	MCFG_A1BUS_CPU("maincpu")
	MCFG_A1BUS_SLOT_ADD("a1bus", "exp", apple1_cards, "cassette")

	/* snapshot */
	MCFG_SNAPSHOT_ADD("snapshot", apple1_state, apple1, "snp", 0)

	MCFG_SOFTWARE_LIST_ADD("cass_list","apple1")

	/* Note that because we always include 4K of RAM at $E000-$EFFF,
	   the RAM amounts listed here will be 4K below the actual RAM
	   total. */
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("48K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,28K,32K,36K,40K,44K")

MACHINE_CONFIG_END

ROM_START(apple1)
	ROM_REGION(0x100, "maincpu",0)
	/* 256-byte main monitor ROM, in two 82s129 or mmi6301 256x4 proms at A1 and A2 called APPLE-A1(bits D3-D0) and APPLE-A2(bits D7-D4) */
	ROM_LOAD_NIB_HIGH( "apple-a2.a2",    0x0000, 0x0100, CRC(254bfb95) SHA1(b6468b72295b7d8ac288d104d252f24de1f1d611) )
	ROM_LOAD_NIB_LOW( "apple-a1.a1",    0x0000, 0x0100, CRC(434f8ce6) SHA1(9deee2d39903209b20c3fc6b58e16372f8efece1) )
	/* 512-byte Signetics 2513 character generator ROM at location D2-D3 */
	ROM_REGION(0x0200, "gfx1",0)
	ROM_LOAD("s2513.d2", 0x0000, 0x0200, CRC(a7e567fc) SHA1(b18aae0a2d4f92f5a7e22640719bbc4652f3f4ee)) // apple1.vid
ROM_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY             FULLNAME */
COMP( 1976, apple1, 0,      0,      apple1,     apple1, apple1_state,       apple1, "Apple Computer",   "Apple I" ,  MACHINE_NO_SOUND )
