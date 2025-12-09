// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/8

    The TI-99/8 was the envisaged successor to the TI-99/4A but never passed
    its prototype state. Only a few dozen consoles were built. The ROMs
    were not even finalized, so the few available consoles have different
    operating system versions and capabilities.


    Characteristics
    ---------------

    Name: "Texas Instruments Computer TI-99/8" (no "Home")

    Unofficial nickname: "Armadillo"

    CPU: Single-CPU system using a TMS9995, but as a variant named MP9537. This
         variant does not offer on-chip RAM or decrementer.

    Video: TMS9118 Video Display Processor with 16 KiB RAM. The 9118 has the
         same capabilities as the 9918/28 in the TI-99/4A, except for the
         missing GROM clock (which must be provided separately) and the
         different DRAM type (2 chips TMS 4416 16K*4). Delivers a 60 Hz
         interrupt to the CPU via the PSI.

    Keyboard: 50-key keyboard, slightly different to the TI-99/4A, but also with
         modifiers Control, Function, Shift, Caps Lock. Connects to the TMS 9901
         PSI like in the TI-99/4A, but the pin assignment and key matrix
         are different:
         - P0-P3: column select
         - INT6*-INT11*: row inputs (INT6* is only used for joystick fire)

    Cassette: Identical to TI-99/4A, except that the CS2 unit is not implemented

    Sound: SN94624 as used in the TI-99/4A

    Speech: TMS5200C, a rare variant of the TMS52xx family. Compatible to the
         speech data for the separate speech synthesizer for the TI-99/4A.
         Speech ROMs CD2325A, CD2326A (total 128K*1)

    ROM: TMS4764 (8K*8), called "ROM0" in the specifications [1]
         TMS47256 (32K*8), called "ROM1" [1]
         TMS47128 (16K*8), "P-Code ROM" (only available in late prototypes)
         See below for contents

    GROMs: TI-specific ROM circuits with internal address counter and 6 KiB
         capacity (see grom.c)
         3 GROMs (system GROMs, access via port at logical address F830)
         8 GROMs (Pascal / Text-to-speech GROMs, port at logical address F840)
         8 GROMs (Pascal GROMs, port at logical address F850)
         3 GROMs (Pascal GROMs, access via port at logical address F860)
         (total of 132 KiB GROM)

    RAM: 1 TMS4016 (SRAM 2K*8)
         8 TMS4164 (DRAM 64K*1)

    PSI: (programmable system interface) TMS9901 with connections to
         keyboard, joystick port, cassette port, and external interrupt lines
         (video, peripheral devices)

    External connectors:
         - Joystick port (compatible to TI-99/4A joystick slot)
         - Cassette port
         - Cartridge port (compatible to TI-99/4A cartridge slot, but vertically
           orientated, so cartridges are plugged in from the top)
         - I/O port (not compatible to TI-99/4A I/O port, needs a special P-Box
           card called "Armadillo interface")
         - Hexbus port (new peripheral system, also seen with later TI designs)
         - Video port (composite)

    Custom chips: Five custom chips contain mapping and selection logic
         - "Vaquerro": Logical address space decoder
         - "Mofetta" : Physical address space decoder
         - "Amigo"   : Mapper
         - "Pollo"   : DRAM controller
         - "Oso"     : Hexbus interface

    Modes:
         - Compatibility mode (TI-99/4A mode): Memory-mapped devices are
           placed at the same location as found in the TI-99/4A, thereby
           providing good downward compatibility.
           The console starts up in compatibility mode.
         - Native mode (Armadillo mode): Devices are located at positions above
           0xF000 that allow for a contiguous usage of memory.

    Mapper
    ------
    The mapper uses 4K pages (unlike the Geneve mapper with 8K pages) which
    are defined by a 32 bit word. The address bits A0-A3 serve as the page
    index, whereas bits A4-A15 are the offset in the page.
    From the 32 bits, 24 bits define the physical address, so this allows for
    a maximum of 16 MiB of mapped-addressable memory.

    See more about the mapper in the file 998board.cpp


    Availability of ROMs and documentation
    --------------------------------------
    By written consent, TI granted free use of all software and documentation
    concerning the TI-99/8, including all specifications, ROMs, and source code
    of ROMs.


    Acknowledgements
    ----------------
    Special thanks go to Ciro Barile of the TI99 Italian User Club
    (www.ti99iuc.it): By his courtesy we have a consistent dump of ROMs for
    one of the most evolved versions of the TI-99/8 with

    - complete GROM set (with Pascal)
    - complete ROM set (with Hexbus DSR and TTS)
    - complete speech ROM set

    Also, by applying test programs on his real console, many unclear
    specifications were resolved.


    References
    ----------
    [1] Texas Instruments: Armadillo Product Specifications, July 1983
    [2] Source code (Assembler and GPL) of the TI-99/8 ROMs and GROMs
    [3] Schematics of the TI-99/8


    Implementation
    --------------
    Initial version by Raphael Nabet, 2003.

    February 2012: Rewritten as class [Michael Zapf]
    November 2013: Included new dumps [Michael Zapf]

===========================================================================
Known Issues (MZ, 2019-05-10)

  KEEP IN MIND THAT TEXAS INSTRUMENTS NEVER RELEASED THE TI-99/8 AND THAT
  THERE ARE ONLY A FEW PROTOTYPES OF THE TI-99/8 AVAILABLE. ALL SOFTWARE
  MUST BE ASSUMED TO HAVE REMAINED IN A PRELIMINARY STATE.

- TI-99/4A disk controllers cannot be used with the TI-99/8 in Extended Basic II.
  In the 99/8, the peripheral access block (PAB, set of data defining the
  access to the device, like floppy) may be located in CPU RAM, while the
  controllers of the 99/4A expect the PAB to be in video RAM only. Exbasic II
  sets up the PAB in CPU RAM, which leads to a crash. Other cartridges from
  the 99/4A certainly use video RAM, and so the disk controller works.
  Therefore, the Hexbus floppy drive HX5102 is recommended for use with the
  TI-99/8. You do not even need to attach the Peripheral Box.

  mame ti99_8 -hexbus hx5102 -flop1 somedisk.dsk

- Multiple cartridges are not shown in the startup screen; only one
  cartridge is presented. You have to manually select the cartridges with the
  dip switch.

- SAVE and OLD MINIMEM do not work properly in XB II. It seems as if the
  mapper shadows the NVRAM of the cartridge. You will lose the contents when
  you turn off the machine.

*****************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"

#include "sound/sn76496.h"
#include "machine/tms9901.h"
#include "machine/tmc0430.h"
#include "imagedev/cassette.h"

#include "bus/ti99/internal/998board.h"
#include "bus/ti99/gromport/gromport.h"
#include "bus/hexbus/hexbus.h"

#include "bus/ti99/joyport/joyport.h"
#include "bus/ti99/internal/ioport.h"

#include "softlist_dev.h"
#include "speaker.h"

// Debugging
#define LOG_WARN        (1U << 1)   // Warnings
#define LOG_CONFIG      (1U << 2)   // Configuration
#define LOG_READY       (1U << 3)
#define LOG_INTERRUPTS  (1U << 4)
#define LOG_CRU         (1U << 5)
#define LOG_CRUREAD     (1U << 6)
#define LOG_RESETLOAD   (1U << 7)

#define VERBOSE ( LOG_CONFIG | LOG_WARN | LOG_RESETLOAD )

#include "logmacro.h"


namespace {

/*
    READY bits.
*/
enum
{
	READY_GROM = 1,
	READY_MAPPER = 2,
	READY_PBOX = 4,
	READY_SOUND = 8,
	READY_CART = 16,
	READY_SPEECH = 32,
	READY_MAINBOARD = 64
};

class ti99_8_state : public driver_device
{
public:
	ti99_8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_tms9901(*this, TI998_TMS9901_TAG),
		m_gromport(*this, TI99_GROMPORT_TAG),
		m_ioport(*this, TI99_IOPORT_TAG),
		m_mainboard(*this, TI998_MAINBOARD_TAG),
		m_joyport(*this, TI_JOYPORT_TAG),
		m_cassette(*this, "cassette"),
		m_keyboard(*this, "COL%u", 0U)
	{
	}

	void ti99_8(machine_config &config);
	void ti99_8_60hz(machine_config &config);
	void ti99_8_50hz(machine_config &config);

	// Lifecycle
	void driver_start() override;
	void driver_reset() override;

private:
	// Processor connections with the main board
	uint8_t cruread(offs_t offset);
	void cruwrite(offs_t offset, uint8_t data);
	void external_operation(offs_t offset, uint8_t data);
	void clock_out(int state);

	// Connections from outside towards the CPU (callbacks)
	void console_ready(int state);
	void console_reset(int state);
	void cpu_hold(int state);
	void notconnected(int state);

	// GROM clock (coming from Vaquerro)
	void gromclk_in(int state);

	// Connections with the system interface chip 9901
	void extint(int state);
	void video_interrupt(int state);

	// Connections with the system interface TMS9901
	uint8_t psi_input(offs_t offset);
	void keyC0(int state);
	void keyC1(int state);
	void keyC2(int state);
	void keyC3(int state);
	void audio_gate(int state);
	void cassette_output(int state);
	void cassette_motor(int state);
	void tms9901_interrupt(offs_t offset, uint8_t data);

	void crumap(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;
	void memmap_setaddress(address_map &map) ATTR_COLD;

	// Keyboard support
	void    set_keyboard_column(int number, int data);
	int     m_keyboard_column = 0;

	// READY handling
	int m_ready_old = 0;

	// Latch for 9901 INT2, INT1 lines
	int  m_int1 = 0;
	int  m_int2 = 0;

	// Connected devices
	required_device<tms9995_device>     m_cpu;
	required_device<tms9901_device>     m_tms9901;
	required_device<bus::ti99::gromport::gromport_device> m_gromport;
	required_device<bus::ti99::internal::ioport_device>     m_ioport;
	required_device<bus::ti99::internal::mainboard8_device>  m_mainboard;
	required_device<bus::ti99::joyport::joyport_device> m_joyport;
	required_device<cassette_image_device> m_cassette;

	required_ioport_array<14> m_keyboard;
};

/*
    Memory map. We have a configurable mapper, so we need to delegate the
    job to the mapper completely.
*/
void ti99_8_state::memmap(address_map &map)
{
	map(0x0000, 0xffff).rw(TI998_MAINBOARD_TAG, FUNC(bus::ti99::internal::mainboard8_device::read), FUNC(bus::ti99::internal::mainboard8_device::write));
}

void ti99_8_state::memmap_setaddress(address_map &map)
{
	map(0x0000, 0xffff).w(TI998_MAINBOARD_TAG, FUNC(bus::ti99::internal::mainboard8_device::setaddress));
}

/*
    CRU map - see description above
    The TMS9901 is fully decoded according to the specification, so we only
    have 32 bits for it; the rest goes to the CRU bus
    (decoded by the "Vaquerro" chip, signal NNOICS*)
*/

void ti99_8_state::crumap(address_map &map)
{
	map(0x0000, 0x2fff).rw(FUNC(ti99_8_state::cruread), FUNC(ti99_8_state::cruwrite));
	map(0x0000, 0x003f).rw(m_tms9901, FUNC(tms9901_device::read), FUNC(tms9901_device::write));
}

/* ti99/8 : 54-key keyboard */
static INPUT_PORTS_START(ti99_8)

	/* 16 ports for keyboard and joystick */
	PORT_START("COL0")    /* col 0 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ALPHA LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL1")    /* col 1 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ! DEL") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL2")    /* col 2 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @ INS") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S (LEFT)") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X (DOWN)") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL3")    /* col 3 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E (UP)") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D (RIGHT)") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL4")    /* col 4 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL5")    /* col 5 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL6")    /* col 6 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL7")    /* col 7 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL8")    /* col 8 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL9")    /* col 9 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL10")    /* col 10 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL11")    /* col 11 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL12")    /* col 12 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("COL13")    /* col 13 */
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END


uint8_t ti99_8_state::cruread(offs_t offset)
{
	LOGMASKED(LOG_CRUREAD, "read access to CRU address %04x\n", offset);
	uint8_t value = 0;

	// Let the mapper, the gromport, and the p-box decide whether they want
	// to change the value at the CRU address
	m_mainboard->crureadz(offset<<1, &value);
	m_gromport->crureadz(offset<<1, &value);
	m_ioport->crureadz(offset<<1, &value);

	LOGMASKED(LOG_CRU, "CRU %04x -> %x\n", offset<<1, value);
	return value;
}

void ti99_8_state::cruwrite(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRU, "CRU %04x <- %x\n", offset<<1, data);
	m_mainboard->cruwrite(offset<<1, data);
	m_gromport->cruwrite(offset<<1, data);
	m_ioport->cruwrite(offset<<1, data);
}

/***************************************************************************
    TI99/8-specific tms9901 I/O handlers
    These methods are callbacks from the TMS9901 system interface. That is,
    they deliver the values queried via the TMS9901, and they represent
    console functions which are under control of the TMS9901 (like the
    keyboard column selection.)
***************************************************************************/

uint8_t ti99_8_state::psi_input(offs_t offset)
{
	switch (offset)
	{
	case tms9901_device::INT1:
		return (m_int1==CLEAR_LINE)? 1 : 0;
	case tms9901_device::INT2:
		return (m_int2==CLEAR_LINE)? 1 : 0;

	case tms9901_device::INT6:
		if (m_keyboard_column >= 14)
			return BIT(m_joyport->read_port(),0);
		[[fallthrough]];
	case tms9901_device::INT7_P15:
		if (m_keyboard_column >= 14)
			return BIT(m_joyport->read_port(),4);
		[[fallthrough]];
	case tms9901_device::INT8_P14:
		if (m_keyboard_column >= 14)
			return BIT(m_joyport->read_port(),1);
		[[fallthrough]];
	case tms9901_device::INT9_P13:
		if (m_keyboard_column >= 14)
			return BIT(m_joyport->read_port(),2);
		[[fallthrough]];
	case tms9901_device::INT10_P12:
		if (m_keyboard_column >= 14)
			return BIT(m_joyport->read_port(),3);

		// return for last 5 cases if column<14
		return BIT(m_keyboard[m_keyboard_column]->read(), offset-tms9901_device::INT6);

	case tms9901_device::INT11_P11:
		return (m_cassette->input() > 0);

	default:
		return 1;
	}
}

/*
    WRITE key column select (P2-P4), TI-99/8
*/
void ti99_8_state::set_keyboard_column(int number, int data)
{
	if (data != 0)
		m_keyboard_column |= 1 << number;
	else
		m_keyboard_column &= ~(1 << number);

	if (m_keyboard_column >= 14)
	{
		m_joyport->write_port(m_keyboard_column - 13);
	}
}

void ti99_8_state::keyC0(int state)
{
	set_keyboard_column(0, state);
}

void ti99_8_state::keyC1(int state)
{
	set_keyboard_column(1, state);
}

void ti99_8_state::keyC2(int state)
{
	set_keyboard_column(2, state);
}

void ti99_8_state::keyC3(int state)
{
	set_keyboard_column(3, state);
}

/*
    Control cassette tape unit motor (P6)
*/
void ti99_8_state::cassette_motor(int state)
{
	m_cassette->change_state(state==ASSERT_LINE? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

/*
    Audio gate (P8)
    Set to 1 before using tape: this enables the mixing of tape input sound
    with computer sound.
    We do not really need to emulate this as the tape recorder generates sound
    on its own.
*/
void ti99_8_state::audio_gate(int state)
{
}

/*
    Tape output (P9)
    I think polarity is correct, but don't take my word for it.
*/
void ti99_8_state::cassette_output(int state)
{
	m_cassette->output(state==ASSERT_LINE? +1 : -1);
}

void ti99_8_state::tms9901_interrupt(offs_t offset, uint8_t data)
{
	m_cpu->set_input_line(INT_9995_INT1, data);
}

/*****************************************************************************/

/*
    set the state of TMS9901's INT2 (called by the VDP)
*/
void ti99_8_state::video_interrupt(int state)
{
	LOGMASKED(LOG_INTERRUPTS, "VDP int 2 on tms9901, level=%02x\n", state);
	m_int2 = (line_state)state;
	m_tms9901->set_int_line(2, state);
}

/***********************************************************
    Links to external devices
***********************************************************/

/*
    Propagate READY signals to the CPU.
*/
void ti99_8_state::console_ready(int state)
{
	if (m_ready_old != state)
		LOGMASKED(LOG_READY, "READY = %d\n", state);
	m_ready_old = (line_state)state;
	m_cpu->ready_line(state);
}

/*
    Enqueue a RESET signal.
*/
void ti99_8_state::console_reset(int state)
{
	LOGMASKED(LOG_RESETLOAD, "Incoming RESET line = %d\n", state);
	if (machine().phase() != machine_phase::INIT)
	{
		// RESET the 9901
		m_tms9901->rst1_line(state);

		// Pull up the CRUS and PTGEN lines (9901 outputs have been deactivated, pull-up resistors on the board show effect)
		m_mainboard->crus_in(true); // assert
		m_mainboard->ptgen_in(true); // clear

		// Setting ready to false so that automatic wait states are enabled
		m_cpu->ready_line(CLEAR_LINE);
		m_cpu->reset_line(ASSERT_LINE);

		// Send RESET to the IOPort
		m_ioport->reset_in(state);
	}
}

/*
    The HOLD line leading to the CPU entering the HOLD state.
*/
void ti99_8_state::cpu_hold(int state)
{
	LOGMASKED(LOG_INTERRUPTS, "Incoming HOLD line = %d\n", state);
	m_cpu->hold_line(state);
}

void ti99_8_state::extint(int state)
{
	LOGMASKED(LOG_INTERRUPTS, "EXTINT level = %02x\n", state);
	m_int1 = (line_state)state;
	m_tms9901->set_int_line(1, state);
}

[[maybe_unused]] void ti99_8_state::notconnected(int state)
{
	LOGMASKED(LOG_INTERRUPTS, "Setting a not connected line ... ignored\n");
}

void ti99_8_state::external_operation(offs_t offset, uint8_t data)
{
	static char const *const extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	if (offset == IDLE_OP) return;
	else
		LOGMASKED(LOG_WARN, "External operation %s not implemented on TI-99/8 board\n", extop[offset]);
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
void ti99_8_state::clock_out(int state)
{
	m_tms9901->phi_line(state);
	m_mainboard->clock_in(state);
}

void ti99_8_state::driver_start()
{
	save_item(NAME(m_keyboard_column));
	save_item(NAME(m_ready_old));
	save_item(NAME(m_int1));
	save_item(NAME(m_int2));
}

void ti99_8_state::driver_reset()
{
	m_cpu->hold_line(CLEAR_LINE);

	// Pulling down the line on RESET configures the CPU to insert one wait
	// state on external memory accesses
	//  m_cpu->ready_line(ASSERT_LINE);

	// m_gromport->set_grom_base(0x9800, 0xfff1);

	m_keyboard_column = 0;

	// Clear INT1 and INT2 latch
	m_int1 = CLEAR_LINE;
	m_int2 = CLEAR_LINE;
	console_reset(ASSERT_LINE);
	console_reset(CLEAR_LINE);
}

void ti99_8_state::ti99_8(machine_config& config)
{
	using namespace bus::ti99::internal;
	// basic machine hardware */
	// TMS9995-MP9537 CPU @ 10.7 MHz
	// MP9537 mask: This variant of the TMS9995 does not contain on-chip RAM
	TMS9995_MP9537(config, m_cpu, XTAL(10'738'635));
	m_cpu->set_addrmap(AS_PROGRAM, &ti99_8_state::memmap);
	m_cpu->set_addrmap(AS_IO, &ti99_8_state::crumap);
	m_cpu->set_addrmap(tms9995_device::AS_SETADDRESS, &ti99_8_state::memmap_setaddress);
	m_cpu->extop_cb().set(FUNC(ti99_8_state::external_operation));
	m_cpu->clkout_cb().set(FUNC(ti99_8_state::clock_out));
	m_cpu->holda_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::holda_line));

	// 9901 configuration
	TMS9901(config, m_tms9901, 0);
	m_tms9901->read_cb().set(FUNC(ti99_8_state::psi_input));
	m_tms9901->p_out_cb(0).set(FUNC(ti99_8_state::keyC0));
	m_tms9901->p_out_cb(1).set(FUNC(ti99_8_state::keyC1));
	m_tms9901->p_out_cb(2).set(FUNC(ti99_8_state::keyC2));
	m_tms9901->p_out_cb(3).set(FUNC(ti99_8_state::keyC3));
	m_tms9901->p_out_cb(4).set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::crus_in));
	m_tms9901->p_out_cb(5).set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptgen_in));
	m_tms9901->p_out_cb(6).set(FUNC(ti99_8_state::cassette_motor));
	m_tms9901->p_out_cb(8).set(FUNC(ti99_8_state::audio_gate));
	m_tms9901->p_out_cb(9).set(FUNC(ti99_8_state::cassette_output));
	m_tms9901->intreq_cb().set(FUNC(ti99_8_state::tms9901_interrupt));

	// Mainboard with custom chips
	TI99_MAINBOARD8(config, m_mainboard, 0);
	m_mainboard->ready_cb().set(FUNC(ti99_8_state::console_ready));
	m_mainboard->reset_cb().set(FUNC(ti99_8_state::console_reset));
	m_mainboard->hold_cb().set(FUNC(ti99_8_state::cpu_hold));

	// Cartridge port
	TI99_GROMPORT(config, m_gromport, 0, ti99_gromport_options_998, "single").extend();
	m_gromport->ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::system_grom_ready));
	m_gromport->reset_cb().set(FUNC(ti99_8_state::console_reset));

	// RAM
	RAM(config, TI998_SRAM_TAG).set_default_size("2K").set_default_value(0);
	RAM(config, TI998_DRAM_TAG).set_default_size("64K").set_default_value(0);

	// Software list
	SOFTWARE_LIST(config, "cart_list_ti99").set_original("ti99_cart");

	// I/O port
	TI99_IOPORT(config, m_ioport, 0, ti99_ioport_options_plain, nullptr);
	m_ioport->extint_cb().set(FUNC(ti99_8_state::extint));
	m_ioport->ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::pbox_ready));

	// Hexbus
	HEXBUS(config, TI998_HEXBUS_TAG, 0, hexbus_options, nullptr);

	// Sound hardware
	SPEAKER(config, "sound_out").front_center();
	sn76496_device& soundgen(SN76496(config, TI998_SOUNDCHIP_TAG, 3579545));
	soundgen.ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::sound_ready));
	soundgen.add_route(ALL_OUTPUTS, "sound_out", 0.75);

	// Speech hardware
	// Note: SPEECHROM uses its tag for referencing the region
	SPEAKER(config, "speech_out").front_center();

	cd2501ecd_device& vsp(CD2501ECD(config, TI998_SPEECHSYN_TAG, 640000L));
	vsp.ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::speech_ready));
	vsp.add_route(ALL_OUTPUTS, "speech_out", 0.50);

	TMS6100(config, TI998_SPEECHROM_REG, 0);
	vsp.m0_cb().set(TI998_SPEECHROM_REG, FUNC(tms6100_device::m0_w));
	vsp.m1_cb().set(TI998_SPEECHROM_REG, FUNC(tms6100_device::m1_w));
	vsp.addr_cb().set(TI998_SPEECHROM_REG, FUNC(tms6100_device::add_w));
	vsp.data_cb().set(TI998_SPEECHROM_REG, FUNC(tms6100_device::data_line_r));
	vsp.romclk_cb().set(TI998_SPEECHROM_REG, FUNC(tms6100_device::clk_w));

	// Cassette drive
	SPEAKER(config, "cass_out").front_center();
	CASSETTE(config, "cassette", 0).add_route(ALL_OUTPUTS, "cass_out", 0.25);

	// GROM library
	using namespace bus::ti99::internal;
	TMC0430(config, TI998_SYSGROM0_TAG, TI998_SYSGROM_REG, 0x0000, 0).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::system_grom_ready));
	TMC0430(config, TI998_SYSGROM1_TAG, TI998_SYSGROM_REG, 0x2000, 1).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::system_grom_ready));
	TMC0430(config, TI998_SYSGROM2_TAG, TI998_SYSGROM_REG, 0x4000, 2).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::system_grom_ready));

	TMC0430(config, TI998_GLIB10_TAG, TI998_GROMLIB1_REG, 0x0000, 0).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB11_TAG, TI998_GROMLIB1_REG, 0x2000, 1).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB12_TAG, TI998_GROMLIB1_REG, 0x4000, 2).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB13_TAG, TI998_GROMLIB1_REG, 0x6000, 3).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB14_TAG, TI998_GROMLIB1_REG, 0x8000, 4).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB15_TAG, TI998_GROMLIB1_REG, 0xa000, 5).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB16_TAG, TI998_GROMLIB1_REG, 0xc000, 6).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));
	TMC0430(config, TI998_GLIB17_TAG, TI998_GROMLIB1_REG, 0xe000, 7).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::ptts_grom_ready));

	TMC0430(config, TI998_GLIB20_TAG, TI998_GROMLIB2_REG, 0x0000, 0).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB21_TAG, TI998_GROMLIB2_REG, 0x2000, 1).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB22_TAG, TI998_GROMLIB2_REG, 0x4000, 2).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB23_TAG, TI998_GROMLIB2_REG, 0x6000, 3).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB24_TAG, TI998_GROMLIB2_REG, 0x8000, 4).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB25_TAG, TI998_GROMLIB2_REG, 0xa000, 5).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB26_TAG, TI998_GROMLIB2_REG, 0xc000, 6).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));
	TMC0430(config, TI998_GLIB27_TAG, TI998_GROMLIB2_REG, 0xe000, 7).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p8_grom_ready));

	TMC0430(config, TI998_GLIB30_TAG, TI998_GROMLIB3_REG, 0x0000, 0).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p3_grom_ready));
	TMC0430(config, TI998_GLIB31_TAG, TI998_GROMLIB3_REG, 0x2000, 1).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p3_grom_ready));
	TMC0430(config, TI998_GLIB32_TAG, TI998_GROMLIB3_REG, 0x4000, 2).ready_cb().set(TI998_MAINBOARD_TAG, FUNC(mainboard8_device::p3_grom_ready));

	// Joystick port
	TI99_JOYPORT(config, m_joyport, 0, ti99_joyport_options_mouse, "twinjoy");
}

/*
    TI-99/8 US version (NTSC, 60 Hz)
*/
void ti99_8_state::ti99_8_60hz(machine_config &config)
{
	ti99_8(config);
	// Video hardware
	tms9118_device &video(TMS9118(config, TI998_VDP_TAG, XTAL(10'738'635)));
	video.set_vram_size(0x4000);
	video.int_callback().set(FUNC(ti99_8_state::video_interrupt));
	video.set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/*
    TI-99/8 European version (PAL, 50 Hz)
*/
void ti99_8_state::ti99_8_50hz(machine_config &config)
{
	ti99_8(config);
	// Video hardware
	tms9129_device &video(TMS9129(config, TI998_VDP_TAG, XTAL(10'738'635)));
	video.set_vram_size(0x4000);
	video.int_callback().set(FUNC(ti99_8_state::video_interrupt));
	video.set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/*
    All ROM dumps except the speech ROM have a CRC16 checksum as the final two
    bytes. ROM1 contains four 8K chunks of ROM contents with an own CRC at their
    ends. All GROMs, ROM0, and the four ROM1 parts were successfully
    validated.
*/
ROM_START(ti99_8)
	// Logical (CPU) memory space: ROM0
	ROM_REGION(0x2000, TI998_ROM0_REG, 0)
	ROM_LOAD("rom0.u4", 0x0000, 0x2000, CRC(901eb8d6) SHA1(13190c5e834baa9c0a70066b566cfcef438ed88a))

	// Physical memory space (mapped): ROM1
	ROM_REGION(0x8000, TI998_ROM1_REG, 0)
	ROM_LOAD("rom1.u25", 0x0000, 0x8000, CRC(b574461a) SHA1(42c6aed44802cfabdd26b565d6e5ddfcd689f11e))

	// Physical memory space (mapped): P-Code ROM
	// This circuit is only available in later versions of the console and seems
	// to be piggy-backed on ROM1.
	// To make things worse, the decoding logic of the custom chips do not show
	// the required select line for this ROM on the available schematics, so
	// they seem to be from the earlier version. The location in the address
	// space was determined by ROM disassembly.
	ROM_REGION(0x8000, TI998_PASCAL_REG, 0)
	ROM_LOAD("pascal.u25a", 0x0000, 0x4000, CRC(d7ed6dd6) SHA1(32212ce6426ceccbff73d342d4a3ef699c0ae1e4))

	// System GROMs. 3 chips @ f830
	// The schematics do not enumerate the circuits but only say
	// "circuits on board" (COB) so we name the GROMs as gM_N.bin where M is the
	// ID (0-7) and N is the access port in the logical address space.
	ROM_REGION(0x6000, TI998_SYSGROM_REG, 0)
	ROM_LOAD("g0_f830.bin", 0x0000, 0x1800, CRC(1026db60) SHA1(7327095bf4f390476e69d9fd8424e98ea1f2325a))
	ROM_LOAD("g1_f830.bin", 0x2000, 0x1800, CRC(93a43d65) SHA1(19be8a07d674bc7554c2bc9c7a5725d81e888e6e))
	ROM_LOAD("g2_f830.bin", 0x4000, 0x1800, CRC(06f2b901) SHA1(f65e0fcb2c63e230b4a9563c72f91259b94ce955))

	// TTS & Pascal library. 8 chips @ f840
	ROM_REGION(0x10000, TI998_GROMLIB1_REG, 0)
	ROM_LOAD("g0_f840.bin", 0x0000, 0x1800, CRC(44501071) SHA1(4b5ef7f1aa43a87e7ae4f02090944be5c39b1f26))
	ROM_LOAD("g1_f840.bin", 0x2000, 0x1800, CRC(5a271d9e) SHA1(bb95befa2ffba2cc17ac437386e069e8ff621248))
	ROM_LOAD("g2_f840.bin", 0x4000, 0x1800, CRC(d52502df) SHA1(17063e33ee8709d0df8030f38bb92c4322d55e1e))
	ROM_LOAD("g3_f840.bin", 0x6000, 0x1800, CRC(86c12396) SHA1(119b6df9211b5399245e017721fc51b88b60879f))
	ROM_LOAD("g4_f840.bin", 0x8000, 0x1800, CRC(f17a2ef8) SHA1(dcb044f71d7f8a165b41f39e35a368d8f2d63b67))
	ROM_LOAD("g5_f840.bin", 0xA000, 0x1800, CRC(7dc41301) SHA1(dff714da68de352db93fba309db8e5a8ae7cab1a))
	ROM_LOAD("g6_f840.bin", 0xC000, 0x1800, CRC(7e310a90) SHA1(e927d8b3f8b32aa4fb9f7d080d5262c566a77fc7))
	ROM_LOAD("g7_f840.bin", 0xE000, 0x1800, CRC(3a9d20df) SHA1(1e6f9f8ec7df4b997a7579be742d0a7d54bc8763))

	// Pascal library. 8 chips @ f850
	ROM_REGION(0x10000, TI998_GROMLIB2_REG, 0)
	ROM_LOAD("g0_f850.bin", 0x0000, 0x1800, CRC(2d948672) SHA1(cf15912d6dae5a450e0cfd796aa36ea5e521dc56))
	ROM_LOAD("g1_f850.bin", 0x2000, 0x1800, CRC(7d64a842) SHA1(d5884bb2af21c8027311478ee506beac6f46203d))
	ROM_LOAD("g2_f850.bin", 0x4000, 0x1800, CRC(e5ed8900) SHA1(03826882ce10fb5a6b3a9ccc85d3d1fe51979d0b))
	ROM_LOAD("g3_f850.bin", 0x6000, 0x1800, CRC(87aaf19e) SHA1(fdbe163773b8a30fa6b9508e679be6fa4f99bf7a))
	ROM_LOAD("g4_f850.bin", 0x8000, 0x1800, CRC(d3e789a5) SHA1(5ab06aa75ca694b1035ce5ac0bebacc928721388))
	ROM_LOAD("g5_f850.bin", 0xA000, 0x1800, CRC(49fd90bd) SHA1(44b2cef29c2d5304a0dcfedbdcdf9f21f2201bf9))
	ROM_LOAD("g6_f850.bin", 0xC000, 0x1800, CRC(31bac4ab) SHA1(e29049f0597d5de0bfd5c9c7bfea902abe858010))
	ROM_LOAD("g7_f850.bin", 0xE000, 0x1800, CRC(71534098) SHA1(75e87123efde885e27dd749e07cb189eb2cc45a8))

	// Pascal library. 3 chips @ f860
	ROM_REGION(0x6000, TI998_GROMLIB3_REG, 0)
	ROM_LOAD("g0_f860.bin", 0x0000, 0x1800, CRC(0ceef210) SHA1(b89957fbff094b758746391a69dea6907c66b950))
	ROM_LOAD("g1_f860.bin", 0x2000, 0x1800, CRC(fc87de25) SHA1(4695b7f979f59a01ec16c55e4587c3379482b658))
	ROM_LOAD("g2_f860.bin", 0x4000, 0x1800, CRC(e833e350) SHA1(6ffe501981a1112be1af596a489d96e287fc6be5))

	// Speech ROM
	ROM_REGION(0x8000, TI998_SPEECHROM_REG, 0)
	ROM_LOAD("cd2325a.vsm", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe))
	ROM_LOAD("cd2326a.vsm", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020))
ROM_END

#define rom_ti99_8e rom_ti99_8

} // Anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE      INPUT   CLASS         INIT        COMPANY              FULLNAME                     FLAGS
COMP( 1983, ti99_8,  0,      0,      ti99_8_60hz, ti99_8, ti99_8_state, empty_init, "Texas Instruments", "TI-99/8 Computer (US)",     MACHINE_SUPPORTS_SAVE )
COMP( 1983, ti99_8e, ti99_8, 0,      ti99_8_50hz, ti99_8, ti99_8_state, empty_init, "Texas Instruments", "TI-99/8 Computer (Europe)", MACHINE_SUPPORTS_SAVE )
