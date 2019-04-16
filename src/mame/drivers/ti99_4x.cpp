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

#include "machine/tms9901.h"
#include "imagedev/cassette.h"

#include "bus/ti99/ti99defs.h"
#include "bus/ti99/internal/datamux.h"
#include "bus/ti99/gromport/gromport.h"
#include "bus/ti99/internal/evpcconn.h"

#include "bus/ti99/joyport/joyport.h"
#include "bus/ti99/internal/ioport.h"
#include "machine/ram.h"

#include "softlist.h"
#include "speaker.h"

// Debugging
#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CONFIG      (1U<<2)   // Configuration
#define LOG_READY       (1U<<3)
#define LOG_INTERRUPTS  (1U<<4)
#define LOG_CRU         (1U<<5)
#define LOG_CRUREAD     (1U<<6)
#define LOG_RESETLOAD   (1U<<7)

#define VERBOSE ( LOG_CONFIG | LOG_WARN | LOG_RESETLOAD )

#include "logmacro.h"

/*
    The console.
*/
class ti99_4x_state : public driver_device
{
public:
	ti99_4x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_keyboard_column(0),
		m_check_alphalock(false),
		m_nready_combined(0),
		m_nready_prev(0),
		m_model(0),
		m_int1(0),
		m_int2(0),
		m_int12(0),
		m_cpu(*this, "maincpu"),
		m_tms9901(*this, TI_TMS9901_TAG),
		m_gromport(*this, TI99_GROMPORT_TAG),
		m_ioport(*this, TI99_IOPORT_TAG),
		m_joyport(*this, TI_JOYPORT_TAG),
		m_datamux(*this, TI99_DATAMUX_TAG),
		m_video(*this, TI_VDP_TAG),
		m_cassette1(*this, "cassette1"),
		m_cassette2(*this, "cassette2"),
		m_keyboard(*this, "COL%u", 0U),
		m_alpha(*this, "ALPHA"),
		m_alpha1(*this, "ALPHA1"),
		m_alphabug(*this, "ALPHABUG")
	{ }

	// Configurations
	void ti99_4_common(machine_config &config);
	void ti99_4(machine_config &config);
	void ti99_4_50hz(machine_config &config);
	void ti99_4ev_60hz(machine_config &config);
	void ti99_4qi(machine_config &config);
	void ti99_4qi_60hz(machine_config &config);
	void ti99_4a_50hz(machine_config &config);
	void ti99_4a_60hz(machine_config &config);
	void ti99_4a(machine_config &config);
	void ti99_4_60hz(machine_config &config);

	// Lifecycle
	void driver_start() override;
	void driver_reset() override;

	// Interrupt triggers
	DECLARE_INPUT_CHANGED_MEMBER( load_interrupt );

private:
	// Processor connections with the main board
	uint8_t cruread(offs_t offset);
	uint8_t interrupt_level();
	void cruwrite(offs_t offset, uint8_t data);
	void external_operation(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER( clock_out );

	// Connections from outside towards the CPU (callbacks)
	DECLARE_WRITE_LINE_MEMBER( console_ready_dmux );
	DECLARE_WRITE_LINE_MEMBER( console_ready_sound );
	DECLARE_WRITE_LINE_MEMBER( console_ready_pbox );
	DECLARE_WRITE_LINE_MEMBER( console_ready_cart );
	DECLARE_WRITE_LINE_MEMBER( console_ready_grom );
	DECLARE_WRITE_LINE_MEMBER( console_reset );
	DECLARE_WRITE_LINE_MEMBER( notconnected );

	// GROM clock
	DECLARE_WRITE_LINE_MEMBER( gromclk_in );

	// Connections with the system interface chip 9901
	DECLARE_WRITE_LINE_MEMBER( extint );
	DECLARE_WRITE_LINE_MEMBER( video_interrupt_in );
	DECLARE_WRITE_LINE_MEMBER( handset_interrupt_in );

	// Connections with the system interface TMS9901
	uint8_t read_by_9901(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(keyC0);
	DECLARE_WRITE_LINE_MEMBER(keyC1);
	DECLARE_WRITE_LINE_MEMBER(keyC2);
	DECLARE_WRITE_LINE_MEMBER(cs1_motor);
	DECLARE_WRITE_LINE_MEMBER(audio_gate);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);
	void tms9901_interrupt(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(handset_ack);
	DECLARE_WRITE_LINE_MEMBER(cs2_motor);
	DECLARE_WRITE_LINE_MEMBER(alphaW);

	// Used by EVPC
	DECLARE_WRITE_LINE_MEMBER( video_interrupt_evpc_in );
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void crumap(address_map &map);
	void memmap(address_map &map);
	void memmap_setaddress(address_map &map);

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
	int  m_int1;
	int  m_int2;
	int  m_int12;

	// Connected devices
	required_device<tms9900_device>     m_cpu;
	required_device<tms9901_device>     m_tms9901;
	required_device<bus::ti99::gromport::gromport_device>   m_gromport;
	required_device<bus::ti99::internal::ioport_device>     m_ioport;
	required_device<bus::ti99::joyport::joyport_device>     m_joyport;
	required_device<bus::ti99::internal::datamux_device>    m_datamux;
	optional_device<tms9928a_device>    m_video;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;

	optional_ioport_array<6> m_keyboard;
	optional_ioport m_alpha;
	optional_ioport m_alpha1;
	optional_ioport m_alphabug;

	// Timer for EVPC (provided by the TMS9929A, but EVPC replaces that VDP)
	emu_timer   *m_gromclk_timer;
};

/*
    Console models.
*/
enum
{
	MODEL_4,
	MODEL_4A,
	MODEL_4EV,
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
    Memory map. All of the work is done in the datamux (see datamux.c).
*/
void ti99_4x_state::memmap(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0xffff).rw(TI99_DATAMUX_TAG, FUNC(bus::ti99::internal::datamux_device::read), FUNC(bus::ti99::internal::datamux_device::write));
}

void ti99_4x_state::memmap_setaddress(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0xffff).w(TI99_DATAMUX_TAG, FUNC(bus::ti99::internal::datamux_device::setaddress));
}

/*
    CRU map
    TMS9900 CRU address space is 12 bits wide, attached to A3-A14, A0-A2 must
    be 000 (other values for external commands like RSET, LREX, CKON...),
    A15 is used as CRUOUT
    The TMS9901 is incompletely decoded
    ---0 00xx xxcc ccc0
    causing 16 mirrors (0000, 0040, 0080, 00c0, ... , 03c0)
*/
void ti99_4x_state::crumap(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(ti99_4x_state::cruread), FUNC(ti99_4x_state::cruwrite));
	map(0x0000, 0x003f).mirror(0x03c0).rw(m_tms9901, FUNC(tms9901_device::read), FUNC(tms9901_device::write));
}


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

	PORT_START( "LOADINT")
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

uint8_t ti99_4x_state::cruread(offs_t offset)
{
	LOGMASKED(LOG_CRUREAD, "read access to CRU address %04x\n", offset << 1);
	uint8_t value = 0;

	// Let the gromport (not in the QI version) and the p-box behind the I/O port
	// decide whether they want to change the value at the CRU address

	if (m_model != MODEL_4QI) m_gromport->crureadz(offset<<1, &value);
	m_ioport->crureadz(offset<<1, &value);

	return value;
}

void ti99_4x_state::cruwrite(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRU, "Write access to CRU address %04x\n", offset << 1);
	// The QI version does not propagate the CRU signals to the cartridge slot
	if (m_model != MODEL_4QI) m_gromport->cruwrite(offset<<1, data);
	m_ioport->cruwrite(offset<<1, data);
}

void ti99_4x_state::external_operation(offs_t offset, uint8_t data)
{
	static char const *const extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	// Some games (e.g. Slymoids) actually use IDLE for synchronization
	if (offset == IDLE_OP) return;
	else
		LOGMASKED(LOG_WARN, "External operation %s not implemented on TI-99 board\n", extop[offset]);
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


uint8_t ti99_4x_state::read_by_9901(offs_t offset)
{
	int answer=0;

	switch (offset & 0x03)
	{
	case tms9901_device::CB_INT7:
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
			if ((m_model!=MODEL_4) && (m_alphabug->read()!=0) ) answer |= (m_alpha->read() | m_alpha1->read());
		}
		else
		{
			answer = m_keyboard[m_keyboard_column]->read();
		}
		if (m_check_alphalock)  // never true for TI-99/4
		{
			answer &= ~(m_alpha->read() | m_alpha1->read());
		}
		answer = (answer << 3);
		if (m_int1 == CLEAR_LINE) answer |= 0x02;
		if (m_int2 == CLEAR_LINE) answer |= 0x04;

		break;

	case tms9901_device::INT8_INT15:
		// |1|1|1|INT12|0|K|K|K|
		if (m_keyboard_column >= (m_model==MODEL_4? 5:6)) answer = 0x07;
		else answer = ((m_keyboard[m_keyboard_column]->read())>>5) & 0x07;
		answer |= 0xe0;
		if (m_model != MODEL_4 || m_int12==CLEAR_LINE) answer |= 0x10;
		break;

	case tms9901_device::P0_P7:
		// Required for the handset (only on TI-99/4)
		if ((m_joyport->read_port() & 0x20)!=0) answer |= 2;
		break;

	case tms9901_device::P8_P15:
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
    TODO: Emulate a pop sound when turning on/off the audio gate; there are
    some few programs that generate a sound with this feature
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

void ti99_4x_state::tms9901_interrupt(offs_t offset, uint8_t data)
{
	// offset contains the interrupt level (0-15)
	// However, the TI board just ignores that level and hardwires it to 1
	// See below (interrupt_level)
	m_cpu->set_input_line(INT_9900_INTREQ, data);
}

uint8_t ti99_4x_state::interrupt_level()
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
	m_tms9901->phi_line(state);
	m_datamux->clock_in(state);
	m_ioport->clock_in(state);
}

/*
    GROMCLK from VDP, propagating to datamux
*/
WRITE_LINE_MEMBER( ti99_4x_state::gromclk_in )
{
	m_datamux->gromclk_in(state);
}

/*
    Used by the EVPC
*/
void ti99_4x_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// Pulse it
	if (m_datamux != nullptr)
	{
		gromclk_in(ASSERT_LINE);
		gromclk_in(CLEAR_LINE);
	}
}

/*****************************************************************************/

WRITE_LINE_MEMBER( ti99_4x_state::video_interrupt_evpc_in )
{
	LOGMASKED(LOG_INTERRUPTS, "VDP INT2 from EVPC on tms9901, level=%d\n", state);
	m_int2 = (line_state)state;
	m_tms9901->set_single_int(2, state);
}

/*
    set the state of TMS9901's INT2 (called by the tms9928 core)
*/
WRITE_LINE_MEMBER( ti99_4x_state::video_interrupt_in )
{
	LOGMASKED(LOG_INTERRUPTS, "VDP INT2 on tms9901, level=%d\n", state);

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
	LOGMASKED(LOG_INTERRUPTS, "joyport INT12 on tms9901, level=%d\n", state);
	m_int12 = (line_state)state;
	m_tms9901->set_single_int(12, state);
}

/*
    One of the common hardware mods was to add a switch to trigger a LOAD
    interrupt
*/
INPUT_CHANGED_MEMBER( ti99_4x_state::load_interrupt )
{
	LOGMASKED(LOG_RESETLOAD, "LOAD interrupt, level=%d\n", newval);
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

	if (m_nready_prev != m_nready_combined)
		LOGMASKED(LOG_READY, "READY bits = %04x\n", ~m_nready_combined);

	m_nready_prev = m_nready_combined;
	m_cpu->set_ready(m_nready_combined==0);
}

/*
    Connections to the READY line. This might look a bit ugly; we need an
    implementation of a "Wired AND" device.
*/
WRITE_LINE_MEMBER( ti99_4x_state::console_ready_grom )
{
	LOGMASKED(LOG_READY, "GROM ready = %d\n", state);
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
	if (machine().phase() != machine_phase::INIT)
	{
		LOGMASKED(LOG_RESETLOAD, "Console reset line = %d\n", state);
		m_cpu->set_input_line(INT_9900_RESET, state);
		m_video->reset_line(state);
	}
}

WRITE_LINE_MEMBER( ti99_4x_state::extint )
{
	LOGMASKED(LOG_INTERRUPTS, "EXTINT level = %02x\n", state);
	m_int1 = (line_state)state;
	m_tms9901->set_single_int(1, state);
}

WRITE_LINE_MEMBER( ti99_4x_state::notconnected )
{
	LOGMASKED(LOG_INTERRUPTS, "Setting a not connected line ... ignored\n");
}

/******************************************************************************
    Machine definitions
******************************************************************************/

void ti99_4x_state::driver_start()
{
	m_nready_combined = 0;
	// Removing the TMS9928a requires to add a replacement for the GROMCLK.
	// In the real hardware this is a circuit (REPL99x) that fits into the VDP socket
	if (m_model == MODEL_4EV)
		m_gromclk_timer = timer_alloc(0);

	save_item(NAME(m_keyboard_column));
	save_item(NAME(m_check_alphalock));
	save_item(NAME(m_nready_combined));
	save_item(NAME(m_nready_prev));
	save_item(NAME(m_model));
	save_item(NAME(m_int1));
	save_item(NAME(m_int2));
	save_item(NAME(m_int12));
}

void ti99_4x_state::driver_reset()
{
	m_cpu->set_ready(ASSERT_LINE);
	m_cpu->set_hold(CLEAR_LINE);
	m_int1 = CLEAR_LINE;
	m_int2 = CLEAR_LINE;
	m_int12 = CLEAR_LINE;
	if (m_model == MODEL_4EV)
		m_gromclk_timer->adjust(attotime::zero, 0, attotime::from_hz(XTAL(10'738'635)/24));
}

/**********************************************************
    Common configuration
**********************************************************/
void ti99_4x_state::ti99_4_common(machine_config& config)
{
	// CPU
	TMS9900(config, m_cpu, 3000000);
	m_cpu->set_addrmap(AS_PROGRAM, &ti99_4x_state::memmap);
	m_cpu->set_addrmap(AS_IO, &ti99_4x_state::crumap);
	m_cpu->set_addrmap(tms99xx_device::AS_SETADDRESS, &ti99_4x_state::memmap_setaddress);
	m_cpu->extop_cb().set(FUNC(ti99_4x_state::external_operation));
	m_cpu->intlevel_cb().set(FUNC(ti99_4x_state::interrupt_level));
	m_cpu->clkout_cb().set(FUNC(ti99_4x_state::clock_out));

	// Programmable system interface (driven by CLKOUT)
	TMS9901(config, m_tms9901, 0);
	m_tms9901->read_cb().set(FUNC(ti99_4x_state::read_by_9901));
	m_tms9901->p_out_cb(2).set(FUNC(ti99_4x_state::keyC0));
	m_tms9901->p_out_cb(3).set(FUNC(ti99_4x_state::keyC1));
	m_tms9901->p_out_cb(4).set(FUNC(ti99_4x_state::keyC2));
	m_tms9901->p_out_cb(6).set(FUNC(ti99_4x_state::cs1_motor));
	m_tms9901->p_out_cb(7).set(FUNC(ti99_4x_state::cs2_motor));
	m_tms9901->p_out_cb(8).set(FUNC(ti99_4x_state::audio_gate));
	m_tms9901->p_out_cb(9).set(FUNC(ti99_4x_state::cassette_output));
	m_tms9901->intlevel_cb().set(FUNC(ti99_4x_state::tms9901_interrupt));

	// Databus multiplexer
	TI99_DATAMUX(config, m_datamux, 0).ready_cb().set(FUNC(ti99_4x_state::console_ready_dmux));

	// Cartridge port (aka GROMport)
	TI99_GROMPORT(config, m_gromport, 0, ti99_gromport_options, "single");
	m_gromport->ready_cb().set(FUNC(ti99_4x_state::console_ready_cart));
	m_gromport->reset_cb().set(FUNC(ti99_4x_state::console_reset));

	// Scratch pad RAM 256 bytes
	RAM(config, TI99_PADRAM_TAG).set_default_size("256").set_default_value(0);

	// Optional RAM expansion
	RAM(config, TI99_EXPRAM_TAG).set_default_size("32K").set_default_value(0);

	// Software list
	SOFTWARE_LIST(config, "cart_list_ti99").set_type("ti99_cart", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	// Cassette drives. Second drive is record-only.
	SPEAKER(config, "cass_out").front_center();
	CASSETTE(config, "cassette1", 0).add_route(ALL_OUTPUTS, "cass_out", 0.25);
	CASSETTE(config, "cassette2", 0);

	// GROM devices
	TMC0430(config, TI99_GROM0_TAG, TI99_CONSOLEGROM, 0x0000, 0).ready_cb().set(FUNC(ti99_4x_state::console_ready_grom));
	TMC0430(config, TI99_GROM1_TAG, TI99_CONSOLEGROM, 0x2000, 1).ready_cb().set(FUNC(ti99_4x_state::console_ready_grom));
	TMC0430(config, TI99_GROM2_TAG, TI99_CONSOLEGROM, 0x4000, 2).ready_cb().set(FUNC(ti99_4x_state::console_ready_grom));
}

/**********************************************************************
    TI-99/4 - predecessor of the more popular TI-99/4A
***********************************************************************/

void ti99_4x_state::ti99_4(machine_config& config)
{
	// Common configuration
	ti99_4_common(config);
	m_model = MODEL_4;

	// Main board
	// Add handset interrupt to 9901
	m_tms9901->p_out_cb(0).set(FUNC(ti99_4x_state::handset_ack));

	// Input/output port: normal config
	TI99_IOPORT(config, m_ioport, 0, ti99_ioport_options_plain, nullptr);
	m_ioport->extint_cb().set(FUNC(ti99_4x_state::extint));
	m_ioport->ready_cb().set(TI99_DATAMUX_TAG, FUNC(bus::ti99::internal::datamux_device::ready_line));

	// Sound hardware (not in EVPC variant)
	SPEAKER(config, "sound_out").front_center();
	sn94624_device& soundgen(SN94624(config, TI_SOUNDCHIP_TAG, 3579545/8));
	soundgen.ready_cb().set(FUNC(ti99_4x_state::console_ready_sound));
	soundgen.add_route(ALL_OUTPUTS, "sound_out", 0.75);

	// Joystick port. We can connect a joyport mouse or a handset (99/4-specific).
	TI99_JOYPORT(config, m_joyport, 0, ti99_joyport_options_994, "twinjoy");
	m_joyport->int_cb().set(FUNC(ti99_4x_state::handset_interrupt_in));
}

/*
    US version: 60 Hz, NTSC
*/
void ti99_4x_state::ti99_4_60hz(machine_config &config)
{
	ti99_4(config);
	TMS9918(config, m_video, XTAL(10'738'635));
	m_video->set_vram_size(0x4000);
	m_video->int_callback().set(FUNC(ti99_4x_state::video_interrupt_in));
	m_video->gromclk_callback().set(FUNC(ti99_4x_state::gromclk_in));
	m_video->set_screen(TI_SCREEN_TAG);

	SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER);
}

/*
    European version: 50 Hz, PAL
*/
void ti99_4x_state::ti99_4_50hz(machine_config &config)
{
	ti99_4(config);
	TMS9929(config, m_video, XTAL(10'738'635));
	m_video->set_vram_size(0x4000);
	m_video->int_callback().set(FUNC(ti99_4x_state::video_interrupt_in));
	m_video->gromclk_callback().set(FUNC(ti99_4x_state::gromclk_in));
	m_video->set_screen(TI_SCREEN_TAG);

	SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER);
}

/**********************************************************************
    TI-99/4A - replaced the 99/4 and became the standard TI-99 console
***********************************************************************/

void ti99_4x_state::ti99_4a(machine_config& config)
{
	// Common configuration
	ti99_4_common(config);
	m_model = MODEL_4A;

	// Main board
	// Add Alphalock to 9901
	m_tms9901->p_out_cb(5).set(FUNC(ti99_4x_state::alphaW));

	// Input/output port: Normal config
	TI99_IOPORT(config, m_ioport, 0, ti99_ioport_options_plain, nullptr);
	m_ioport->extint_cb().set(FUNC(ti99_4x_state::extint));
	m_ioport->ready_cb().set(TI99_DATAMUX_TAG, FUNC(bus::ti99::internal::datamux_device::ready_line));

	// Sound hardware (not in EVPC variant)
	SPEAKER(config, "sound_out").front_center();
	sn94624_device& soundgen(SN94624(config, TI_SOUNDCHIP_TAG, 3579545/8));
	soundgen.ready_cb().set(FUNC(ti99_4x_state::console_ready_sound));
	soundgen.add_route(ALL_OUTPUTS, "sound_out", 0.75);

	// Joystick port
	TI99_JOYPORT(config, m_joyport, 0, ti99_joyport_options_mouse, "twinjoy");
}

/*
    US version: 60 Hz, NTSC
*/
void ti99_4x_state::ti99_4a_60hz(machine_config &config)
{
	ti99_4a(config);
	TMS9918A(config, m_video, XTAL(10'738'635));
	m_video->set_vram_size(0x4000);
	m_video->int_callback().set(FUNC(ti99_4x_state::video_interrupt_in));
	m_video->gromclk_callback().set(FUNC(ti99_4x_state::gromclk_in));
	m_video->set_screen(TI_SCREEN_TAG);

	SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER);
}

/*
    European version: 50 Hz, PAL
*/
void ti99_4x_state::ti99_4a_50hz(machine_config &config)
{
	ti99_4a(config);
	TMS9929A(config, m_video, XTAL(10'738'635));
	m_video->set_vram_size(0x4000);
	m_video->int_callback().set(FUNC(ti99_4x_state::video_interrupt_in));
	m_video->gromclk_callback().set(FUNC(ti99_4x_state::gromclk_in));
	m_video->set_screen(TI_SCREEN_TAG);

	SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER);
}

/************************************************************************
    TI-99/4QI - the final version of the TI-99/4A
    This was a last modification of the console. One purpose was to lower
    production costs by a redesigned board layout. The other was that TI
    removed the ROM search for cartridges so that only cartridges with GROMs
    could be started, effectively kicking out all third-party cartridges like
    those from Atarisoft.
*************************************************************************/

/*
    US version: 60 Hz, NTSC
    There were no European versions.
*/
void ti99_4x_state::ti99_4qi_60hz(machine_config &config)
{
	ti99_4a(config);
	m_model = MODEL_4QI;

	TMS9918A(config, m_video, XTAL(10'738'635));
	m_video->set_vram_size(0x4000);
	m_video->int_callback().set(FUNC(ti99_4x_state::video_interrupt_in));
	m_video->gromclk_callback().set(FUNC(ti99_4x_state::gromclk_in));
	m_video->set_screen(TI_SCREEN_TAG);

	SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER);
}

/************************************************************************
    TI-99/4A with 80-column support. Actually a separate expansion card (EVPC),
    replacing the console video processor.

    Note that the sound chip is also moved to this card, because the SGCPU,
    which is intended to use the EVPC, does not have an own sound chip.
*************************************************************************/

void ti99_4x_state::ti99_4ev_60hz(machine_config& config)
{
	// Common configuration
	ti99_4_common(config);
	m_model = MODEL_4EV;

	// Main board
	// Add Alphalock
	m_tms9901->p_out_cb(5).set(FUNC(ti99_4x_state::alphaW));

	// EVPC connector
	// This is needed for delivering the video interrupt from the
	// EVPC expansion card into the console, after the video processor has been removed
	TI99_EVPCCONN(config, TI99_EVPC_CONN_TAG, 0).vdpint_cb().set(FUNC(ti99_4x_state::video_interrupt_evpc_in));

	// Input/output port: Configure for EVPC
	TI99_IOPORT(config, m_ioport, 0, ti99_ioport_options_evpc, "peb");
	m_ioport->extint_cb().set(FUNC(ti99_4x_state::extint));
	m_ioport->ready_cb().set(TI99_DATAMUX_TAG, FUNC(bus::ti99::internal::datamux_device::ready_line));

	// Joystick port
	// No joyport mouse, since we have a bus mouse with the EVPC
	TI99_JOYPORT(config, m_joyport, 0, ti99_joyport_options_plain, "twinjoy");
}

/*****************************************************************************
    ROM loading
    Note that we use the same ROMset for 50Hz and 60Hz versions.
    ROMs for peripheral equipment have been moved to the respective files.
******************************************************************************/
#define rom_ti99_4e rom_ti99_4
#define rom_ti99_4ae rom_ti99_4a

ROM_START(ti99_4)
	// CPU memory space
	// ROM files do not have a CRC16 at their end
	ROM_REGION16_BE(0x2000, TI99_CONSOLEROM, 0)
	ROM_LOAD16_BYTE("994_rom_hb.u610", 0x0000, 0x1000, CRC(6fcf4b15) SHA1(d085213c64701d429ae535f9a4ac8a50427a8343)) /* CPU ROMs high */
	ROM_LOAD16_BYTE("994_rom_lb.u611", 0x0001, 0x1000, CRC(491c21d1) SHA1(7741ae9294c51a44a78033d1b77c01568a6bbfb9)) /* CPU ROMs low */

	// GROM memory space
	// GROM files do not have a CRC16 at their end
	ROM_REGION(0x6000, TI99_CONSOLEGROM, 0)
	ROM_LOAD("994_grom0.u500", 0x0000, 0x1800, CRC(aa757e13) SHA1(4658d3d01c0131c283a30cebd12e76754d41a84a)) /* system GROM 0 */
	ROM_LOAD("994_grom1.u501", 0x2000, 0x1800, CRC(c863e460) SHA1(6d849a76011273a069a98ed0c3feaf13831c942f)) /* system GROM 1 */
	ROM_LOAD("994_grom2.u502", 0x4000, 0x1800, CRC(b0eda548) SHA1(725e3f26f8c819f356e4bb405b4102b5ae1e0e70)) /* system GROM 2 */
ROM_END

ROM_START(ti99_4a)
	// CPU memory space
	// ROM files have valid CRC16 as last word
	ROM_REGION16_BE(0x2000, TI99_CONSOLEROM, 0)
	ROM_LOAD16_BYTE("994a_rom_hb.u610", 0x0000, 0x1000, CRC(ee859c5f) SHA1(a45245707c3dccea902b718554a882d214a82504)) /* CPU ROMs high */
	ROM_LOAD16_BYTE("994a_rom_lb.u611", 0x0001, 0x1000, CRC(37859301) SHA1(f4e774fd5913b387a763f1b8de5524c54b255434)) /* CPU ROMs low */

	// GROM memory space
	// GROM files have valid CRC16 as last word
	ROM_REGION(0x6000, TI99_CONSOLEGROM, 0)
	ROM_LOAD("994a_grom0.u500", 0x0000, 0x1800, CRC(2445a5e8) SHA1(ea15d8b0ac52112dc0d5f4ab9a79ac8ca1cc1bbc)) /* system GROM 0 */
	ROM_LOAD("994a_grom1.u501", 0x2000, 0x1800, CRC(b8f367ab) SHA1(3ecead4b83ec525084c70b6123d4053f8a80e1f7)) /* system GROM 1 */
	ROM_LOAD("994a_grom2.u502", 0x4000, 0x1800, CRC(e0bb5341) SHA1(e255f0d65d69b927cecb8fcfac7a4c17d585ea96)) /* system GROM 2 */
ROM_END

ROM_START(ti99_4qi)
	// CPU memory space
	// ROM files are the same as for TI-99/4A, but located in sockets u3 and u5
	ROM_REGION16_BE(0x2000, TI99_CONSOLEROM, 0)
	ROM_LOAD16_BYTE("994a_rom_hb.u610", 0x0000, 0x1000, CRC(ee859c5f) SHA1(a45245707c3dccea902b718554a882d214a82504)) /* CPU ROMs high */
	ROM_LOAD16_BYTE("994a_rom_lb.u611", 0x0001, 0x1000, CRC(37859301) SHA1(f4e774fd5913b387a763f1b8de5524c54b255434)) /* CPU ROMs low */

	// GROM memory space
	// GROM files have valid CRC16 as last word
	// GROM1 and GROM2 are the same as for TI-99/4A, located in u30 and u31
	ROM_REGION(0x6000, TI99_CONSOLEGROM, 0)
	ROM_LOAD("994qi_grom0.u29", 0x0000, 0x1800, CRC(8b07772d) SHA1(95dcf5b7350ade65297eadd2d680c27561cc975c)) /* system GROM 0 */
	ROM_LOAD("994a_grom1.u501", 0x2000, 0x1800, CRC(b8f367ab) SHA1(3ecead4b83ec525084c70b6123d4053f8a80e1f7)) /* system GROM 1 */
	ROM_LOAD("994a_grom2.u502", 0x4000, 0x1800, CRC(e0bb5341) SHA1(e255f0d65d69b927cecb8fcfac7a4c17d585ea96)) /* system GROM 2 */
ROM_END

ROM_START(ti99_4ev)
	// CPU memory space
	// ROM files have valid CRC16 as last word
	// ROM files are the same as for TI-99/4A
	ROM_REGION16_BE(0x2000, TI99_CONSOLEROM, 0)
	ROM_LOAD16_BYTE("994a_rom_hb.u610", 0x0000, 0x1000, CRC(ee859c5f) SHA1(a45245707c3dccea902b718554a882d214a82504)) /* CPU ROMs high */
	ROM_LOAD16_BYTE("994a_rom_lb.u611", 0x0001, 0x1000, CRC(37859301) SHA1(f4e774fd5913b387a763f1b8de5524c54b255434)) /* CPU ROMs low */

	// GROM memory space
	// GROM files have valid CRC16 as last word
	// GROM1 has been patched to support the EVPC, but the CRC16 was not updated, being invalid now
	ROM_REGION(0x6000, TI99_CONSOLEGROM, 0)
	ROM_LOAD("994a_grom0.u500", 0x0000, 0x1800, CRC(2445a5e8) SHA1(ea15d8b0ac52112dc0d5f4ab9a79ac8ca1cc1bbc)) /* system GROM 0 */
	ROM_LOAD("994ev_grom1.u501", 0x2000, 0x1800, CRC(6885326d) SHA1(1a98de5ee886dce705de5cce11034a7be31aceac)) /* system GROM 1 */
	ROM_LOAD("994a_grom2.u502", 0x4000, 0x1800, CRC(e0bb5341) SHA1(e255f0d65d69b927cecb8fcfac7a4c17d585ea96)) /* system GROM 2 */
ROM_END

//    YEAR  NAME      PARENT   COMPAT  MACHINE        INPUT    CLASS          INIT        COMPANY              FULLNAME                            FLAGS
COMP( 1979, ti99_4,   0,       0,      ti99_4_60hz,   ti99_4,  ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4 Home Computer (US)",       MACHINE_SUPPORTS_SAVE)
COMP( 1980, ti99_4e,  ti99_4,  0,      ti99_4_50hz,   ti99_4,  ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4 Home Computer (Europe)",   MACHINE_SUPPORTS_SAVE)
COMP( 1981, ti99_4a,  0,       0,      ti99_4a_60hz,  ti99_4a, ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4A Home Computer (US)",      MACHINE_SUPPORTS_SAVE)
COMP( 1981, ti99_4ae, ti99_4a, 0,      ti99_4a_50hz,  ti99_4a, ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4A Home Computer (Europe)",  MACHINE_SUPPORTS_SAVE)
COMP( 1983, ti99_4qi, ti99_4a, 0,      ti99_4qi_60hz, ti99_4a, ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4QI Home Computer (US)",     MACHINE_SUPPORTS_SAVE)
COMP( 1994, ti99_4ev, ti99_4a, 0,      ti99_4ev_60hz, ti99_4a, ti99_4x_state, empty_init, "Texas Instruments", "TI-99/4A Home Computer with EVPC", MACHINE_SUPPORTS_SAVE)
