// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    The MESS TI-99/8 emulation driver

    The TI-99/8 was the envisaged successor to the TI-99/4A but never passed
    its prototype state. Only a few dozens of consoles were built. The ROMs
    were not even finalized, so the few available consoles have different
    operating system versions and capabilities.


    Characteristics
    ---------------

    Name: "Texas Instruments Computer TI-99/8" (no "Home")

    Inofficial nickname: "Armadillo"

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
           providing a good downward compatibility.
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

    See more about the mapper in the file mapper8.c.


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
Known Issues (MZ, 2010-11-07)

  KEEP IN MIND THAT TEXAS INSTRUMENTS NEVER RELEASED THE TI-99/8 AND THAT
  THERE ARE ONLY A FEW PROTOTYPES OF THE TI-99/8 AVAILABLE. ALL SOFTWARE
  MUST BE ASSUMED TO HAVE REMAINED IN A PRELIMINARY STATE.

- Extended Basic II does not start when a floppy controller is present. This is
  a problem of the prototypical XB II which we cannot solve. It seems as if only
  hexbus devices are properly supported, but we currently do not have an
  emulation for those. Thus you can currently only use cassette to load and
  save programs. You MUST not plug in any floppy controller when you intend to
  start XB II. Other cartridges (like Editor/Assembler)
  seem to be unaffected by this problem and can make use of the floppy
  controllers.
    Technical detail: The designers of XB II seem to have decided to put PABs
    (Peripheral access block; contains pointers to buffers, the file name, and
    the access modes) into CPU RAM instead of the traditional storage in VDP
    RAM. The existing peripheral cards are hard-coded to interpret the given
    pointer to the PAB as pointing to a VDP RAM address. That is, as soon as
    the card is found, control is passed to the DSR (device service routine),
    the file name will not be found, and control returns with an error. It seems
    as if XB II does not properly handle this situation and may lock up
    (sometimes it starts up, but file access is still not possible).

    TODO: Emulate a Hexbus floppy.

- Multiple cartridges are not shown in the startup screen; only one
  cartridge is presented. You have to manually select the cartridges with the
  dip switch.

- SAVE and OLD MINIMEM do not work properly in XB II. It seems as if the
  mapper shadows the NVRAM of the cartridge. You will lose the contents when
  you turn off the machine.

*****************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"

#include "bus/ti99x/ti99defs.h"

#include "sound/sn76496.h"
#include "sound/wave.h"
#include "machine/tms9901.h"
#include "machine/tmc0430.h"
#include "imagedev/cassette.h"

#include "bus/ti99x/998board.h"
#include "bus/ti99x/videowrp.h"
#include "bus/ti99x/gromport.h"
#include "bus/ti99x/joyport.h"

#include "bus/ti99_peb/peribox.h"

#include "softlist.h"

// Debugging
#define TRACE_READY 0
#define TRACE_INTERRUPTS 0
#define TRACE_RESET 0
#define TRACE_CRU 0

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
		m_tms9901(*this, TMS9901_TAG),
		m_gromport(*this, GROMPORT_TAG),
		m_peribox(*this, PERIBOX_TAG),
		m_mainboard(*this, MAINBOARD8_TAG),
		m_joyport(*this, JOYPORT_TAG),
		m_cassette(*this, "cassette") { };

	// Machine management
	DECLARE_MACHINE_START(ti99_8);
	DECLARE_MACHINE_RESET(ti99_8);

	// Processor connections with the main board
	DECLARE_READ8_MEMBER( cruread );
	DECLARE_WRITE8_MEMBER( cruwrite );
	DECLARE_WRITE8_MEMBER( external_operation );
	DECLARE_WRITE_LINE_MEMBER( clock_out );
	DECLARE_WRITE_LINE_MEMBER( dbin_line );

	// Connections from outside towards the CPU (callbacks)
	DECLARE_WRITE_LINE_MEMBER( console_ready );
	DECLARE_WRITE_LINE_MEMBER( console_reset );
	DECLARE_WRITE_LINE_MEMBER( cpu_hold );
	DECLARE_WRITE_LINE_MEMBER( notconnected );

	// GROM clock (coming from Vaquerro)
	DECLARE_WRITE_LINE_MEMBER( gromclk_in );

	// Connections with the system interface chip 9901
	DECLARE_WRITE_LINE_MEMBER( extint );
	DECLARE_WRITE_LINE_MEMBER( video_interrupt );

	// Connections with the system interface TMS9901
	DECLARE_READ8_MEMBER(read_by_9901);
	DECLARE_WRITE_LINE_MEMBER(keyC0);
	DECLARE_WRITE_LINE_MEMBER(keyC1);
	DECLARE_WRITE_LINE_MEMBER(keyC2);
	DECLARE_WRITE_LINE_MEMBER(keyC3);
	DECLARE_WRITE_LINE_MEMBER(audio_gate);
	DECLARE_WRITE_LINE_MEMBER(cassette_output);
	DECLARE_WRITE_LINE_MEMBER(cassette_motor);
	DECLARE_WRITE8_MEMBER(tms9901_interrupt);

private:
	// Keyboard support
	void    set_keyboard_column(int number, int data);
	int     m_keyboard_column;

	// READY handling
	line_state m_ready_old;

	// Latch for 9901 INT2, INT1 lines
	line_state  m_int1;
	line_state  m_int2;

	// Connected devices
	required_device<tms9995_device>     m_cpu;
	required_device<tms9901_device>     m_tms9901;
	required_device<gromport_device>    m_gromport;
	required_device<peribox_device>     m_peribox;
	required_device<mainboard8_device>  m_mainboard;
	required_device<joyport_device>     m_joyport;
	required_device<cassette_image_device> m_cassette;
};

/*
    Memory map. We have a configurable mapper, so we need to delegate the
    job to the mapper completely.
*/
static ADDRESS_MAP_START(memmap, AS_PROGRAM, 8, ti99_8_state)
	AM_RANGE(0x0000, 0xffff) AM_DEVREADWRITE(MAINBOARD8_TAG, mainboard8_device, read, write) AM_DEVSETOFFSET(MAINBOARD8_TAG, mainboard8_device, setoffset)
ADDRESS_MAP_END

/*
    CRU map - see description above
    The TMS9901 is fully decoded according to the specification, so we only
    have 32 bits for it; the rest goes to the CRU bus
    (decoded by the "Vaquerro" chip, signal NNOICS*)
*/

static ADDRESS_MAP_START(crumap, AS_IO, 8, ti99_8_state)
	AM_RANGE(0x0000, 0x0003) AM_DEVREAD(TMS9901_TAG, tms9901_device, read)
	AM_RANGE(0x0000, 0x02ff) AM_READ(cruread)

	AM_RANGE(0x0000, 0x001f) AM_DEVWRITE(TMS9901_TAG, tms9901_device, write)
	AM_RANGE(0x0000, 0x17ff) AM_WRITE(cruwrite)
ADDRESS_MAP_END

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


READ8_MEMBER( ti99_8_state::cruread )
{
//  if (VERBOSE>6) logerror("read access to CRU address %04x\n", offset << 4);
	UINT8 value = 0;

	// Similar to the bus8z_devices, just let the mapper, the gromport, and the p-box
	// decide whether they want to change the value at the CRU address
	// Also, we translate the bit addresses to base addresses
	m_mainboard->crureadz(space, offset<<4, &value);
	m_gromport->crureadz(space, offset<<4, &value);
	m_peribox->crureadz(space, offset<<4, &value);

	if (TRACE_CRU) logerror("ti99_8: CRU %04x -> %02x\n", offset<<4, value);
	return value;
}

WRITE8_MEMBER( ti99_8_state::cruwrite )
{
	if (TRACE_CRU) logerror("ti99_8: CRU %04x <- %x\n", offset<<1, data);
	m_mainboard->cruwrite(space, offset<<1, data);
	m_gromport->cruwrite(space, offset<<1, data);
	m_peribox->cruwrite(space, offset<<1, data);
}

/***************************************************************************
    TI99/8-specific tms9901 I/O handlers
    These methods are callbacks from the TMS9901 system interface. That is,
    they deliver the values queried via the TMS9901, and they represent
    console functions which are under control of the TMS9901 (like the
    keyboard column selection.)
***************************************************************************/

static const char *const column[] = {
	"COL0", "COL1", "COL2", "COL3", "COL4", "COL5", "COL6", "COL7",
	"COL8", "COL9", "COL10", "COL11", "COL12", "COL13"
};

READ8_MEMBER( ti99_8_state::read_by_9901 )
{
	int answer=0;
	UINT8 joyst;
	switch (offset & 0x03)
	{
	case TMS9901_CB_INT7:
		// Read pins INT3*-INT7* of TI99's 9901.
		//
		// bit 1: INT1 status
		// bit 2: INT2 status
		// bits 3-4: unused?
		// bit 5: ???
		// bit 6-7: keyboard status bits 0 through 1

		// |K|K|-|-|-|I2|I1|C|
		if (m_keyboard_column >= 14)
		{
			// TI-99/8's wiring differs from the TI-99/4A
			joyst = m_joyport->read_port();
			answer = (joyst & 0x01) | ((joyst & 0x10)>>3);
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		answer = (answer << 6);
		if (m_int1 == CLEAR_LINE) answer |= 0x02;
		if (m_int2 == CLEAR_LINE) answer |= 0x04;

		break;

	case TMS9901_INT8_INT15:
		// Read pins INT8*-INT15* of TI99's 9901.
		//
		// bit 0-2: keyboard status bits 2 to 4
		// bit 3: tape input mirror
		// bit 4: unused
		// bit 5-7: weird, not emulated

		// |0|0|0|0|0|K|K|K|

		if (m_keyboard_column >= 14)
		{
			joyst = m_joyport->read_port();
			answer = joyst << 1;
		}
		else
		{
			answer = ioport(column[m_keyboard_column])->read();
		}
		answer = (answer >> 2) & 0x07;
		break;

	case TMS9901_P0_P7:
		// Read pins P0-P7 of TI99's 9901. None here.
		break;

	case TMS9901_P8_P15:
		// Read pins P8-P15 of TI99's 9901. (TI-99/8)
		//
		// bit 26: high
		// bit 27: tape input
		answer = 4;
		if (m_cassette->input() > 0)
			answer |= 8;
		break;
	}
	return answer;
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

WRITE_LINE_MEMBER( ti99_8_state::keyC0 )
{
	set_keyboard_column(0, state);
}

WRITE_LINE_MEMBER( ti99_8_state::keyC1 )
{
	set_keyboard_column(1, state);
}

WRITE_LINE_MEMBER( ti99_8_state::keyC2 )
{
	set_keyboard_column(2, state);
}

WRITE_LINE_MEMBER( ti99_8_state::keyC3 )
{
	set_keyboard_column(3, state);
}

/*
    Control cassette tape unit motor (P6)
*/
WRITE_LINE_MEMBER( ti99_8_state::cassette_motor )
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
WRITE_LINE_MEMBER( ti99_8_state::audio_gate )
{
}

/*
    Tape output (P9)
    I think polarity is correct, but don't take my word for it.
*/
WRITE_LINE_MEMBER( ti99_8_state::cassette_output )
{
	m_cassette->output(state==ASSERT_LINE? +1 : -1);
}

WRITE8_MEMBER( ti99_8_state::tms9901_interrupt )
{
	m_cpu->set_input_line(INT_9995_INT1, data);
}

/*****************************************************************************/

/*
    set the state of TMS9901's INT2 (called by the VDP)
*/
WRITE_LINE_MEMBER( ti99_8_state::video_interrupt )
{
	if (TRACE_INTERRUPTS) logerror("VDP int 2 on tms9901, level=%02x\n", state);
	m_int2 = (line_state)state;
	m_tms9901->set_single_int(2, state);
}

/***********************************************************
    Links to external devices
***********************************************************/

/*
    Propagate READY signals to the CPU.
*/
void ti99_8_state::console_ready(int state)
{
	if (TRACE_READY)
	{
		if (m_ready_old != state) logerror("READY = %d\n", state);
	}

	m_ready_old = (line_state)state;
	m_cpu->ready_line(state);
}

/*
    Enqueue a RESET signal.
*/
WRITE_LINE_MEMBER( ti99_8_state::console_reset )
{
	if (TRACE_RESET) logerror("Incoming RESET line = %d\n", state);
	if (machine().phase() != MACHINE_PHASE_INIT)
	{
		// RESET the 9901
		m_tms9901->rst1_line(state);

		// Pull up the CRUS and PTGEN lines (9901 outputs have been deactivated, pull-up resistors on the board show effect)
		m_mainboard->crus_in(TRUE); // assert
		m_mainboard->ptgen_in(TRUE); // clear

		// Setting ready to false so that automatic wait states are enabled
		m_cpu->ready_line(CLEAR_LINE);
		m_cpu->reset_line(ASSERT_LINE);
	}
}

/*
    The HOLD line leading to the CPU entering the HOLD state.
*/
WRITE_LINE_MEMBER( ti99_8_state::cpu_hold )
{
	if (TRACE_INTERRUPTS) logerror("Incoming HOLD line = %d\n", state);
	m_cpu->hold_line(state);
}

WRITE_LINE_MEMBER( ti99_8_state::extint )
{
	if (TRACE_INTERRUPTS) logerror("EXTINT level = %02x\n", state);
	m_int1 = (line_state)state;
	m_tms9901->set_single_int(1, state);
}

WRITE_LINE_MEMBER( ti99_8_state::notconnected )
{
	if (TRACE_INTERRUPTS) logerror("Setting a not connected line ... ignored\n");
}

WRITE8_MEMBER( ti99_8_state::external_operation )
{
	static const char* extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	if (offset == IDLE_OP) return;
	else
	{
		logerror("External operation %s not implemented on TI-99/8 board\n", extop[offset]);
	}
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
WRITE_LINE_MEMBER( ti99_8_state::clock_out )
{
	m_mainboard->clock_in(state);
}

/*
   Data bus in (DBIN) line from the CPU.
*/
WRITE_LINE_MEMBER( ti99_8_state::dbin_line )
{
	m_mainboard->dbin_in(state);
}

MACHINE_START_MEMBER(ti99_8_state,ti99_8)
{
	m_peribox->senila(CLEAR_LINE);
	m_peribox->senilb(CLEAR_LINE);
	// m_mainboard->set_gromport(m_gromport);

	// Need to configure the speech ROM for inverse bit order
	speechrom_device* mem = subdevice<speechrom_device>(SPEECHROM_REG);
	mem->set_reverse_bit_order(true);
}

MACHINE_RESET_MEMBER(ti99_8_state, ti99_8)
{
	m_cpu->hold_line(CLEAR_LINE);

	// Pulling down the line on RESET configures the CPU to insert one wait
	// state on external memory accesses
	m_cpu->ready_line(CLEAR_LINE);

	// m_gromport->set_grom_base(0x9800, 0xfff1);

	// Clear INT1 and INT2 latch
	m_int1 = CLEAR_LINE;
	m_int2 = CLEAR_LINE;
}

static MACHINE_CONFIG_START( ti99_8, ti99_8_state )
	// basic machine hardware */
	// TMS9995-MP9537 CPU @ 10.7 MHz
	// MP9537 mask: This variant of the TMS9995 does not contain on-chip RAM
	MCFG_TMS99xx_ADD("maincpu", TMS9995_MP9537, XTAL_10_738635MHz, memmap, crumap)
	MCFG_TMS9995_EXTOP_HANDLER( WRITE8(ti99_8_state, external_operation) )
	MCFG_TMS9995_CLKOUT_HANDLER( WRITELINE(ti99_8_state, clock_out) )
	MCFG_TMS9995_DBIN_HANDLER( WRITELINE(ti99_8_state, dbin_line) )
	MCFG_TMS9995_HOLDA_HANDLER( DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, holda_line) )

	MCFG_MACHINE_START_OVERRIDE(ti99_8_state, ti99_8 )
	MCFG_MACHINE_RESET_OVERRIDE(ti99_8_state, ti99_8 )

	// 9901 configuration
	MCFG_DEVICE_ADD(TMS9901_TAG, TMS9901, XTAL_10_738635MHz/4.0)
	MCFG_TMS9901_READBLOCK_HANDLER( READ8(ti99_8_state, read_by_9901) )
	MCFG_TMS9901_P0_HANDLER( WRITELINE( ti99_8_state, keyC0) )
	MCFG_TMS9901_P1_HANDLER( WRITELINE( ti99_8_state, keyC1) )
	MCFG_TMS9901_P2_HANDLER( WRITELINE( ti99_8_state, keyC2) )
	MCFG_TMS9901_P3_HANDLER( WRITELINE( ti99_8_state, keyC3) )
	MCFG_TMS9901_P4_HANDLER( DEVWRITELINE( MAINBOARD8_TAG, mainboard8_device, crus_in) )
	MCFG_TMS9901_P5_HANDLER( DEVWRITELINE( MAINBOARD8_TAG, mainboard8_device, ptgen_in) )
	MCFG_TMS9901_P6_HANDLER( WRITELINE( ti99_8_state, cassette_motor) )
	MCFG_TMS9901_P8_HANDLER( WRITELINE( ti99_8_state, audio_gate) )
	MCFG_TMS9901_P9_HANDLER( WRITELINE( ti99_8_state, cassette_output) )
	MCFG_TMS9901_INTLEVEL_HANDLER( WRITE8( ti99_8_state, tms9901_interrupt) )

	// Mainboard with custom chips
	MCFG_DEVICE_ADD(MAINBOARD8_TAG, MAINBOARD8, 0)
	MCFG_MAINBOARD8_READY_CALLBACK(WRITELINE(ti99_8_state, console_ready))
	MCFG_MAINBOARD8_RESET_CALLBACK(WRITELINE(ti99_8_state, console_reset))
	MCFG_MAINBOARD8_HOLD_CALLBACK(WRITELINE(ti99_8_state, cpu_hold))

	MCFG_GROMPORT8_ADD( GROMPORT_TAG )
	MCFG_GROMPORT_READY_HANDLER( DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, system_grom_ready) )
	MCFG_GROMPORT_RESET_HANDLER( WRITELINE(ti99_8_state, console_reset) )

	/* Software list */
	MCFG_SOFTWARE_LIST_ADD("cart_list_ti99", "ti99_cart")

	// Peripheral expansion box
	MCFG_DEVICE_ADD( PERIBOX_TAG, PERIBOX_998, 0)
	MCFG_PERIBOX_INTA_HANDLER( WRITELINE(ti99_8_state, extint) )
	MCFG_PERIBOX_INTB_HANDLER( WRITELINE(ti99_8_state, notconnected) )
	MCFG_PERIBOX_READY_HANDLER( DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, pbox_ready) )

	// Sound hardware
	MCFG_SPEAKER_STANDARD_MONO("sound_out")
	MCFG_SOUND_ADD(TISOUNDCHIP_TAG, SN76496, 3579545)   /* 3.579545 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "sound_out", 0.75)
	MCFG_SN76496_READY_HANDLER(DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, sound_ready))

	// Speech hardware
	// Note: SPEECHROM uses its tag for referencing the region
	MCFG_DEVICE_ADD(SPEECHROM_REG, SPEECHROM, 0)
	MCFG_SPEAKER_STANDARD_MONO("speech_out")
	MCFG_SOUND_ADD(SPEECHSYN_TAG, CD2501ECD, 640000L)
	MCFG_TMS52XX_READYQ_HANDLER(DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, speech_ready))
	MCFG_TMS52XX_SPEECHROM(SPEECHROM_REG)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech_out", 0.50)

	// Cassette drive
	MCFG_SPEAKER_STANDARD_MONO("cass_out")
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "cass_out", 0.25)

	// GROM library
	MCFG_GROM_ADD( SYSGROM0_TAG, 0, SYSGROM_REG, 0x0000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, system_grom_ready))
	MCFG_GROM_ADD( SYSGROM1_TAG, 1, SYSGROM_REG, 0x2000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, system_grom_ready))
	MCFG_GROM_ADD( SYSGROM2_TAG, 2, SYSGROM_REG, 0x4000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, system_grom_ready))

	MCFG_GROM_ADD( GLIB10_TAG, 0, GROMLIB1_REG, 0x0000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB11_TAG, 1, GROMLIB1_REG, 0x2000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB12_TAG, 2, GROMLIB1_REG, 0x4000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB13_TAG, 3, GROMLIB1_REG, 0x6000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB14_TAG, 4, GROMLIB1_REG, 0x8000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB15_TAG, 5, GROMLIB1_REG, 0xa000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB16_TAG, 6, GROMLIB1_REG, 0xc000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))
	MCFG_GROM_ADD( GLIB17_TAG, 7, GROMLIB1_REG, 0xe000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, ptts_grom_ready))

	MCFG_GROM_ADD( GLIB20_TAG, 0, GROMLIB2_REG, 0x0000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB21_TAG, 1, GROMLIB2_REG, 0x2000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB22_TAG, 2, GROMLIB2_REG, 0x4000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB23_TAG, 3, GROMLIB2_REG, 0x6000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB24_TAG, 4, GROMLIB2_REG, 0x8000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB25_TAG, 5, GROMLIB2_REG, 0xa000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB26_TAG, 6, GROMLIB2_REG, 0xc000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))
	MCFG_GROM_ADD( GLIB27_TAG, 7, GROMLIB2_REG, 0xe000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p8_grom_ready))

	MCFG_GROM_ADD( GLIB30_TAG, 0, GROMLIB3_REG, 0x0000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p3_grom_ready))
	MCFG_GROM_ADD( GLIB31_TAG, 1, GROMLIB3_REG, 0x2000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p3_grom_ready))
	MCFG_GROM_ADD( GLIB32_TAG, 2, GROMLIB3_REG, 0x4000, DEVWRITELINE(MAINBOARD8_TAG, mainboard8_device, p3_grom_ready))

	// Joystick port
	MCFG_TI_JOYPORT4A_ADD( JOYPORT_TAG )
MACHINE_CONFIG_END

/*
    TI-99/8 US version (NTSC, 60 Hz)
*/
static MACHINE_CONFIG_DERIVED( ti99_8_60hz, ti99_8 )
	// Video hardware
	MCFG_DEVICE_ADD( VDP_TAG, TMS9118, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(ti99_8_state, video_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( VDP_TAG, tms9928a_device, screen_update )
MACHINE_CONFIG_END

/*
    TI-99/8 European version (PAL, 50 Hz)
*/
static MACHINE_CONFIG_DERIVED( ti99_8_50hz, ti99_8 )
	// Video hardware
	MCFG_DEVICE_ADD( VDP_TAG, TMS9129, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(ti99_8_state,video_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( VDP_TAG, tms9928a_device, screen_update )
MACHINE_CONFIG_END

/*
    All ROM dumps except the speech ROM have a CRC16 checksum as the final two
    bytes. ROM1 contains four 8K chunks of ROM contents with an own CRC at their
    ends. All GROMs, ROM0, and the four ROM1 parts were successfully
    validated.
*/
ROM_START(ti99_8)
	// Logical (CPU) memory space: ROM0
	ROM_REGION(0x2000, ROM0_REG, 0)
	ROM_LOAD("u4_rom0.bin", 0x0000, 0x2000, CRC(901eb8d6) SHA1(13190c5e834baa9c0a70066b566cfcef438ed88a))

	// Physical memory space (mapped): ROM1
	ROM_REGION(0x8000, ROM1_REG, 0)
	ROM_LOAD("u25_rom1.bin", 0x0000, 0x8000, CRC(b574461a) SHA1(42c6aed44802cfabdd26b565d6e5ddfcd689f11e))

	// Physical memory space (mapped): P-Code ROM
	// This circuit is only available in later versions of the console and seems
	// to be picky-backed on ROM1.
	// To make things worse, the decoding logic of the custom chips do not show
	// the required select line for this ROM on the available schematics, so
	// they seem to be from the earlier version. The location in the address
	// space was determined by ROM disassembly.
	ROM_REGION(0x8000, PASCAL_REG, 0)
	ROM_LOAD("u25a_pas.bin", 0x0000, 0x4000, CRC(d7ed6dd6) SHA1(32212ce6426ceccbff73d342d4a3ef699c0ae1e4))

	// System GROMs. 3 chips @ f830
	// The schematics do not enumerate the circuits but only talk about
	// "circuits on board" (COB) so we name the GROMs as gM_N.bin where M is the
	// ID (0-7) and N is the access port in the logical address space.
	ROM_REGION(0x6000, SYSGROM_REG, 0)
	ROM_LOAD("g0_f830.bin", 0x0000, 0x1800, CRC(1026db60) SHA1(7327095bf4f390476e69d9fd8424e98ea1f2325a))
	ROM_LOAD("g1_f830.bin", 0x2000, 0x1800, CRC(93a43d65) SHA1(19be8a07d674bc7554c2bc9c7a5725d81e888e6e))
	ROM_LOAD("g2_f830.bin", 0x4000, 0x1800, CRC(06f2b901) SHA1(f65e0fcb2c63e230b4a9563c72f91259b94ce955))

	// TTS & Pascal library. 8 chips @ f840
	ROM_REGION(0x10000, GROMLIB1_REG, 0)
	ROM_LOAD("g0_f840.bin", 0x0000, 0x1800, CRC(44501071) SHA1(4b5ef7f1aa43a87e7ae4f02090944be5c39b1f26))
	ROM_LOAD("g1_f840.bin", 0x2000, 0x1800, CRC(5a271d9e) SHA1(bb95befa2ffba2cc17ac437386e069e8ff621248))
	ROM_LOAD("g2_f840.bin", 0x4000, 0x1800, CRC(d52502df) SHA1(17063e33ee8709d0df8030f38bb92c4322d55e1e))
	ROM_LOAD("g3_f840.bin", 0x6000, 0x1800, CRC(86c12396) SHA1(119b6df9211b5399245e017721fc51b88b60879f))
	ROM_LOAD("g4_f840.bin", 0x8000, 0x1800, CRC(f17a2ef8) SHA1(dcb044f71d7f8a165b41f39e35a368d8f2d63b67))
	ROM_LOAD("g5_f840.bin", 0xA000, 0x1800, CRC(7dc41301) SHA1(dff714da68de352db93fba309db8e5a8ae7cab1a))
	ROM_LOAD("g6_f840.bin", 0xC000, 0x1800, CRC(7e310a90) SHA1(e927d8b3f8b32aa4fb9f7d080d5262c566a77fc7))
	ROM_LOAD("g7_f840.bin", 0xE000, 0x1800, CRC(3a9d20df) SHA1(1e6f9f8ec7df4b997a7579be742d0a7d54bc8763))

	// Pascal library. 8 chips @ f850
	ROM_REGION(0x10000, GROMLIB2_REG, 0)
	ROM_LOAD("g0_f850.bin", 0x0000, 0x1800, CRC(2d948672) SHA1(cf15912d6dae5a450e0cfd796aa36ea5e521dc56))
	ROM_LOAD("g1_f850.bin", 0x2000, 0x1800, CRC(7d64a842) SHA1(d5884bb2af21c8027311478ee506beac6f46203d))
	ROM_LOAD("g2_f850.bin", 0x4000, 0x1800, CRC(e5ed8900) SHA1(03826882ce10fb5a6b3a9ccc85d3d1fe51979d0b))
	ROM_LOAD("g3_f850.bin", 0x6000, 0x1800, CRC(87aaf19e) SHA1(fdbe163773b8a30fa6b9508e679be6fa4f99bf7a))
	ROM_LOAD("g4_f850.bin", 0x8000, 0x1800, CRC(d3e789a5) SHA1(5ab06aa75ca694b1035ce5ac0bebacc928721388))
	ROM_LOAD("g5_f850.bin", 0xA000, 0x1800, CRC(49fd90bd) SHA1(44b2cef29c2d5304a0dcfedbdcdf9f21f2201bf9))
	ROM_LOAD("g6_f850.bin", 0xC000, 0x1800, CRC(31bac4ab) SHA1(e29049f0597d5de0bfd5c9c7bfea902abe858010))
	ROM_LOAD("g7_f850.bin", 0xE000, 0x1800, CRC(71534098) SHA1(75e87123efde885e27dd749e07cb189eb2cc45a8))

	// Pascal library. 3 chips @ f860
	ROM_REGION(0x6000, GROMLIB3_REG, 0)
	ROM_LOAD("g0_f860.bin", 0x0000, 0x1800, CRC(0ceef210) SHA1(b89957fbff094b758746391a69dea6907c66b950))
	ROM_LOAD("g1_f860.bin", 0x2000, 0x1800, CRC(fc87de25) SHA1(4695b7f979f59a01ec16c55e4587c3379482b658))
	ROM_LOAD("g2_f860.bin", 0x4000, 0x1800, CRC(e833e350) SHA1(6ffe501981a1112be1af596a489d96e287fc6be5))

	// Speech ROM
	ROM_REGION(0x8000, SPEECHROM_REG, 0)
	ROM_LOAD("cd2325a.vsm", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe))
	ROM_LOAD("cd2326a.vsm", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020))
ROM_END

#define rom_ti99_8e rom_ti99_8

/*      YEAR    NAME        PARENT  COMPAT  MACHINE     INPUT   INIT      COMPANY                 FULLNAME */
COMP(   1983,   ti99_8,     0,      0,  ti99_8_60hz,ti99_8, driver_device,  0,      "Texas Instruments",    "TI-99/8 Computer (US)" , 0)
COMP(   1983,   ti99_8e,    ti99_8, 0,  ti99_8_50hz,ti99_8, driver_device,  0,      "Texas Instruments",    "TI-99/8 Computer (Europe)" , 0 )
