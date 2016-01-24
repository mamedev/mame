// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    MESS Driver for TI-99/4 and TI-99/4A Home Computers.
    TI99/4 info:

    Similar to TI99/4a, except for the following:
    * tms9918/9928 has no bitmap mode
    * smaller, 40-key keyboard
    * many small differences in the contents of system ROMs

    Historical notes: TI made several last minute design changes.
    * TI99/4 prototypes had an extra port for an I/R joystick and keypad interface.
    * early TI99/4 prototypes were designed for a tms9985, not a tms9900.

    Emulation architecture:
    (also see datamux.c, peribox.c)

              +---- video (upper 8 bits of databus)
              |
    CPU       |
    TMS9900 ==##== datamux --------+------ peribox ----------- [8] slots with peripheral devices
       |      ||   (16->8)         |
       |      ||                   +------ Console GROMs (0, 1, 2)
       |      ##=== Console ROM    |
    TMS9901   ||                   +------ gromport (cartridge port)
     (I/O)    ##=== 256 byte RAM   |
       |      ||                   +------ sound
       |      ##=== 32KiB RAM
   +---+--+         unofficial mod
   |      |         (16 bit)
  Cass  joyport

  Raphael Nabet, 1999-2003.
  Rewritten by Michael Zapf
  February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900.h"

#include "sound/wave.h"
#include "video/v9938.h"
#include "machine/tms9901.h"
#include "imagedev/cassette.h"

#include "bus/ti99x/videowrp.h"
#include "bus/ti99x/datamux.h"
#include "bus/ti99x/grom.h"
#include "bus/ti99x/gromport.h"
#include "bus/ti99x/joyport.h"

#include "bus/ti99_peb/peribox.h"
#include "softlist.h"

// Debugging
#define TRACE_READY 0
#define TRACE_INTERRUPTS 0
#define TRACE_CRU 0

/*
    The console.
*/
class ti99_4x_state : public driver_device
{
public:
	ti99_4x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_tms9901(*this, TMS9901_TAG),
		m_gromport(*this, GROMPORT_TAG),
		m_peribox(*this, PERIBOX_TAG),
		m_joyport(*this, JOYPORT_TAG),
		m_datamux(*this, DATAMUX_TAG),
		m_video(*this, VIDEO_SYSTEM_TAG),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2")
		{ }

	// Machine management
	DECLARE_MACHINE_START(ti99_4);
	DECLARE_MACHINE_START(ti99_4a);
	DECLARE_MACHINE_START(ti99_4qi);
	DECLARE_MACHINE_RESET(ti99_4);
	DECLARE_MACHINE_RESET(ti99_4a);

	// Processor connections with the main board
	DECLARE_READ8_MEMBER( cruread );
	DECLARE_READ8_MEMBER( interrupt_level );
	DECLARE_WRITE8_MEMBER( cruwrite );
	DECLARE_WRITE8_MEMBER( external_operation );
	DECLARE_WRITE_LINE_MEMBER( clock_out );
	DECLARE_WRITE_LINE_MEMBER( dbin_line );

	// Connections from outside towards the CPU (callbacks)
	DECLARE_WRITE_LINE_MEMBER( console_ready_dmux );
	DECLARE_WRITE_LINE_MEMBER( console_ready_sound );
	DECLARE_WRITE_LINE_MEMBER( console_ready_pbox );
	DECLARE_WRITE_LINE_MEMBER( console_ready_cart );
	DECLARE_WRITE_LINE_MEMBER( console_ready_grom );
	DECLARE_WRITE_LINE_MEMBER( console_reset );
	DECLARE_WRITE_LINE_MEMBER( notconnected );

	// Connections with the system interface chip 9901
	DECLARE_WRITE_LINE_MEMBER( extint );
	DECLARE_WRITE_LINE_MEMBER( video_interrupt_in );
	DECLARE_WRITE_LINE_MEMBER( handset_interrupt_in );

	// Connections with the system interface TMS9901
	DECLARE_READ8_MEMBER(read_by_9901);
	DECLARE_WRITE_LINE_MEMBER(keyC0);
	DECLARE_WRITE_LINE_MEMBER(keyC1);
	DECLARE_WRITE_LINE_MEMBER(keyC2);
	DECLARE_WRITE_LINE_MEMBER(cs1_motor);
	DECLARE_WRITE_LINE_MEMBER(audio_gate);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);
	DECLARE_WRITE8_MEMBER(tms9901_interrupt);
	DECLARE_WRITE_LINE_MEMBER(handset_ack);
	DECLARE_WRITE_LINE_MEMBER(cs2_motor);
	DECLARE_WRITE_LINE_MEMBER(alphaW);

	// Interrupt triggers
	DECLARE_INPUT_CHANGED_MEMBER( load_interrupt );

private:
	void    set_keyboard_column(int number, int data);
	int     m_keyboard_column;
	int     m_check_alphalock;

	// READY handling
	int     m_nready_combined;
	int     m_nready_prev;
	void    console_ready_join(int id, int state);

	// Console type
	int     m_model;

	// Latch for 9901 INT1, INT2, and INT12 lines
	line_state  m_int1;
	line_state  m_int2;
	line_state  m_int12;

	// Connected devices
	required_device<tms9900_device>     m_cpu;
	required_device<tms9901_device>     m_tms9901;
	required_device<gromport_device>    m_gromport;
	required_device<peribox_device>     m_peribox;
	required_device<joyport_device>     m_joyport;
	required_device<ti99_datamux_device> m_datamux;
	required_device<ti_video_device>    m_video;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
};

/*
    Console models.
*/
enum
{
	MODEL_4,
	MODEL_4A,
	MODEL_4QI
};

/*
    READY bits.
*/
enum
{
	READY_GROM = 1,
	READY_DMUX = 2,
	READY_PBOX = 4,
	READY_SOUND = 8,
	READY_CART = 16
};

/*
    Memory map.
    Most of the work is done in the datamux (see datamux.c). We only keep ROM
    and the small 256 byte PAD RAM here because they are directly connected
    to the 16bit bus, and the wait state logic is not active during their
    accesses.
*/
static ADDRESS_MAP_START(memmap, AS_PROGRAM, 16, ti99_4x_state)
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8000, 0x80ff) AM_MIRROR(0x0300) AM_RAM
	AM_RANGE(0x0000, 0xffff) AM_DEVREADWRITE(DATAMUX_TAG, ti99_datamux_device, read, write) AM_DEVSETOFFSET(DATAMUX_TAG, ti99_datamux_device, setoffset)
ADDRESS_MAP_END

/*
    CRU map
    TMS9900 CRU address space is 12 bits wide, attached to A3-A14, A0-A2 must
    be 000 (other values for external commands like RSET, LREX, CKON...),
    A15 is used as CRUOUT
    The TMS9901 is incompletely decoded
    ---0 00xx xxcc ccc0
    causing 16 mirrors (0000, 0040, 0080, 00c0, ... , 03c0)

    Reading is done by transfering 8 successive bits, so addresses refer to
    8 bit groups; writing, however, is done using output lines. The CRU base
    address in the ti99 systems is twice the bit address:

    (base=0, bit=0x10) == (base=0x20,bit=0)

    Read: 0000 - 003f translates to base addresses 0000 - 03fe
          0000 - 01ff is the complete CRU address space 0000 - 1ffe (for TMS9900)

    Write:0000 - 01ff corresponds to bit 0 of base address 0000 - 03fe
*/
static ADDRESS_MAP_START(cru_map, AS_IO, 8, ti99_4x_state)
	AM_RANGE(0x0000, 0x0003) AM_MIRROR(0x003c) AM_DEVREAD(TMS9901_TAG, tms9901_device, read)
	AM_RANGE(0x0000, 0x01ff) AM_READ(cruread)

	AM_RANGE(0x0000, 0x001f) AM_MIRROR(0x01e0) AM_DEVWRITE(TMS9901_TAG, tms9901_device, write)
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(cruwrite)
ADDRESS_MAP_END


/*****************************************************************************
    Input ports
 ****************************************************************************/

static INPUT_PORTS_START(ti99_4)
	PORT_START("COL0")  // col 0
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q QUIT") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR(UCHAR_MAMEKEY(F12))
		/* TI99/4 has a second space key which maps the same */
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(' ')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P \"") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('"')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L =") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('=')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("COL1")  // col 1
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W BEGIN") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A AID") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z BACK") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O +") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('+')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K /") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('/')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", .") PORT_CODE(KEYCODE_STOP) PORT_CHAR(',') PORT_CHAR('.')

	PORT_START("COL2")  // col 2
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E UP") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S LEFT") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X DOWN") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I -") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('-')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J ^") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('^')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M ;") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(';')
				/* col 3 */
	PORT_START("COL3")  // col 3
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R REDO") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR(UCHAR_MAMEKEY(F8))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D RIGHT") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C CLEAR") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U _") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H <") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('<')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N :") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(':')

	PORT_START("COL4")  // col 4
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T ERASE") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR(UCHAR_MAMEKEY(F3))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F DEL") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V PROC'D") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\'')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y >") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('>')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G INS") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B ?") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('?')

INPUT_PORTS_END

/* TI99/4a: 48-key keyboard */
static INPUT_PORTS_START(ti99_4a)
	PORT_START( "ALPHABUG" )
		PORT_CONFNAME( 0x01, 0x01, "Alpha Lock blocks joystick up" )
		PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
		PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START( "LOADINT ")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load interrupt") PORT_CODE(KEYCODE_PRTSCR) PORT_CHANGED_MEMBER(DEVICE_SELF, ti99_4x_state, load_interrupt, 1)

	PORT_START("COL0")  // col 0
		PORT_BIT(0x88, IP_ACTIVE_LOW, IPT_UNUSED)
		/* The original control key is located on the left, but we accept the right control key as well */
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")      PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		/* TI99/4a has a second shift key which maps the same */
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		/* The original function key is located on the right, but we accept the left alt key as well */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN")      PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT")  PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHAR(UCHAR_MAMEKEY(F12))

	PORT_START("COL1")  // col 1
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR('~')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK")  PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR('\'')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("COL2")  // col 2
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('`')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 # ERASE") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(UCHAR_MAMEKEY(F3))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO")  PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(UCHAR_MAMEKEY(F8))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR('?')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("COL3")  // col 3
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('[')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID")   PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR('_')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("COL4")  // col 4
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(']')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN")  PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("COL5")  // col 5
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR('\\')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR('|')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR(UCHAR_MAMEKEY(F10))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('\"')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('-')

	PORT_START("ALPHA") /* one more port for Alpha line */
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alpha Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	/* another version of Alpha Lock which is non-toggling; this is useful when we want to attach
	    a real TI keyboard for input. For home computers, the Alpha Lock / Shift Lock was a physically
	    locking key. */
	PORT_START("ALPHA1")
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alpha Lock non-toggle") PORT_CODE(KEYCODE_RWIN)

INPUT_PORTS_END


/*****************************************************************************
    Components
******************************************************************************/

static GROM_CONFIG(grom0_config)
{
	false, 0, region_grom, 0x0000, 0x1800, GROMFREQ
};

static GROM_CONFIG(grom1_config)
{
	false, 1, region_grom, 0x2000, 0x1800, GROMFREQ
};

static GROM_CONFIG(grom2_config)
{
	false, 2, region_grom, 0x4000, 0x1800, GROMFREQ
};

READ8_MEMBER( ti99_4x_state::cruread )
{
//  if (TRACE_CRU) logerror("read access to CRU address %04x\n", offset << 4);
	UINT8 value = 0;

	// Similar to the bus8z_devices, just let the gromport and the p-box
	// decide whether they want to change the value at the CRU address
	// Also, we translate the bit addresses to base addresses

	// The QI version does not propagate the CRU signals to the cartridge slot
	if (m_model != MODEL_4QI) m_gromport->crureadz(space, offset<<4, &value);
	m_peribox->crureadz(space, offset<<4, &value);

	return value;
}

WRITE8_MEMBER( ti99_4x_state::cruwrite )
{
	if (TRACE_CRU) logerror("ti99_4x: write access to CRU address %04x\n", offset << 1);
	// The QI version does not propagate the CRU signals to the cartridge slot
	if (m_model != MODEL_4QI) m_gromport->cruwrite(space, offset<<1, data);
	m_peribox->cruwrite(space, offset<<1, data);
}

WRITE8_MEMBER( ti99_4x_state::external_operation )
{
	static const char* extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	// Some games (e.g. Slymoids) actually use IDLE for synchronization
	if (offset == IDLE_OP) return;
	else
	{
		logerror("ti99_4x: External operation %s not implemented on TI-99 board\n", extop[offset]);
	}
}

/***************************************************************************
    TI99/4x-specific tms9901 I/O handlers

    See mess/machine/tms9901.c for generic tms9901 CRU handlers.

    TMS9901 interrupt handling on a TI99/4(a).

    TI99/4(a) uses the following interrupts:
    INT1: external interrupt (used by RS232 controller, for instance)
    INT2: VDP interrupt
    TMS9901 timer interrupt (overrides INT3)
    INT12: handset interrupt (only on a TI-99/4 with the handset prototypes)

    Three (occasionally four) interrupts are used by the system (INT1, INT2,
    timer, and INT12 on a TI-99/4 with remote handset prototypes), out of 15/16
    possible interrupts.  Keyboard pins can be used as interrupt pins, too, but
    this is not emulated (it's a trick, anyway, and I don't know any program
    which uses it).

    When an interrupt line is set (and the corresponding bit in the interrupt mask is set),
    a level 1 interrupt is requested from the TMS9900.  This interrupt request lasts as long as
    the interrupt pin and the relevant bit in the interrupt mask are set.

***************************************************************************/


static const char *const column[] = { "COL0", "COL1", "COL2", "COL3", "COL4", "COL5" };

READ8_MEMBER( ti99_4x_state::read_by_9901 )
{
	int answer=0;

	switch (offset & 0x03)
	{
	case TMS9901_CB_INT7:
		//
		// Read pins INT3*-INT7* of TI99's 9901.
		// bit 1: INT1 status
		// bit 2: INT2 status
		// bit 3-7: keyboard status bits 0 to 4
		//
		// |K|K|K|K|K|I2|I1|C|
		//
		if (m_keyboard_column >= (m_model==MODEL_4? 5:6)) // joy 1, 2, handset
		{
			answer = m_joyport->read_port();
			// The hardware bug of the TI-99/4A: you have to release the
			// Alphalock key when using joysticks. This is a maldesign of the
			// board: When none of the other keyboard lines are selected the
			// depressed Alphalock key pulls up the /INT7 line which is also
			// used for joystick up. The joystick switch then fails to lower
			// the line enough to make the TMS9901 sense the low level.
			// A reported, feasible fix was to cut the line and insert a diode
			// below the Alphalock key.
			if ((m_model!=MODEL_4) && (ioport("ALPHABUG")->read()!=0) ) answer |= (ioport("ALPHA")->read() | ioport("ALPHA1")->read());
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		if (m_check_alphalock)  // never true for TI-99/4
		{
			answer &= ~(ioport("ALPHA")->read() | ioport("ALPHA1")->read());
		}
		answer = (answer << 3);
		if (m_int1 == CLEAR_LINE) answer |= 0x02;
		if (m_int2 == CLEAR_LINE) answer |= 0x04;

		break;

	case TMS9901_INT8_INT15:
		// |1|1|1|INT12|0|K|K|K|
		if (m_keyboard_column >= (m_model==MODEL_4? 5:6)) answer = 0x07;
		else answer = ((ioport(column[m_keyboard_column])->read())>>5) & 0x07;
		answer |= 0xe0;
		if (m_model != MODEL_4 || m_int12==CLEAR_LINE) answer |= 0x10;
		break;

	case TMS9901_P0_P7:
		// Required for the handset (only on TI-99/4)
		if ((m_joyport->read_port() & 0x20)!=0) answer |= 2;
		break;

	case TMS9901_P8_P15:
		// Preset to 1
		answer = 4;

		// Interrupt pin of the handset (only on TI-99/4)
		// Negative logic (interrupt pulls line down)
		if ((m_joyport->read_port() & 0x40)==0) answer = 0;

		// we don't take CS2 into account, as CS2 is a write-only unit
		if (m_cassette1->input() > 0)
		{
			answer |= 8;
		}
		break;
	}
	return answer;
}

/*
    Handler for TMS9901 P0 pin (handset data acknowledge); only for 99/4
*/
WRITE_LINE_MEMBER( ti99_4x_state::handset_ack )
{
	// Write a value to the joyport. If there is a handset this will set its
	// ACK line.
	m_joyport->write_port(state==ASSERT_LINE? 0x01 : 0x00);
}

/*
    WRITE key column select (P2-P4), TI-99/4
*/
void ti99_4x_state::set_keyboard_column(int number, int data)
{
	if (data != 0)
		m_keyboard_column |= 1 << number;
	else
		m_keyboard_column &= ~ (1 << number);

	if (m_keyboard_column >= (m_model==MODEL_4? 5:6))
	{
		m_joyport->write_port(m_keyboard_column - (m_model==MODEL_4? 5:6) + 1);
	}

	// TI-99/4:  joystick 1 = column 5
	//           joystick 2 = column 6
	// (only for the prototype versions; the released versions had no IR
	// handset and the board was already redesigned to use columns 6 and 7)

	// TI-99/4A: joystick 1 = column 6
	//           joystick 2 = column 7
}

WRITE_LINE_MEMBER( ti99_4x_state::keyC0 )
{
	set_keyboard_column(0, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::keyC1 )
{
	set_keyboard_column(1, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::keyC2 )
{
	set_keyboard_column(2, state);
}

/*
    Select alpha lock line - TI99/4a only (P5)
*/
WRITE_LINE_MEMBER( ti99_4x_state::alphaW )
{
	m_check_alphalock = (state==0);
}

/*
    Control CS1 tape unit motor (P6)
*/
WRITE_LINE_MEMBER( ti99_4x_state::cs1_motor )
{
	m_cassette1->change_state(state==ASSERT_LINE? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

/*
    Control CS2 tape unit motor (P7)
*/
WRITE_LINE_MEMBER( ti99_4x_state::cs2_motor )
{
	m_cassette2->change_state(state==ASSERT_LINE? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

/*
    Audio gate (P8)
    Set to 1 before using tape: this enables the mixing of tape input sound
    with computer sound.
    We do not really need to emulate this as the tape recorder generates sound
    on its own.
*/
WRITE_LINE_MEMBER( ti99_4x_state::audio_gate )
{
}

/*
    Tape output (P9)
    I think polarity is correct, but don't take my word for it.
*/
WRITE_LINE_MEMBER( ti99_4x_state::cassette_output )
{
	m_cassette1->output(state==ASSERT_LINE? +1 : -1);
	m_cassette2->output(state==ASSERT_LINE? +1 : -1);
}

WRITE8_MEMBER( ti99_4x_state::tms9901_interrupt )
{
	// offset contains the interrupt level (0-15)
	// However, the TI board just ignores that level and hardwires it to 1
	// See below (interrupt_level)
	m_cpu->set_input_line(INT_9900_INTREQ, data);
}

READ8_MEMBER( ti99_4x_state::interrupt_level )
{
	// On the TI-99 systems these IC lines are not used; the input lines
	// at the CPU are hardwired to level 1.
	return 1;
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
WRITE_LINE_MEMBER( ti99_4x_state::clock_out )
{
	m_datamux->clock_in(state);
}

/*
   Data bus in (DBIN) line from the CPU.
*/
WRITE_LINE_MEMBER( ti99_4x_state::dbin_line )
{
	m_datamux->dbin_in(state);
}

/*****************************************************************************/

/*
    set the state of TMS9901's INT2 (called by the tms9928 core)
*/
WRITE_LINE_MEMBER( ti99_4x_state::video_interrupt_in )
{
	if (TRACE_INTERRUPTS) logerror("ti99_4x: VDP INT2 on tms9901, level=%d\n", state);

	// Pulse for the handset
	if (m_model == MODEL_4) m_joyport->pulse_clock();

	m_int2 = (line_state)state;
	m_tms9901->set_single_int(2, state);
}

/*
    set the state of TMS9901's INT12 (called by the handset prototype of TI-99/4)
*/
WRITE_LINE_MEMBER( ti99_4x_state::handset_interrupt_in)
{
	if (TRACE_INTERRUPTS) logerror("ti99_4x: joyport INT12 on tms9901, level=%d\n", state);
	m_int12 = (line_state)state;
	m_tms9901->set_single_int(12, state);
}

/*
    One of the common hardware mods was to add a switch to trigger a LOAD
    interrupt
*/
INPUT_CHANGED_MEMBER( ti99_4x_state::load_interrupt )
{
	logerror("ti99_4x: LOAD interrupt, level=%d\n", newval);
	m_cpu->set_input_line(INT_9900_LOAD, (newval==0)? ASSERT_LINE : CLEAR_LINE);
}

/***********************************************************
    Links to external devices
***********************************************************/

/*
    We combine the incoming READY signals and propagate them to the CPU.
    An alternative would be to let the CPU get the READY state, but this would
    be a much higher overhead, as this happens in each clock tick.
*/
void ti99_4x_state::console_ready_join(int id, int state)
{
	if (state==CLEAR_LINE)
		m_nready_combined |= id;
	else
		m_nready_combined &= ~id;

	if (TRACE_READY)
	{
		if (m_nready_prev != m_nready_combined) logerror("ti99_4x: READY bits = %04x\n", ~m_nready_combined);
	}

	m_nready_prev = m_nready_combined;
	m_cpu->set_ready(m_nready_combined==0);
}

/*
    Connections to the READY line. This might look a bit ugly; we need an
    implementation of a "Wired AND" device.
*/
WRITE_LINE_MEMBER( ti99_4x_state::console_ready_grom )
{
	console_ready_join(READY_GROM, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::console_ready_dmux )
{
	console_ready_join(READY_DMUX, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::console_ready_pbox )
{
	console_ready_join(READY_PBOX, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::console_ready_sound )
{
	console_ready_join(READY_SOUND, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::console_ready_cart )
{
	console_ready_join(READY_CART, state);
}

/*
    The RESET line leading to a reset of the CPU. This is asserted when a
    cartridge is plugged in.
*/
WRITE_LINE_MEMBER( ti99_4x_state::console_reset )
{
	if (machine().phase() != MACHINE_PHASE_INIT)
	{
		logerror("ti99_4x: Console reset line = %d\n", state);
		m_cpu->set_input_line(INT_9900_RESET, state);
		m_video->reset_vdp(state);
	}
}

WRITE_LINE_MEMBER( ti99_4x_state::extint )
{
	if (TRACE_INTERRUPTS) logerror("ti99_4x: EXTINT level = %02x\n", state);
	m_int1 = (line_state)state;
	m_tms9901->set_single_int(1, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::notconnected )
{
	if (TRACE_INTERRUPTS) logerror("ti99_4x: Setting a not connected line ... ignored\n");
}

/*****************************************************************************/

/*
    Devices attached to the databus multiplexer. We cannot solve this with
    the common address maps since the multiplexer also inserts wait states
    that we want to emulate properly. Also, devices may reside on the same
    memory locations (like GROMs) and select themselves according to some
    inner state (e.g. GROMs have an own address counter and a given address
    area).
*/
static const dmux_device_list_entry dmux_devices[] =
{
	{ VIDEO_SYSTEM_TAG, 0x8800, 0xfc01, 0x0400, nullptr, 0, 0 },
	{ GROM0_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ GROM1_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ GROM2_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ TISOUND_TAG,   0x8400, 0xfc01, 0x0000, nullptr, 0, 0 },
	{ GROMPORT_TAG,  0x9800, 0xfc01, 0x0400, nullptr, 0, 0 },
	{ GROMPORT_TAG,  0x6000, 0xe000, 0x0000, nullptr, 0, 0 },
	{ PERIBOX_TAG,   0x0000, 0x0000, 0x0000, nullptr, 0, 0 },  // Peribox needs all addresses
	{ nullptr, 0, 0, 0, nullptr, 0, 0  }
};

static const dmux_device_list_entry dmux_devices_ev[] =
{
	{ VIDEO_SYSTEM_TAG, 0x8800, 0xfc01, 0x0400, nullptr, 0, 0 },
	{ GROM0_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ GROM1_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ GROM2_TAG,     0x9800, 0xfc01, 0x0400, "GROMENA", 0x01, 0x00 },
	{ TISOUND_TAG,   0x8400, 0xfc01, 0x0000, nullptr, 0, 0 },
	{ GROMPORT_TAG,  0x9800, 0xfc01, 0x0400, nullptr, 0, 0 },
	{ GROMPORT_TAG,  0x6000, 0xe000, 0x0000, nullptr, 0, 0 },
	{ PERIBOX_TAG,   0x0000, 0x0000, 0x0000, nullptr, 0, 0 },  // Peribox needs all addresses
	{ nullptr, 0, 0, 0, nullptr, 0, 0  }
};

static DMUX_CONFIG( datamux_conf )
{
	dmux_devices
};

static DMUX_CONFIG( datamux_conf_ev )
{
	dmux_devices_ev
};

/******************************************************************************
    Machine definitions
******************************************************************************/

MACHINE_START_MEMBER(ti99_4x_state,ti99_4)
{
	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);
	m_nready_combined = 0;
	m_model = MODEL_4;
}

MACHINE_RESET_MEMBER(ti99_4x_state,ti99_4)
{
	m_cpu->set_ready(ASSERT_LINE);
	m_cpu->set_hold(CLEAR_LINE);
	m_int1 = CLEAR_LINE;
	m_int2 = CLEAR_LINE;
	m_int12 = CLEAR_LINE;
}

/**********************************************************************
    TI-99/4 - predecessor of the more popular TI-99/4A
***********************************************************************/

static MACHINE_CONFIG_START( ti99_4, ti99_4x_state )
	// CPU
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map)
	MCFG_TMS99xx_EXTOP_HANDLER( WRITE8(ti99_4x_state, external_operation) )
	MCFG_TMS99xx_INTLEVEL_HANDLER( READ8(ti99_4x_state, interrupt_level) )
	MCFG_TMS99xx_CLKOUT_HANDLER( WRITELINE(ti99_4x_state, clock_out) )
	MCFG_TMS99xx_DBIN_HANDLER( WRITELINE(ti99_4x_state, dbin_line) )

	MCFG_MACHINE_START_OVERRIDE(ti99_4x_state, ti99_4 )
	MCFG_MACHINE_RESET_OVERRIDE(ti99_4x_state, ti99_4 )

	/* Main board */
	MCFG_DEVICE_ADD(TMS9901_TAG, TMS9901, 3000000)
	MCFG_TMS9901_READBLOCK_HANDLER( READ8(ti99_4x_state, read_by_9901) )
	MCFG_TMS9901_P0_HANDLER( WRITELINE( ti99_4x_state, handset_ack) )
	MCFG_TMS9901_P2_HANDLER( WRITELINE( ti99_4x_state, keyC0) )
	MCFG_TMS9901_P3_HANDLER( WRITELINE( ti99_4x_state, keyC1) )
	MCFG_TMS9901_P4_HANDLER( WRITELINE( ti99_4x_state, keyC2) )
	MCFG_TMS9901_P6_HANDLER( WRITELINE( ti99_4x_state, cs1_motor) )
	MCFG_TMS9901_P7_HANDLER( WRITELINE( ti99_4x_state, cs2_motor) )
	MCFG_TMS9901_P8_HANDLER( WRITELINE( ti99_4x_state, audio_gate) )
	MCFG_TMS9901_P9_HANDLER( WRITELINE( ti99_4x_state, cassette_output) )
	MCFG_TMS9901_INTLEVEL_HANDLER( WRITE8( ti99_4x_state, tms9901_interrupt) )

	MCFG_DMUX_ADD( DATAMUX_TAG, datamux_conf )
	MCFG_DMUX_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_dmux) )
	MCFG_TI99_GROMPORT_ADD( GROMPORT_TAG )
	MCFG_GROMPORT_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_cart) )
	MCFG_GROMPORT_RESET_HANDLER( WRITELINE(ti99_4x_state, console_reset) )

	/* Software list */
	MCFG_SOFTWARE_LIST_ADD("cart_list_ti99", "ti99_cart")

	/* Peripheral expansion box */
	MCFG_DEVICE_ADD( PERIBOX_TAG, PERIBOX, 0)
	MCFG_PERIBOX_INTA_HANDLER( WRITELINE(ti99_4x_state, extint) )
	MCFG_PERIBOX_INTB_HANDLER( WRITELINE(ti99_4x_state, notconnected) )
	MCFG_PERIBOX_READY_HANDLER( DEVWRITELINE(DATAMUX_TAG, ti99_datamux_device, ready_line) )

	/* sound hardware */
	MCFG_TI_SOUND_94624_ADD( TISOUND_TAG )
	MCFG_TI_SOUND_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_sound) )

	/* Cassette drives */
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_ADD( "cassette2" )

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	/* GROM devices */
	MCFG_GROM_ADD( GROM0_TAG, grom0_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM1_TAG, grom1_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM2_TAG, grom2_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))

	// Joystick port
	MCFG_TI_JOYPORT4_ADD( JOYPORT_TAG )
	MCFG_JOYPORT_INT_HANDLER( WRITELINE(ti99_4x_state, handset_interrupt_in) )
MACHINE_CONFIG_END

/*
    US version: 60 Hz, NTSC
*/
static MACHINE_CONFIG_DERIVED( ti99_4_60hz, ti99_4 )
	MCFG_TI_TMS991x_ADD_NTSC(VIDEO_SYSTEM_TAG, TMS9918, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/*
    European version: 50 Hz, PAL
*/
static MACHINE_CONFIG_DERIVED( ti99_4_50hz, ti99_4 )
	MCFG_TI_TMS991x_ADD_PAL(VIDEO_SYSTEM_TAG, TMS9929, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/**********************************************************************
    TI-99/4A - replaced the 99/4 and became the standard TI-99 console
***********************************************************************/

MACHINE_START_MEMBER(ti99_4x_state,ti99_4a)
{
	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);
	m_nready_combined = 0;
	m_model = MODEL_4A;
}

MACHINE_RESET_MEMBER(ti99_4x_state,ti99_4a)
{
	m_cpu->set_ready(ASSERT_LINE);
	m_cpu->set_hold(CLEAR_LINE);
	m_int1 = CLEAR_LINE;
	m_int2 = CLEAR_LINE;
	m_int12 = CLEAR_LINE;
}

static MACHINE_CONFIG_START( ti99_4a, ti99_4x_state )
	/* CPU */
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map)
	MCFG_TMS99xx_EXTOP_HANDLER( WRITE8(ti99_4x_state, external_operation) )
	MCFG_TMS99xx_INTLEVEL_HANDLER( READ8(ti99_4x_state, interrupt_level) )
	MCFG_TMS99xx_CLKOUT_HANDLER( WRITELINE(ti99_4x_state, clock_out) )
	MCFG_TMS99xx_DBIN_HANDLER( WRITELINE(ti99_4x_state, dbin_line) )

	MCFG_MACHINE_START_OVERRIDE(ti99_4x_state, ti99_4a )
	MCFG_MACHINE_RESET_OVERRIDE(ti99_4x_state, ti99_4a )

	/* Main board */
	MCFG_DEVICE_ADD(TMS9901_TAG, TMS9901, 3000000)
	MCFG_TMS9901_READBLOCK_HANDLER( READ8(ti99_4x_state, read_by_9901) )
	MCFG_TMS9901_P2_HANDLER( WRITELINE( ti99_4x_state, keyC0) )
	MCFG_TMS9901_P3_HANDLER( WRITELINE( ti99_4x_state, keyC1) )
	MCFG_TMS9901_P4_HANDLER( WRITELINE( ti99_4x_state, keyC2) )
	MCFG_TMS9901_P5_HANDLER( WRITELINE( ti99_4x_state, alphaW) )
	MCFG_TMS9901_P6_HANDLER( WRITELINE( ti99_4x_state, cs1_motor) )
	MCFG_TMS9901_P7_HANDLER( WRITELINE( ti99_4x_state, cs2_motor) )
	MCFG_TMS9901_P8_HANDLER( WRITELINE( ti99_4x_state, audio_gate) )
	MCFG_TMS9901_P9_HANDLER( WRITELINE( ti99_4x_state, cassette_output) )
	MCFG_TMS9901_INTLEVEL_HANDLER( WRITE8( ti99_4x_state, tms9901_interrupt) )

	MCFG_DMUX_ADD( DATAMUX_TAG, datamux_conf )
	MCFG_DMUX_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_dmux) )
	MCFG_TI99_GROMPORT_ADD( GROMPORT_TAG )
	MCFG_GROMPORT_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_cart) )
	MCFG_GROMPORT_RESET_HANDLER( WRITELINE(ti99_4x_state, console_reset) )

	/* Software list */
	MCFG_SOFTWARE_LIST_ADD("cart_list_ti99", "ti99_cart")

	/* Peripheral expansion box */
	MCFG_DEVICE_ADD( PERIBOX_TAG, PERIBOX, 0)
	MCFG_PERIBOX_INTA_HANDLER( WRITELINE(ti99_4x_state, extint) )
	MCFG_PERIBOX_INTB_HANDLER( WRITELINE(ti99_4x_state, notconnected) )
	MCFG_PERIBOX_READY_HANDLER( DEVWRITELINE(DATAMUX_TAG, ti99_datamux_device, ready_line) )

	/* sound hardware */
	MCFG_TI_SOUND_94624_ADD( TISOUND_TAG )
	MCFG_TI_SOUND_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_sound) )

	/* Cassette drives */
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_ADD( "cassette2" )

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	/* GROM devices */
	MCFG_GROM_ADD( GROM0_TAG, grom0_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM1_TAG, grom1_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM2_TAG, grom2_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG )
MACHINE_CONFIG_END

/*
    US version: 60 Hz, NTSC
*/
static MACHINE_CONFIG_DERIVED( ti99_4a_60hz, ti99_4a )
	MCFG_TI_TMS991x_ADD_NTSC(VIDEO_SYSTEM_TAG, TMS9918A, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/*
    European version: 50 Hz, PAL
*/
static MACHINE_CONFIG_DERIVED( ti99_4a_50hz, ti99_4a )
	MCFG_TI_TMS991x_ADD_PAL(VIDEO_SYSTEM_TAG, TMS9929A, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/************************************************************************
    TI-99/4QI - the final version of the TI-99/4A
    This was a last modification of the console. One purpose was to lower
    production costs by a redesigned board layout. The other was that TI
    removed the ROM search for cartridges so that only cartridges with GROMs
    could be started, effectively kicking out all third-party cartridges like
    those from Atarisoft.
*************************************************************************/

MACHINE_START_MEMBER(ti99_4x_state, ti99_4qi)
{
	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);
	m_model = MODEL_4QI;
	m_nready_combined = 0;
}

static MACHINE_CONFIG_DERIVED( ti99_4qi, ti99_4a )
	MCFG_MACHINE_START_OVERRIDE(ti99_4x_state, ti99_4qi )
MACHINE_CONFIG_END

/*
    US version: 60 Hz, NTSC
*/
static MACHINE_CONFIG_DERIVED( ti99_4qi_60hz, ti99_4qi )
	MCFG_TI_TMS991x_ADD_NTSC(VIDEO_SYSTEM_TAG, TMS9918A, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/*
    European version: 50 Hz, PAL
*/
static MACHINE_CONFIG_DERIVED( ti99_4qi_50hz, ti99_4qi )
	MCFG_TI_TMS991x_ADD_PAL(VIDEO_SYSTEM_TAG, TMS9929A, 0x4000, ti99_4x_state, video_interrupt_in)
MACHINE_CONFIG_END

/************************************************************************
    TI-99/4A with 80-column support. Actually a separate expansion card (EVPC),
    replacing the console video processor.
*************************************************************************/

static MACHINE_CONFIG_START( ti99_4ev_60hz, ti99_4x_state )
	/* CPU */
	MCFG_TMS99xx_ADD("maincpu", TMS9900, 3000000, memmap, cru_map)
	MCFG_TMS99xx_EXTOP_HANDLER( WRITE8(ti99_4x_state, external_operation) )
	MCFG_TMS99xx_INTLEVEL_HANDLER( READ8(ti99_4x_state, interrupt_level) )
	MCFG_TMS99xx_CLKOUT_HANDLER( WRITELINE(ti99_4x_state, clock_out) )
	MCFG_TMS99xx_DBIN_HANDLER( WRITELINE(ti99_4x_state, dbin_line) )

	MCFG_MACHINE_START_OVERRIDE(ti99_4x_state, ti99_4a )

	/* video hardware */
	MCFG_DEVICE_ADD(VIDEO_SYSTEM_TAG, V9938VIDEO, 0)
	MCFG_V9938_ADD(VDP_TAG, SCREEN_TAG, 0x20000, XTAL_21_4772MHz)  /* typical 9938 clock, not verified */
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(ti99_4x_state, video_interrupt_in))
	MCFG_V99X8_SCREEN_ADD_NTSC(SCREEN_TAG, VDP_TAG, XTAL_21_4772MHz)

	/* Main board */
	MCFG_DEVICE_ADD(TMS9901_TAG, TMS9901, 3000000)
	MCFG_TMS9901_READBLOCK_HANDLER( READ8(ti99_4x_state, read_by_9901) )
	MCFG_TMS9901_P2_HANDLER( WRITELINE( ti99_4x_state, keyC0) )
	MCFG_TMS9901_P3_HANDLER( WRITELINE( ti99_4x_state, keyC1) )
	MCFG_TMS9901_P4_HANDLER( WRITELINE( ti99_4x_state, keyC2) )
	MCFG_TMS9901_P5_HANDLER( WRITELINE( ti99_4x_state, alphaW) )
	MCFG_TMS9901_P6_HANDLER( WRITELINE( ti99_4x_state, cs1_motor) )
	MCFG_TMS9901_P7_HANDLER( WRITELINE( ti99_4x_state, cs2_motor) )
	MCFG_TMS9901_P8_HANDLER( WRITELINE( ti99_4x_state, audio_gate) )
	MCFG_TMS9901_P9_HANDLER( WRITELINE( ti99_4x_state, cassette_output) )
	MCFG_TMS9901_INTLEVEL_HANDLER( WRITE8( ti99_4x_state, tms9901_interrupt) )

	MCFG_DMUX_ADD( DATAMUX_TAG, datamux_conf_ev )
	MCFG_DMUX_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_dmux) )
	MCFG_TI99_GROMPORT_ADD( GROMPORT_TAG )
	MCFG_GROMPORT_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_cart) )
	MCFG_GROMPORT_RESET_HANDLER( WRITELINE(ti99_4x_state, console_reset) )

	/* Software list */
	MCFG_SOFTWARE_LIST_ADD("cart_list_ti99", "ti99_cart")

	/* Peripheral expansion box */
	MCFG_DEVICE_ADD( PERIBOX_TAG, PERIBOX_EV, 0)
	MCFG_PERIBOX_INTA_HANDLER( WRITELINE(ti99_4x_state, extint) )
	MCFG_PERIBOX_INTB_HANDLER( WRITELINE(ti99_4x_state, notconnected) )
	MCFG_PERIBOX_READY_HANDLER( DEVWRITELINE(DATAMUX_TAG, ti99_datamux_device, ready_line) )

	/* sound hardware */
	MCFG_TI_SOUND_94624_ADD( TISOUND_TAG )
	MCFG_TI_SOUND_READY_HANDLER( WRITELINE(ti99_4x_state, console_ready_sound) )

	/* Cassette drives */
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_ADD( "cassette2" )

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	/* GROM devices */
	MCFG_GROM_ADD( GROM0_TAG, grom0_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM1_TAG, grom1_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))
	MCFG_GROM_ADD( GROM2_TAG, grom2_config )
	MCFG_GROM_READY_CALLBACK(WRITELINE(ti99_4x_state, console_ready_grom))

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG )

MACHINE_CONFIG_END

/*****************************************************************************
    ROM loading
    Note that we use the same ROMset for 50Hz and 60Hz versions.
    ROMs for peripheral equipment have been moved to the respective files.
******************************************************************************/
#define rom_ti99_4e rom_ti99_4
#define rom_ti99_4ae rom_ti99_4a
#define rom_ti99_4qe rom_ti99_4qi

ROM_START(ti99_4)
	// CPU memory space
	ROM_REGION16_BE(0x2000, "maincpu", 0)
	ROM_LOAD16_BYTE("u610.bin", 0x0000, 0x1000, CRC(6fcf4b15) SHA1(d085213c64701d429ae535f9a4ac8a50427a8343)) /* CPU ROMs high */
	ROM_LOAD16_BYTE("u611.bin", 0x0001, 0x1000, CRC(491c21d1) SHA1(7741ae9294c51a44a78033d1b77c01568a6bbfb9)) /* CPU ROMs low */

	// GROM memory space
	ROM_REGION(0x10000, region_grom, 0)
	ROM_LOAD("u500.bin", 0x0000, 0x1800, CRC(aa757e13) SHA1(4658d3d01c0131c283a30cebd12e76754d41a84a)) /* system GROM 0 */
	ROM_LOAD("u501.bin", 0x2000, 0x1800, CRC(c863e460) SHA1(6d849a76011273a069a98ed0c3feaf13831c942f)) /* system GROM 1 */
	ROM_LOAD("u502.bin", 0x4000, 0x1800, CRC(b0eda548) SHA1(725e3f26f8c819f356e4bb405b4102b5ae1e0e70)) /* system GROM 2 */
ROM_END

ROM_START(ti99_4a)
	// CPU memory space
	ROM_REGION16_BE(0x2000, "maincpu", 0)
	ROM_LOAD16_WORD("994arom.bin", 0x0000, 0x2000, CRC(db8f33e5) SHA1(6541705116598ab462ea9403c00656d6353ceb85)) /* system ROMs */

	// GROM memory space
	ROM_REGION(0x10000, region_grom, 0)
	ROM_LOAD("994agrom.bin", 0x0000, 0x6000, CRC(af5c2449) SHA1(0c5eaad0093ed89e9562a2c0ee6a370bdc9df439)) /* system GROMs */
ROM_END

ROM_START(ti99_4qi)
	// CPU memory space
	ROM_REGION16_BE(0x2000, "maincpu", 0)
	ROM_LOAD16_WORD("994qirom.bin", 0x0000, 0x2000, CRC(db8f33e5) SHA1(6541705116598ab462ea9403c00656d6353ceb85)) /* system ROMs */

	// GROM memory space
	ROM_REGION(0x10000, region_grom, 0)
	ROM_LOAD("994qigr0.bin", 0x0000, 0x1800, CRC(8b07772d) SHA1(95dcf5b7350ade65297eadd2d680c27561cc975c)) /* system GROM 0 */
	ROM_LOAD("994qigr1.bin", 0x2000, 0x1800, CRC(b8f367ab) SHA1(3ecead4b83ec525084c70b6123d4053f8a80e1f7)) /* system GROM 1 */
	ROM_LOAD("994qigr2.bin", 0x4000, 0x1800, CRC(e0bb5341) SHA1(e255f0d65d69b927cecb8fcfac7a4c17d585ea96)) /* system GROM 2 */
ROM_END


ROM_START(ti99_4ev)
	/*CPU memory space*/
	ROM_REGION16_BE(0x2000, "maincpu", 0)
	ROM_LOAD16_WORD("994arom.bin", 0x0000, 0x2000, CRC(db8f33e5) SHA1(6541705116598ab462ea9403c00656d6353ceb85)) /* system ROMs */

	/*GROM memory space*/
	ROM_REGION(0x10000, region_grom, 0)
	ROM_LOAD("994agr38.bin", 0x0000, 0x6000, CRC(bdd9f09b) SHA1(9b058a55d2528d2a6a69d7081aa296911ed7c0de)) /* system GROMs */
ROM_END

/*    YEAR  NAME      PARENT   COMPAT   MACHINE      INPUT    INIT      COMPANY             FULLNAME */
COMP( 1979, ti99_4,   0,       0,       ti99_4_60hz,  ti99_4, driver_device,   0,   "Texas Instruments", "TI-99/4 Home Computer (US)" , 0)
COMP( 1980, ti99_4e,  ti99_4,  0,       ti99_4_50hz,  ti99_4, driver_device,  0,    "Texas Instruments", "TI-99/4 Home Computer (Europe)" , 0)
COMP( 1981, ti99_4a,  0,       0,       ti99_4a_60hz, ti99_4a, driver_device, 0,    "Texas Instruments", "TI-99/4A Home Computer (US)" , 0)
COMP( 1981, ti99_4ae, ti99_4a, 0,       ti99_4a_50hz, ti99_4a, driver_device, 0,    "Texas Instruments", "TI-99/4A Home Computer (Europe)" , 0)
COMP( 1983, ti99_4qe, ti99_4qi, 0,       ti99_4qi_50hz, ti99_4a, driver_device, 0,    "Texas Instruments", "TI-99/4QI Home Computer (Europe)" , 0)
COMP( 1983, ti99_4qi, 0,        0,       ti99_4qi_60hz, ti99_4a, driver_device, 0,    "Texas Instruments", "TI-99/4QI Home Computer" , 0)
COMP( 1994, ti99_4ev, ti99_4a, 0,       ti99_4ev_60hz,ti99_4a, driver_device, 0, "Texas Instruments", "TI-99/4A Home Computer with EVPC" , 0)
