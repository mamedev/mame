// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 *
 * History of Didact
 *------------------
 * Didact Laromedelsproduktion was started in Linkoping in Sweden by Anders Andersson, Arne Kullbjer and
 * Lars Bjorklund. They constructed a series of microcomputers for educational purposes such as "Mikrodator 6802",
 * Esselte 100 and the Candela computer for the swedish schools to educate the students in assembly programming
 * and BASIC for electro mechanical applications such as stepper motors, simple process control, buttons
 * and LED:s. Didact designs were marketed by Esselte Studium to the swedish schools. The Candela computer
 * was designed to be the big breakthough and developed by Candela Data AB, "a Didact Company". The Candela
 * system was based around a main unit that could run OS-9 or Flex and a terminal unit that had a propietary
 * software including CDBASIC. The Candela system lost the battle of the swedish schools to
 * the Compis computer by TeleNova which was based on CP/M initially.  Later both lost to IBM PC as we know.
 * Candela Data continued to sell their system to the swedish industry without major successes despite great
 * innovation and spririt.
 *
 * The Esselte 1000 was an educational package based on Apple II plus software and litterature
 * but the relation to Didact is at this point unknown so it is probably a pure Esselte software production.
 *
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 * http://frakaday.blogspot.com/p/candela.html
 * http://elektronikforumet.com/forum/viewtopic.php?f=11&t=51424
 * http://kilroy71.fastmail.fm/gallery/Miscellaneous/20120729_019.jpg
 * http://elektronikforumet.com/forum/download/file.php?id=63988&mode=view
 * http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=150#p1203915
 *
 *  TODO:
 *  Didact designs:    mp68a, md6802, Modulab, Esselte 100, can09t, can09
 * --------------------------------------------------------------------------
 *  - Add PCB layouts   OK     OK                OK          OK
 *  - Dump ROM:s,       OK     OK                rev2        OK      OK
 *  - Keyboard          OK     OK                rev2
 *  - Display/CRT       OK     OK                OK
 *  - Clickable Artwork RQ     RQ
 *  - Sound             NA     NA
 *  - Cassette i/f                               OK
 *  - Expansion bus
 *  - Expansion overlay
 *  - Interrupts        OK                       OK
 *  - Serial                   XX                XX          OK
 *   XX = needs debug
 ****************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h" // For mp68a, md6802 and e100
#include "cpu/m6809/m6809.h" // For candela
#include "machine/6821pia.h" // For all boards
#include "machine/6840ptm.h" // For candela
#include "machine/6850acia.h"// For candela
#include "machine/74145.h"   // For the md6802 and e100
#include "machine/clock.h"   // For candela
#include "machine/ram.h"     // For candela
#include "machine/wd_fdc.h"  // For candela
#include "video/dm9368.h"    // For the mp68a
#include "video/mc6845.h"    // For candela
// Features
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"

#include "screen.h"

// Generated artwork includes
#include "mp68a.lh"
#include "md6802.lh"

#define VERBOSE 0

#define LOGPRINT(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG(x) LOGPRINT(x)
#define LOGSCAN(x) {}
#define LOGSER(x) {}
#define LOGSCREEN(x) {}
#define RLOG(x) {}
#define LOGCS(x) {}

#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define PIA1_TAG "pia1"
#define PIA2_TAG "pia2"
#define PIA3_TAG "pia3"
#define PIA4_TAG "pia4"

/* Didact base class */
class didact_state : public driver_device
{
	public:
	didact_state(const machine_config &mconfig, device_type type, const char * tag)
		: driver_device(mconfig, type, tag)
		,m_io_line0(*this, "LINE0")
		,m_io_line1(*this, "LINE1")
		,m_io_line2(*this, "LINE2")
		,m_io_line3(*this, "LINE3")
		,m_io_line4(*this, "LINE4")
		,m_line0(0)
		,m_line1(0)
		,m_line2(0)
		,m_line3(0)
		,m_reset(0)
		,m_shift(0)
		,m_led(0)
		,m_rs232(*this, "rs232")
		{ }
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	uint8_t m_line0;
	uint8_t m_line1;
	uint8_t m_line2;
	uint8_t m_line3;
	uint8_t m_reset;
	uint8_t m_shift;
	uint8_t m_led;
	optional_device<rs232_port_device> m_rs232;
	TIMER_DEVICE_CALLBACK_MEMBER(scan_artwork);
};


/*  _____________________________________________________________________________________________   ___________________________________________________________________________
 * |The Didact Mikrodator 6802 CPU board by Lars Bjorklund 1983                            (  ) |  |The Didact Mikrodator 6802 TB16 board by Lars Bjorklund 1983               |
 * |                                                                                     +----= |  |             +-|||||||-+                                         ______    |
 * |                                                                                     |    = |  | CA2 Tx      |terminal |                                        |  ()  |   |
 * |                                                                                     |    = |  | PA7 Rx      +---------+               +----------+  C1nF,<=R18k|      |   |
 * |     Photo of CPU board mainly covered by TB16 Keypad/Display board                  +--- = |  | CA1 DTR               +-----------+   |          |   CB2->CB1  |  E   |   |
 * |                                                                                            |  |               PA4-PA6 |           | 1 | BCD      |    +----+   |  X   |   |
 * |                                                                                            |  |               ------->| 74LS145   |   | digit 5  |    |LS  |   |  P   |   |
 * |                                                                                            |  |                       +-----------+   |----------|    | 122|   |  A   |   |
 * |                                                                                     +-----=|  |                                   |   |          |    |    |   |  N   |   |
 * |                                                                          +-------+  |     =|  |------ +--------+                  | 2 | BCD      |    |    |   |  S   |   |
 * |                                                                          |       |  |     =|  | RES*  | SHIFT  |  LED( )          |   | digit 4  |    |    |   |  I   |   |
 * |                                                                          |       |  |     =|  |       |  '*'   |    CA2           v   |----------|    +----+   |  O   |   |
 * |                                                                          | 6821  |  |     =|  |   PA3 |PA7 PA2 | PA1      PA0         |          |        +----|  N   |   |
 * |                                                                          | PIA   |  |     =|  |----|--+-----|--+--|-----+--|---+    3 |          |    PB0-|LS  |      |   |
 * |                                                                          |       |  |     =|  |    v  |     v  |  v     |  v   |      | BCD      |     PB7| 244|  C   |   |
 * |                                                                          |       |  |     =|  | ADR   | RUN    | SST    | CON  | 1    | digit 3  |    --->|    |  O   |   |
 * |                                                                          |       |  |     =|  |  0    |  4     |  8     |  C   |      |----------|        |    |  N   |   |
 * |                                                                          |       |  |     =|  |-------+--------+--------+------+      |          |<-------|    |  N   |   |
 * |                                                                          |       |  |     =|  |       |        |        |      |    4 |          |        +----|  E   |   |
 * |                                                                          |       |  |     =|  | STA   | BPS    | USERV  |      | 2    | BCD      |             |  C   |   |
 * |                                                                          |       |  |     =|  |  1    |  5     |  9     |  D   |      | digit 2  |             |  T   |   |
 * |                                                                          |       |  |     =|  |-------+--------+--------+------+      |----------|             |  O   |   |
 * |                                                                          |       |  |     =|  |       |        |        |      |      |          |             |  R   |   |
 * |                                                                          |       |  |     =|  | EXF   | EXB    | MOV    | LOAD | 3  5 | BCD      |             |      |   |
 * |                                                                          |       |  |     =|  |  2    |  6     |  A     |  E   |      | digit 1  |             |      |   |
 * |                                                                          +-------+  |     =|  |-------+--------+--------+------+      |----------|             |      |   |
 * |                                                                                     |     =|  |       |        |        |      |      |          |             |      |   |
 * |                                                                                     +-----=|  | CLR   |  SP    | USERJ  | FLAG | 4  6 | BCD      |             |      |   |
 * |                                                                                            |  |  3    |  7     |  B     |  F   |      | digit 0  |             |  ()  |   |
 * |                                                                                            |  |-------+--------+--------+------+      +----------+             +------+   |
 * |                                                                                            |  |                                                                           |
 * |                                                                                            |  |                                                                           |
 * |____________________________________________________________________________________________|  |___________________________________________________________________________|
 */

/* Mikrodator 6802 driver class */
class md6802_state : public didact_state
{
	public:
	md6802_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_tb16_74145(*this, "tb16_74145")
		,m_segments(0)
		,m_pia1(*this, PIA1_TAG)
		,m_pia2(*this, PIA2_TAG)
		{ }
	required_device<m6802_cpu_device> m_maincpu;
	required_device<ttl74145_device> m_tb16_74145;
	uint8_t m_segments;
	DECLARE_READ8_MEMBER( pia2_kbA_r );
	DECLARE_WRITE8_MEMBER( pia2_kbA_w );
	DECLARE_READ8_MEMBER( pia2_kbB_r );
	DECLARE_WRITE8_MEMBER( pia2_kbB_w );
	DECLARE_WRITE_LINE_MEMBER( pia2_ca2_w);

	virtual void machine_reset() override;
	virtual void machine_start() override;
protected:
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
};

/* Keyboard */
READ8_MEMBER( md6802_state::pia2_kbA_r )
{
	uint8_t ls145;
	uint8_t pa = 0xff;

	// Read out the selected column
	ls145 = m_tb16_74145->read() & 0x0f;

	// read out the artwork, line04 is handled by the timer
	m_line0 = m_io_line0->read();
	m_line1 = m_io_line1->read();
	m_line2 = m_io_line2->read();
	m_line3 = m_io_line3->read();

	// Mask out those rows that has a button pressed
	pa &= ~(((~m_line0 & ls145 ) != 0) ? 1 : 0);
	pa &= ~(((~m_line1 & ls145 ) != 0) ? 2 : 0);
	pa &= ~(((~m_line2 & ls145 ) != 0) ? 4 : 0);
	pa &= ~(((~m_line3 & ls145 ) != 0) ? 8 : 0);

	if (m_shift)
	{
		pa &= 0x7f;   // Clear shift bit if button being pressed (PA7) to ground (internal pullup)
		LOG( ("SHIFT is pressed\n") );
	}

	// Serial IN - needs debug/verification
	pa &= (m_rs232->rxd_r() != 0 ? 0xff : 0x7f);

	return pa;
}

/* Pull the cathodes low enabling the correct digit and lit the segments held by port B */
WRITE8_MEMBER( md6802_state::pia2_kbA_w )
{
	uint8_t digit_nbr;

//  LOG(("--->%s(%02x)\n", FUNCNAME, data));

	digit_nbr = (data >> 4) & 0x07;
	m_tb16_74145->write( digit_nbr );
	if (digit_nbr < 6)
	{
		output().set_digit_value( digit_nbr, m_segments);
	}
}

/* PIA 2 Port B is all outputs to drive the display so it is very unlikelly that this function is called */
READ8_MEMBER( md6802_state::pia2_kbB_r )
{
	LOG( ("Warning, trying to read from Port B designated to drive the display, please check why\n") );
	logerror("Warning, trying to read from Port B designated to drive the display, please check why\n");
	return 0;
}

/* Port B is fully used ouputting the segment pattern to the display */
WRITE8_MEMBER( md6802_state::pia2_kbB_w )
{
//  LOG(("--->%s(%02x)\n", FUNCNAME, data));

	/* Store the segment pattern but do not lit up the digit here, done by pulling the correct cathode low on Port A */
	m_segments = BITSWAP8(data, 0, 4, 5, 3, 2, 1, 7, 6);
}

WRITE_LINE_MEMBER( md6802_state::pia2_ca2_w )
{
	LOG(("--->%s(%02x) LED is connected through resisitor to +5v so logical 0 will lit it\n", FUNCNAME, state));
	output().set_led_value(m_led, !state);

	// Serial Out - needs debug/verification
	m_rs232->write_txd(state);

	m_shift = !state;
}

void md6802_state::machine_start()
{
	LOG(("--->%s()\n", FUNCNAME));
	save_item(NAME(m_shift));
	save_item(NAME(m_led));
	save_item(NAME(m_reset));
}

void md6802_state::machine_reset()
{
	LOG(("--->%s()\n", FUNCNAME));
	m_led = 1;
	m_maincpu->reset();
}

// This address map is traced from schema
static ADDRESS_MAP_START( md6802_map, AS_PROGRAM, 8, md6802_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_MIRROR(0x1800)
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE(PIA1_TAG, pia6821_device, read, write) AM_MIRROR(0x1ffc)
	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE(PIA2_TAG, pia6821_device, read, write) AM_MIRROR(0x1ffc)
	AM_RANGE(0xe000, 0xe7ff) AM_ROM AM_MIRROR(0x1800) AM_REGION("maincpu", 0xe000)
ADDRESS_MAP_END

/*
 *  ___________________________________________________________________________________________________________           _____________________________________________________
 * | The Didact Mp68A CPU board, by Anders Andersson 1979                                                      |         |The Didact Mp68A keypad/display  PB6   +oooo+        |
 * |                  +------+ +-------+     +--+                                                              |         |  by Anders Andersson 1979  +-------+  |cass|        |
 * |                  | 7402 | | 74490 |     |  |      +-------+               +--+                            |         |                    +--+    | 9368  |  +----+    +--+|
 * |       +-------+  +------+ +-------+     |  |      |       |               |  |                            |         |+-------+    2x5082-|B |    +-------+            |  ||
 * |       |       |    2112   2112          |  |      | EXP   |               |  |                            |         || 74132 |       7433|CD| 145  PA0-PA3            |E ||
 * |       | ROM   |    +--+   +--+          +--+      | ANS   |               |P |                            |         |+-------+           |DI| +--+               132  |X ||
 * |       | 7641  |    |  |   |  |                    | ION   |               |I |                            |         |+------+------+     | S| |  |               +--+ |P ||
 * |       |       |    |A |   |B |       +-----+      | BUSES |               |A |                            |         ||      |SHIFT |     | P| |  | PA4-PA6       |  | |A ||
 * |       | 512x8 |    |  |   |  |       |     |      | (2 x) |               |  |                            |         || RES  |(led) |     +--+ |  |               |  | |N ||
 * |       |       |    +--+   +--+       |     |      | FOR   |               |A |                            |         ||      |  *   |          +--+               |  | |S ||
 * |       +-------+    RAMS 4x256x4      |     |      |       |               |  |                            |         |+------+------+------+------+               +--+ |I ||
 * |     ROMS 2x512x8   2112   2112       |     |      | KEY   |               |E |                            |         ||      |      |      |      |                    |O ||
 * |       +-------+    +--+   +--+       |CPU  |      | BOARD | +------+      |X |                            |         || ADR  | RUN  | SST  | REG  |                    |N ||
 * |       |       |    |  |   |  |       |6800 |      |       | |      |      |P |                            |         ||  0   |  4   |  8   |  C   |                    |  ||
 * |       | ROM   |    |A |   |B |       |     |      | AND   | |      |      |A |                            |         |+------+------+------+------+                    |C ||
 * |       | 7641  |    |  |   |  |       |     |      |       | |      |      |N |                            |         ||      |      |      |      |                    |O ||
 * |       |       |    +--+   +--+       |     |      | I/O   | | 6820 |      |S |                            |         || STA  | STO  | BPR  | BPS  |                    |N ||
 * |       | 512x8 |    512 bytes RAM     |     |      | BOARDS| | PIA  |      |I |                            |         ||  1   |  5   |  9   |  D   |                    |N ||
 * |       +-------+                      |     |      |       | |  #1  |      |O |                         +-----+      |+------+------+------+------+           +------+ |E ||
 * |     1024 bytes ROM                   |     |      |       | |      |      |N |                         |     |      ||      |      |      |      |           |      | |C ||
 * |                                      +-----+      |       | |      |      |  |                  PIA A  |    |       || EXF  | EXB  | MOV  | PRM  |           |      | |T ||
 * |        7402  7412                                 |       | |      |      |B |                EXPANSION|    |       ||  2   |  6   |  A   |  E   |           |      | |O ||
 * |        +--+  +--+                                 |       | |      |      |U |                CONNECTOR|    |       |+------+------+------+------+           | 6820 | |R ||
 * |        |  |  |  |                                 |       | |      |      |S |                         |   _|       ||      |      |      |      |           | PIA  | |  ||
 * |        |  |  |  |                                 |       | |      |      |  |                     J4  |  |         || CLR  | REL  | REC  | PLA  |           |  #2  | |  ||
 * |        |  |  |  |                                 |       | +------+      |  |                         |  |_        ||  3   |  7   |  B   |  F   |           |      | |  ||
 * |        +--+  +--+         +--------+              |       |               |  |                         |    |       |+------+------+------+------+           |      | |  ||
 * |                  +-+      | 96LS02 |              |       |               |  |                         |    |       | +-------+ +-------+  +------+          |      | |  ||
 * |       R * * * R  |T|      +--------+              |       |               |  |                         |    |       | | 74148 | | 74148 |  | 7400 |          |      | |  ||
 * |       O  X    A  |R|                              |       |               |  |                         |    |       | +-------+ +-------+  +------+          |      | +--+|
 * |       M * * * M  |M|  Oscillator circuits         +-------+               +--+                         |     |      |                PB3    PB0-PB2          |      |     |
 * |                  |_|                               J1   J2                 J3                          +-----+      |       +---------+                      +------+  J1 |
 * |____________________________________________________________________________________________________________|        |______ |  _|||_  |___________________________________|
 *
 */
/* Didact mp68a driver class */

// Just a statement that the real mp68a hardware was designed with 6820 and not 6821
// They are functional equivalents BUT has different electrical characteristics.
#define pia6820_device pia6821_device
#define PIA6820 PIA6821
class mp68a_state : public didact_state
{
	public:
	mp68a_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_digit0(*this, "digit0")
		,m_digit1(*this, "digit1")
		,m_digit2(*this, "digit2")
		,m_digit3(*this, "digit3")
		,m_digit4(*this, "digit4")
		,m_digit5(*this, "digit5")
		,m_pia1(*this, PIA1_TAG)
		,m_pia2(*this, PIA2_TAG)
		{ }

	required_device<m6800_cpu_device> m_maincpu;

	// The display segment driver device (there is actually just one, needs rewrite to be correct)
	required_device<dm9368_device> m_digit0;
	required_device<dm9368_device> m_digit1;
	required_device<dm9368_device> m_digit2;
	required_device<dm9368_device> m_digit3;
	required_device<dm9368_device> m_digit4;
	required_device<dm9368_device> m_digit5;

	DECLARE_READ8_MEMBER( pia2_kbA_r );
	DECLARE_WRITE8_MEMBER( pia2_kbA_w );
	DECLARE_READ8_MEMBER( pia2_kbB_r );
	DECLARE_WRITE8_MEMBER( pia2_kbB_w );
	DECLARE_READ_LINE_MEMBER( pia2_cb1_r );

	virtual void machine_reset() override;
	virtual void machine_start() override;
protected:
	required_device<pia6820_device> m_pia1;
	required_device<pia6820_device> m_pia2;
};

READ8_MEMBER( mp68a_state::pia2_kbA_r )
{
	LOG(("--->%s\n", FUNCNAME));

	return 0;
}

WRITE8_MEMBER( mp68a_state::pia2_kbA_w )
{
	uint8_t digit_nbr;

	/* Display memory is at $702 to $708 in AAAADD format (A=address digit, D=Data digit)
	   but we are using data read from the port. */
	digit_nbr = (data >> 4) & 0x07;

	/* There is actually only one 9368 and a 74145 to drive the cathode of the right digit low */
	/* This can be emulated by prentending there are one 9368 per digit, at least for now      */
	switch (digit_nbr)
	{
	case 0: m_digit0->a_w(data & 0x0f); break;
	case 1: m_digit1->a_w(data & 0x0f); break;
	case 2: m_digit2->a_w(data & 0x0f); break;
	case 3: m_digit3->a_w(data & 0x0f); break;
	case 4: m_digit4->a_w(data & 0x0f); break;
	case 5: m_digit5->a_w(data & 0x0f); break;
	case 7: break; // used as an 'unselect' by the ROM between digit accesses.
	default: logerror("Invalid digit index %d\n", digit_nbr);
	}
}

READ8_MEMBER( mp68a_state::pia2_kbB_r )
{
	uint8_t a012, line, pb;

	LOG(("--->%s %02x %02x %02x %02x %02x => ", FUNCNAME, m_line0, m_line1, m_line2, m_line3, m_shift));

	a012 = 0;
	if ((line = (m_line0 | m_line1)) != 0)
	{
		a012 = 8;
		while (a012 > 0 && !(line & (1 << --a012)));
		a012 += 8;
	}
	if ( a012 == 0 && (line = ((m_line2) | m_line3)) != 0)
	{
		a012 = 8;
		while (a012 > 0 && !(line & (1 << --a012)));
	}

	pb  = a012;       // A0-A2 -> PB0-PB3

	if (m_shift)
	{
		pb |= 0x80;   // Set shift bit (PB7)
		m_shift = 0;  // Reset flip flop
		output().set_led_value(m_led, m_shift);
		LOG( ("SHIFT is released\n") );
	}

	LOG(("%02x\n", pb));

	return pb;
}

WRITE8_MEMBER( mp68a_state::pia2_kbB_w )
{
	LOG(("--->%s(%02x)\n", FUNCNAME, data));
}

READ_LINE_MEMBER( mp68a_state::pia2_cb1_r )
{
	m_line0 = m_io_line0->read();
	m_line1 = m_io_line1->read();
	m_line2 = m_io_line2->read();
	m_line3 = m_io_line3->read();

#if VERBOSE
	if ((m_line0 | m_line1 | m_line2 | m_line3) != 0)
		LOG(("%s()-->%02x %02x %02x %02x\n", FUNCNAME, m_line0, m_line1, m_line2, m_line3));
#endif

	return (m_line0 | m_line1 | m_line2 | m_line3) != 0 ? 0 : 1;
}

void mp68a_state::machine_reset()
{
	LOG(("--->%s()\n", FUNCNAME));
	m_maincpu->reset();
}

void mp68a_state::machine_start()
{
	LOG(("--->%s()\n", FUNCNAME));

	/* register for state saving */
	save_item(NAME(m_shift));
	save_item(NAME(m_led));
	save_item(NAME(m_reset));
}

// This address map is traced from pcb
static ADDRESS_MAP_START( mp68a_map, AS_PROGRAM, 8, mp68a_state )
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_MIRROR(0xf000)
	AM_RANGE(0x0500, 0x0503) AM_DEVREADWRITE(PIA1_TAG, pia6820_device, read, write) AM_MIRROR(0xf0fc)
	AM_RANGE(0x0600, 0x0603) AM_DEVREADWRITE(PIA2_TAG, pia6820_device, read, write) AM_MIRROR(0xf0fc)
	AM_RANGE(0x0700, 0x07ff) AM_RAM AM_MIRROR(0xf000)
	AM_RANGE(0x0800, 0x0bff) AM_ROM AM_MIRROR(0xf400) AM_REGION("maincpu", 0x0800)
ADDRESS_MAP_END

/*  __________________________________________________________________________________________________________________________________________
 * | The Didact Esselte 100 CPU board rev1, 14/8 1980                                                                          in-PCB coil     +----
 * |   +--+     +--+     +--+     +--+        +--+     +--+                                                                 +--------+    |VHF
 * |   74       74       74       74          74       74                   7805CT              7805CT        trim 3,5-13pF |+-----+ |    |  TV
 * |    157      393       04       10          00       03                                                        2N2369 | || o-+ | |    +----
 * |   +--+     +--+     +--+     +--+        +--+     +--+                                                               | |+---+ | |       |
 * |1Kohm                                                                                                                 | +------+ |    +----
 * |trim                                                                                                                  +----------+    |CVS
 * | 8 +--+              +--+          +--+                                                              7805CP                           | MON
 * | 0 74                74            74                                                                                                 +----
 * | 0  132               157            93                                                                                                  |
 * | 8 +--+              +--+          +--+                                                                          J401                    |
 * | 1 +--+                                                                          +--+  +--+                                     LM339    |
 * | 4 74                +--+          +--+                                          74    74     +--+ +--+                   J402           |
 * |    165              74            74                                             122    00   74   74    4Mhz                            |
 * | J +--+               157           393                                          +--+  +--+    138  138  XTAL                         +----
 * | G                   +--+          +--+                                                       +--+ +--+    +----+  +----+  +----+     |TAPE
 * | +----+      +----+                                                                                               optional            |
 * |  CHAR       VIDEO                 +--+    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +====++  CPU     PIA2    PIA1      +----
 * |   ROM        RAM                  74      6116   6116   6116   6116                                    ||                               |
 * |  2716       MK4118                 245      alt    alt    alt    alt                                2x ||  6802    6821    6821      +----
 * | +----+      +----+                +--+    MK4118 MK4118 MK4118 MK4118  2716   2716   2716   2716   2716||                            |PRNT
 * |DIDACT ESS 100 CPU                         +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----++                            |
 * |___________________________________________________________________________________________________________+----+__+----+__+----+_____+----
 *
 * rev2 board had 4Kb more ROM memory, 2 x 2764 instead of the 6 x 2716 (note the rev1 piggy back on righ most 2716) with funny address decoding.
 * Once we get a rom dump for rev 1 the driver need to accomodate another keymap too so probably needs to be splitted somehow.
 *  __________________________________________________________________________________________________________________________________________
 * | The Didact Esselte 100 CPU board rev2, 15/4 1983                                                                     in-PCB coil     +----
 * |           +--+     +--+     +--+     +--+     +--+     +--+                                                            +--------+    |VHF
 * |           74       74       74       74       74       74              7805CT              7805CT        trim 3,5-13pF |+-----+ |    |  TV
 * |             93      393       10      393       00       03                                                   2N2369 | || o-+ | |    +----
 * |           +--+     +--+     +--+     +--+     +--+     +--+                                                          | |+---+ | |       |
 * |1Kohm                                                                                                                 | +------+ |    +----
 * |trim                                                                                                                  +----------+    |CVS
 * |   +--+    +--+     +--+    +--+   +--+   J                                                          7805CP                           | MON
 * |   74      74       74      74     74     2                                                                                           +----
 * |    165     132      157     157     04   0                                                                                              |
 * |   +--+    +--+     +--+    +--+   +--+   1       J 2 0 2                     +----+ +----+                      J401                    |
 * |                                                                                                                                         |
 * | +----+            +----+          +--+             +--+   +--+   +--+  +--+   U202   U201                                J402           |
 * |  CHAR             VIDEO           74               74     74     74    74                               4Mhz                            |
 * |   ROM              RAM             157              138    08     00    138   2764   2764               XTAL                         +----
 * |  2716             HM6116          +--+             +--+   +--+   +--+  +--+                               +----+  +----+  +----+     |TAPE
 * | +----+            +----+                J                                    +----+ +----+                       optional            |
 * |                                   +--+  2        +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+   CPU     PIA2    PIA1      +----
 * |                                   74    0        6116   6116   6116   6116   6116   6116   6116   6116                          +--+    |
 * |   8169 830415                      245  3                                     opt    opt    opt    opt     6802    6821    6821 LM   +----
 * |  ESSELTE 100                      +--+                                                                                           339 |PRNT
 * |  CPU 100                                         +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+                        +--+ |
 * |___________________________________________________________________________________________________________+----+__+----+__+----+_____+----
 *
 *   Both rev1 and rev2 has a matrix keyboard PCB with a 74LS145 connected to J402 (PIA2)
 */

/* Esselte 100 driver class */
class e100_state : public didact_state
{
public:
	e100_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_kbd_74145(*this, "kbd_74145")
		,m_videoram(*this, "videoram")
		,m_cassette(*this, "cassette")
		,m_pia1(*this, PIA1_TAG)
		,m_pia2(*this, PIA2_TAG)
		,m_io_line5(*this, "LINE5")
		,m_io_line6(*this, "LINE6")
		,m_io_line7(*this, "LINE7")
		,m_io_line8(*this, "LINE8")
		,m_io_line9(*this, "LINE9")
		,m_pia1_B(0)
		,m_50hz(0)
		{ }
	required_device<m6802_cpu_device> m_maincpu;
	required_device<ttl74145_device> m_kbd_74145;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<cassette_image_device> m_cassette;
	uint8_t *m_char_ptr;
	uint8_t *m_vram;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_reset() override { m_maincpu->reset(); LOG(("--->%s()\n", FUNCNAME)); };
	virtual void machine_start() override;
	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );
	DECLARE_READ8_MEMBER( pia1_kbA_r );
	DECLARE_WRITE8_MEMBER( pia1_kbA_w );
	DECLARE_READ8_MEMBER( pia1_kbB_r );
	DECLARE_WRITE8_MEMBER( pia1_kbB_w );
	DECLARE_READ_LINE_MEMBER( pia1_ca1_r );
	DECLARE_READ_LINE_MEMBER( pia1_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( pia1_ca2_w);
	DECLARE_WRITE_LINE_MEMBER( pia1_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_w);
protected:
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_ioport m_io_line9;
	uint8_t m_pia1_B;
	uint8_t m_50hz;
};

TIMER_DEVICE_CALLBACK_MEMBER(e100_state::rtc_w)
{
	m_pia2->ca1_w((m_50hz++ & 1));
}

void e100_state::machine_start()
{
	LOG(("%s()\n", FUNCNAME));
	m_char_ptr  = memregion("chargen")->base();
	m_vram      = (uint8_t *)m_videoram.target();

	/* register for state saving */
	save_pointer (NAME (m_char_ptr), sizeof(m_char_ptr));
	save_pointer (NAME (m_vram), sizeof(m_vram));
	save_item(NAME(m_50hz));
	save_item(NAME(m_pia1_B));
}

uint32_t e100_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int vramad;
	uint8_t *chardata;
	uint8_t charcode;

	LOGSCREEN(("%s()\n", FUNCNAME));
	vramad = 0;
	for (int row = 0; row < 32 * 8; row += 8)
	{
		for (int col = 0; col < 32 * 8; col += 8)
		{
			/* look up the character data */
			charcode = m_vram[vramad];
			if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n %c at X=%d Y=%d: ", charcode, col, row));
			chardata = &m_char_ptr[(charcode * 8)];
			/* plot the character */
			for (y = 0; y < 8; y++)
			{
				if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n  %02x: ", *chardata));
				for (x = 0; x < 8; x++)
				{
					if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN((" %02x: ", *chardata));
					bitmap.pix16(row + y, col + x) = (*chardata & (1 << x)) ? 1 : 0;
				}
				chardata++;
			}
			vramad++;
		}
		if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n"));
	}

	return 0;
}

/* PIA write - the Esselte 100 allows the PIA:s to be accessed simultaneously */
WRITE8_MEMBER( e100_state::pia_w )
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
	if ((offset & 0x08) == 0x08)
	{
		LOG(("- PIA1\n"));
		m_pia1->write(space, offset, data);
	}
	if ((offset & 0x10) == 0x10)
	{
		LOG(("- PIA2\n"));
		m_pia2->write(space, offset, data);
	}
	if (VERBOSE && (offset & 0x18) == 0x18)
	{
		LOGCS(("- Dual device write access!\n"));
	}
	if (VERBOSE && (offset & 0x18) == 0x00)
	{
		logerror("- Funny write at offset %02x!\n", offset);
	}
}

/* PIA read  - the Esselte 100 allows the PIA:s to be accessed simultaneously */
READ8_MEMBER( e100_state::pia_r )
{
	uint8_t data = 0;

	switch (offset & 0x18)
	{
	case 0x18: // read PIA1 and PIA2 at the same time, should really only happen for writes...
		{
			uint8_t data1 =  m_pia1->read(space, offset);
			uint8_t data2 =  m_pia2->read(space, offset);
			logerror("%s: Dual device read may have caused unpredictable results on real hardware\n", FUNCNAME);
			data = data1 & data2; // We assume that the stable behaviour is that data lines with a low level by either device succeeds
			LOGCS(("%s %s[%02x] %02x & %02x -> %02x Dual device read!!\n", PIA1_TAG "/" PIA2_TAG, FUNCNAME, offset, data1, data2, data));
		}
		break;
	case 0x08: // PIA1
		data = m_pia1->read(space, offset);
		LOGCS(("%s %s(%02x)\n", PIA1_TAG, FUNCNAME, data));
		break;
	case 0x10: // PIA2
		data = m_pia2->read(space, offset);
		LOGCS(("%s %s(%02x)\n", PIA2_TAG, FUNCNAME, data));
		break;
	default: // None of the devices are selected
		logerror("%s: Funny read at offset %02x\n", FUNCNAME, offset);
	}
	return data;
}

WRITE8_MEMBER( e100_state::pia1_kbA_w )
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

READ8_MEMBER( e100_state::pia1_kbA_r )
{
	int ls145;
	uint8_t pa = 0x00;

	// Read out the selected column
	ls145 = m_kbd_74145->read() & 0x3ff;

	// read out the artwork
	switch (ls145)
	{
	case 0: pa = 0x00; break;
	case 1 << 0: pa = (~m_io_line0->read()) & 0xff; break;
	case 1 << 1: pa = (~m_io_line1->read()) & 0xff; break;
	case 1 << 2: pa = (~m_io_line2->read()) & 0xff; break;
	case 1 << 3: pa = (~m_io_line3->read()) & 0xff; break;
	case 1 << 4: pa = (~m_io_line4->read()) & 0xff; break;
	case 1 << 5: pa = (~m_io_line5->read()) & 0xff; break;
	case 1 << 6: pa = (~m_io_line6->read()) & 0xff; break;
	case 1 << 7: pa = (~m_io_line7->read()) & 0xff; break;
	case 1 << 8: pa = (~m_io_line8->read()) & 0xff; break;
	case 1 << 9: pa = (~m_io_line9->read()) & 0xff; break;
	default: logerror("Keyboard is misconfigured, please report!: %04x", ls145); break;
	}
	if (VERBOSE && ls145 && pa) LOGSCAN(("%s  [%03x]%04x\n", FUNCNAME, ls145, pa));

	return ~pa;
}

/*
  PB0-PB3 is connected to U601 (74LS145) which select a column to scan
  PB4-PB5 together with CA1, CA2, CB1 and CB2 are used for the printer interface
  PB6-PB7 forms the cassette interface

  The serial bitbanging perform enreliable atm, can be poor original code or inexact CPU timing.
  Best results is achieved with 8 bit at 9600 baud as follows:

    mame e100 -window -rs232 null_modem -bitbngr socket.127.0.0.1:4321

  Start the favourite Telnet client towards the 4321 port and exit the startup screen of MAME.
  At the "Esselte 100 #" prompt change to 8 bit communication and start the terminal mode:

   POKE (69,1)
   TERM(9600)

  It is now possible to send characters from the Esselte screen to the Telnet terminal. When a
  carriage return has been sent to the terminal the Esselte 100 goes into receiving mode until
  it receives a carriage return from the terminal at which point it will start sending again.

  TODO:
  - Fix key mapping of the Ctl-PI exit sequence to get out of the TERM mode.
  - Fix timing issues for the PIA bit banging, could be related to that the CPU emulation is not
    cycle exact or the ROM code is buggy
*/
#define SERIAL_OUT 0x10
#define SERIAL_IN  0x20
#define CASS_OUT   0x40
#define CASS_IN    0x80
WRITE8_MEMBER( e100_state::pia1_kbB_w )
{
	uint8_t col;

	// Keyboard
	//  if (VERBOSE && data != m_pia1_B) LOGSCAN(("%s(%02x)\n", FUNCNAME, data));
	m_pia1_B = data;
	col = data & 0x0f;
	m_kbd_74145->write( col );

	// Cassette
	m_cassette->output(data & CASS_OUT ? 1.0 : -1.0);

	// Serial
	m_rs232->write_txd(data & SERIAL_OUT ? 0 : 1);
}

READ8_MEMBER( e100_state::pia1_kbB_r )
{
	m_pia1_B &= ~(CASS_IN|SERIAL_IN);

	m_pia1_B |= (m_cassette->input() > 0.03 ? CASS_IN : 0x00);

	m_pia1_B |= (m_rs232->rxd_r() != 0 ? SERIAL_IN : 0x00);

	return m_pia1_B;
}

READ_LINE_MEMBER(e100_state::pia1_ca1_r)
{
	// TODO: Make this a slot device for time meassurements
	return ASSERT_LINE;  // Default is handshake for serial port TODO: Fix RS 232 handshake as default
}

READ_LINE_MEMBER(e100_state::pia1_cb1_r)
{
	return m_rs232->rxd_r();
}

WRITE_LINE_MEMBER(e100_state::pia1_ca2_w)
{
	// TODO: Make this a slot device to trigger time meassurements
}

WRITE_LINE_MEMBER(e100_state::pia1_cb2_w)
{
	m_rs232->write_txd(!state);
}

// This map is derived from info in "TEMAL 100 - teknisk manual Esselte 100"
static ADDRESS_MAP_START( e100_map, AS_PROGRAM, 8, e100_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xc800, 0xc81f) AM_READWRITE(pia_r, pia_w) AM_MIRROR(0x07e0)
	AM_RANGE(0xd000, 0xffff) AM_ROM AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

/*
 *         The CAN09 v1.4 CPU board                                                  The CAN09 ROM and AD/DA board
 *       +-------------------------------------------------------------+  ___     +----------------------------------------------------------+
 *      +-+  +---+   +------+ +-----++-----+     +-----++-----+        |_|   |   +-+   +-----++-----++-----++-----++-----++-----+            |
 *      | |  |RST|   |      | | ROM ||     |+---+| RAM ||     | +--+   | |   |   | |J2 |PROM7||PROM6||PROM5||PROM4||PROM3||PROM2|  +---+     |
 *      | |  +---+   |      | |27256||62256||74 ||58256||62256| |CN|   | |   |   | |   |empty||empty||empty||empty||empty||empty|  |74 |     |
 *      | |      O   | 6809 | | MON ||empty||245||     ||empty| |J7|   | |   |   | |  o|     ||     ||     ||     ||     ||     |  |393|     |
 *      | |    J2O   |      | | 58B ||     ||   ||     ||     | |  |   | |   |   | |  o|     ||     ||     ||     ||     ||     |  |   |     |
 *      | |      O   |      | |     ||     ||   ||     ||     | |  |   | |   |   | |  o+-----++-----++-----++-----++-----++-----+  +---+     |
 *      | |      O   |      | +-----++-----++---++-----++-----+ +--+   | |EUR|   | |  o        ______                              +---+     |
 *      | |J1    O   |      |      +-------+         +-----+ +-----+   | |   |   | |  oJ1     | 74138|                             |74 |     |
 *      +-+      O   |      |      | 74138 |         |     | |     |   | |CN |   +-+  o       +------+                             |393|     |
 *      +-+      O +-+------+      +-------+  +-----+|     | |     |   | |J4 |  J4|8  o   +-----------------+  +-----------------+ |   |     |
 *      | |J6    O |74165 |        +--++-----+|     || PIA | | PIA |   | |   |    |8  o   | PIA 6821        |  | PIA 6821        | +---+     |
 *      | |      O +------+   +---+|CN||     || PTM || 6821| | 6821|   | |   |   +-+  o   |                 |  |                 |           |
 *      | |      O  VR1 VR2   |82S||J8||     || 6840||     | |     |   | |   |   | |  o   +-----------------+  +-----------------+           |
 *      | |      O +------+   |123||  ||ACIA ||     ||     | |     |   | |   |   | |  o   +-------++-------+   +--------+   +--------+       |
 *      +-+      O |7405  |   |   ||  || 6850||     ||     | |     |   | |   |   | |  o   |MPD1603|| 14051 |   | TL501  |   | 74LS02 |       |
 *      _===       +------+   ++--++--+|     ||     ||     | |     |   | |   |   | |      +-------++-------+   +--------+   +--------+       |
 * comp _|=        |74132 |    | 7400 |+----+++-----++-----+ +-----+   |_|   |   | |J3       +-------+       +---------+  +----------+       |
 * video ===       +------+    +------+     |MAX232 |                  | |___|   +-+         |MC34004|       |AD7528   |  |ADC0820   |       |
 *       +----------------------------------+-------+------------------+          +----------+-------+-------+---------+--+----------+-------+
 *
 */
/*
 * Candela Terminal
 * TODO:
 * - Map additional PIA:s on the ROM board
 * - ROM/RAM paging by using the PIA:s, now static address map but CDBASIC RLOAD doesn't work for instance
 * - Vram and screen
 * - Keyboard
 * - LCD
 * - AD/DA support for related BASIC functions
 * - Promett rom cartridge support
 */
/* Candela terminal driver class */
class can09t_state : public didact_state
{
public:
	can09t_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_pia1(*this, PIA1_TAG)
		,m_pia2(*this, PIA2_TAG)
		,m_pia3(*this, PIA3_TAG)
		,m_pia4(*this, PIA4_TAG)
		,m_ptm(*this, "ptm")
		,m_acia(*this, "acia")
#if 0
		,m_io_line5(*this, "LINE5")
		,m_io_line6(*this, "LINE6")
		,m_io_line7(*this, "LINE7")
		,m_io_line8(*this, "LINE8")
		,m_io_line9(*this, "LINE9")
#endif
		{ }
	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override { m_maincpu->reset(); LOG(("--->%s()\n", FUNCNAME)); };
	virtual void machine_start() override { LOG(("%s()\n", FUNCNAME)); };
	DECLARE_READ8_MEMBER( pia1_A_r );
	DECLARE_READ8_MEMBER( pia1_B_r );
	DECLARE_WRITE8_MEMBER( pia1_B_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_cb2_w);
	DECLARE_WRITE_LINE_MEMBER( pia2_cb2_w);
	DECLARE_WRITE_LINE_MEMBER (write_acia_clock);
protected:
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;
	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;
#if 0
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_ioport m_io_line9;
#endif
};

READ8_MEMBER( can09t_state::pia1_A_r )
{
	LOG(("%s()\n", FUNCNAME));
	return 0;
}

READ8_MEMBER( can09t_state::pia1_B_r )
{
	LOG(("%s()\n", FUNCNAME));
	return 0;
}

WRITE8_MEMBER( can09t_state::pia1_B_w )
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

WRITE_LINE_MEMBER(can09t_state::pia1_cb2_w)
{
	LOG(("%s(%02x)\n", FUNCNAME, state));
}

WRITE_LINE_MEMBER(can09t_state::pia2_cb2_w)
{
	LOG(("%s(%02x)\n", FUNCNAME, state));
}

WRITE_LINE_MEMBER (can09t_state::write_acia_clock){
		m_acia->write_txc (state);
		m_acia->write_rxc (state);
}

// traced and guessed from pcb images and debugger
// It is very likelly that this is a PIA based dynamic address map, needs more analysis
static ADDRESS_MAP_START( can09t_map, AS_PROGRAM, 8, can09t_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x8000, 0xafff) AM_RAM
	AM_RANGE(0xb100, 0xb100) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0xb101, 0xb101) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0xb110, 0xb113) AM_DEVREADWRITE(PIA1_TAG, pia6821_device, read_alt, write_alt)
	AM_RANGE(0xb120, 0xb123) AM_DEVREADWRITE(PIA2_TAG, pia6821_device, read_alt, write_alt)
	AM_RANGE(0xb130, 0xb137) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xb200, 0xc1ff) AM_ROM AM_REGION("roms", 0x3200)
	AM_RANGE(0xc200, 0xdfff) AM_RAM /* Needed for BASIC etc */
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("roms", 0x6000)
ADDRESS_MAP_END

static INPUT_PORTS_START( can09t )
	PORT_START("LINE0")
	PORT_START("LINE1")
	PORT_START("LINE2")
	PORT_START("LINE3")
	PORT_START("LINE4")
INPUT_PORTS_END

/*
 * Candela Main Unit
 * TODO:
 * - Map PIA:S
 * - ROM/RAM paging by using the PIA:s and the myriad of 74138:s on the board
 * - Vram and screen for the 6845 CRTC
 * - Keyboard
 * - Serial port
 * - Floppy controller
 * - Split out commonalities in a candela_state base class for can09 and can09t
 */
/* Candela main unit driver class */
class can09_state : public didact_state
{
public:
	can09_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_pia1(*this, PIA1_TAG)
		,m_ram(*this, RAM_TAG)
		,m_bank1(*this, "bank1")
		,m_crtc(*this, "crtc")
	{ }
	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	DECLARE_READ8_MEMBER( pia1_A_r );
	DECLARE_WRITE8_MEMBER( pia1_A_w );
	DECLARE_READ8_MEMBER( pia1_B_r );
	DECLARE_WRITE8_MEMBER( pia1_B_w );
	DECLARE_WRITE_LINE_MEMBER( pia1_cb2_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
protected:
	required_device<pia6821_device> m_pia1;
	required_device<ram_device> m_ram;
	required_memory_bank m_bank1;
	required_device<h46505_device> m_crtc;
};

void can09_state::machine_reset()
{
	LOG(("%s()\n", FUNCNAME));
	m_bank1->set_entry(0);
}

void can09_state::machine_start()
{
	LOG(("%s()\n", FUNCNAME));
	m_bank1->configure_entries(0, 8, m_ram->pointer(), 0x8000);
}

uint32_t can09_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
#if 0
	int vramad;
	UINT8 *chardata;
	UINT8 charcode;
#endif

	LOGSCREEN(("%s()\n", FUNCNAME));
	//  vramad = 0;
	for (int row = 0; row < 72 * 8; row += 8)
	{
		for (int col = 0; col < 64 * 8; col += 8)
		{
#if 0
			/* look up the character data */
			charcode = m_vram[vramad];
			if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n %c at X=%d Y=%d: ", charcode, col, row));
			chardata = &m_char_ptr[(charcode * 8)];
#endif
			/* plot the character */
			for (y = 0; y < 8; y++)
			{
				//              if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n  %02x: ", *chardata));
				for (x = 0; x < 8; x++)
				{
					//                  if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN((" %02x: ", *chardata));
					bitmap.pix16(row + y, col + x) = x & 1; //(*chardata & (1 << x)) ? 1 : 0;
				}
				//              chardata++;
			}
			//          vramad++;
		}
		//      if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n"));
	}

	return 0;
}

READ8_MEMBER( can09_state::pia1_A_r )
{
	LOG(("%s()\n", FUNCNAME));
	return 0x40;
}

WRITE8_MEMBER( can09_state::pia1_A_w )
{
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

READ8_MEMBER( can09_state::pia1_B_r )
{
	LOG(("%s()\n", FUNCNAME));
	return 0;
}

WRITE8_MEMBER( can09_state::pia1_B_w )
{
	//  UINT8 *RAM = memregion("maincpu")->base();
	LOG(("%s(%02x)\n", FUNCNAME, data));
	//  membank("bank1")->set_entry((data & 0x70) >> 4);
	m_bank1->set_entry((data & 0x70) >> 4);
#if 0
	switch (data & 0x70){
	case 0x00: membank("bank1")->set_base(&RAM[0x10000]); break;
	case 0x10: membank("bank1")->set_base(&RAM[0x18000]); break;
	case 0x20: membank("bank1")->set_base(&RAM[0x20000]); break;
	case 0x30: membank("bank1")->set_base(&RAM[0x28000]); break;
	case 0x40: membank("bank1")->set_base(&RAM[0x30000]); break;
	case 0x50: membank("bank1")->set_base(&RAM[0x38000]); break;
	case 0x60: membank("bank1")->set_base(&RAM[0x40000]); logerror("strange memory bank"); break;
	case 0x70: membank("bank1")->set_base(&RAM[0x48000]); logerror("strange memory bank"); break;
	default: logerror("%s Programming error, please report!\n", FUNCNAME);
	}
#endif
}

WRITE_LINE_MEMBER(can09_state::pia1_cb2_w)
{
	LOG(("%s(%02x)\n", FUNCNAME, state));
}

static INPUT_PORTS_START( can09 )
	PORT_START("LINE0")
	PORT_START("LINE1")
	PORT_START("LINE2")
	PORT_START("LINE3")
	PORT_START("LINE4")
INPUT_PORTS_END

// traced and guessed from pcb images and debugger
// It is very likelly that this is a PIA based dynamic address map, needs more analysis
static ADDRESS_MAP_START( can09_map, AS_PROGRAM, 8, can09_state )
/*
 * Port A=0x18 B=0x20 erase 0-7fff
 * Port A=0x18 B=0x30 erase 0-7fff
 * Port A=0x18 B=0x00
 * Port A=0x10 B=
*/
//  AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_RAMBANK("bank1")
	AM_RANGE(0xe020, 0xe020) AM_DEVWRITE("crtc", h46505_device, address_w)
	AM_RANGE(0xe021, 0xe021) AM_DEVWRITE("crtc", h46505_device, register_w)
	AM_RANGE(0xe034, 0xe037) AM_DEVREADWRITE(PIA1_TAG, pia6821_device, read, write)

#if 0
	AM_RANGE(0xb100, 0xb100) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0xb101, 0xb101) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0xb110, 0xb113) AM_DEVREADWRITE(PIA1_TAG, pia6821_device, read_alt, write_alt)
	AM_RANGE(0xb120, 0xb123) AM_DEVREADWRITE(PIA2_TAG, pia6821_device, read_alt, write_alt)
	AM_RANGE(0xb130, 0xb137) AM_DEVREADWRITE("ptm", ptm6840_device, read, write)
	AM_RANGE(0xb200, 0xc1ff) AM_ROM AM_REGION("roms", 0x3200)
	AM_RANGE(0xc200, 0xdfff) AM_RAM /* Needed for BASIC etc */
#endif
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

/* E100 Input ports
 * Four e100 keys are not mapped yet,
 * - The redundant '*' on the keyboard together with the '\'' single quote, both on same e100 key
 * - The 'E' key on the keypad, presumably used for calculator applications to remove the last entered number
 * - The 'Break' key on rev2 will be mapped to NMI at some point, a recomended modification of the rev1 mother board
 * - The 'REPT' key has a so far unknown function
 */
static INPUT_PORTS_START( e100 )
/*  Bits read on PIA1 A when issueing line number on PIA1 B bits 0-3 through a 74145 demultiplexer */
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_LSHIFT)       PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)   PORT_NAME("REPT") /* Not mapped yet */
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_UNUSED)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_Z)            PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_A)            PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_Q)            PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_1)            PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_UNUSED)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR('-')
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_X)            PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_S)            PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_W)            PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_2)            PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR('*')

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_COLON)        PORT_CHAR(0xF6) PORT_CHAR(0xD6)
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_L)            PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_O)            PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_9)            PORT_CHAR(')')  PORT_CHAR('9')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_P)            PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_0)            PORT_CHAR('0')  PORT_CHAR('=')

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(0xE4) PORT_CHAR(0xC4)
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_K)            PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_I)            PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_8)            PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(0xE5) PORT_CHAR(0xC5)
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('+')  PORT_CHAR('?')

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD) PORT_NAME("'/*") /* No good mapping */
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_M)            PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_J)            PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_U)            PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_7)            PORT_CHAR('7')  PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD) PORT_NAME("PI")               PORT_CODE(KEYCODE_ESC)          PORT_CHAR(0x27)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_N)            PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_H)            PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_Y)            PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_6)            PORT_CHAR('&')  PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_ENTER)        PORT_CHAR('\r')
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('<')  PORT_CHAR('>')

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_B)            PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_G)            PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_T)            PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_5)            PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(STOP))
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_V)            PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_F)            PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_R)            PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_4)            PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW,   IPT_KEYBOARD) PORT_NAME("Keypad E") /* No good mapping */
	PORT_BIT(0x04, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_C)            PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_D)            PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_E)            PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_3)            PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x40, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW,   IPT_KEYBOARD)                               PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
INPUT_PORTS_END

static INPUT_PORTS_START( md6802 )
	PORT_START("LINE0") /* KEY ROW 0 */
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)  PORT_CHAR('0')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)  PORT_CHAR('1')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)  PORT_CHAR('2')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)  PORT_CHAR('3')
	PORT_BIT(0xf0, 0x00, IPT_UNUSED )

	PORT_START("LINE1") /* KEY ROW 1 */
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)  PORT_CHAR('4')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)  PORT_CHAR('5')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)  PORT_CHAR('6')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)  PORT_CHAR('7')
	PORT_BIT(0xf0, 0x00, IPT_UNUSED )

	PORT_START("LINE2") /* KEY ROW 2 */
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)  PORT_CHAR('8')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)  PORT_CHAR('9')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)  PORT_CHAR('A')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)  PORT_CHAR('B')
	PORT_BIT(0xf0, 0x00, IPT_UNUSED )

	PORT_START("LINE3") /* KEY ROW 3 */
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)  PORT_CHAR('C')
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)  PORT_CHAR('D')
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)  PORT_CHAR('E')
	PORT_BIT(0x08, 0x08, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)  PORT_CHAR('F')
	PORT_BIT(0xf0, 0x00, IPT_UNUSED )

	PORT_START("LINE4") /* Special KEY ROW for reset and Shift/'*' keys */
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR('*')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0xf3, 0x00, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mp68a )
	PORT_START("LINE0") /* KEY ROW 0 */
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)    PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)    PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)    PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)    PORT_CHAR('F')
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LINE1") /* KEY ROW 1 */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)    PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)    PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)    PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)    PORT_CHAR('B')
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LINE2") /* KEY ROW 2 */
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)    PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)    PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)    PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)    PORT_CHAR('7')
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LINE3") /* KEY ROW 3 */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)    PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)    PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)    PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)    PORT_CHAR('3')
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LINE4") /* Special KEY ROW for reset and Shift/'*' keys */
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0xf3, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

#ifdef UNUSED_VARIABLE
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END
#endif

// TODO: Fix shift led for mp68a correctly, workaround doesn't work anymore! Shift works though...
TIMER_DEVICE_CALLBACK_MEMBER(didact_state::scan_artwork)
{
	//  LOG(("--->%s()\n", FUNCNAME));

	// Poll the artwork Reset key
	if ( (m_io_line4->read() & 0x04) )
	{
		LOG( ("RESET is pressed, resetting the CPU\n") );
		m_shift = 0;
		output().set_led_value(m_led, m_shift); // For mp68a only
		if (m_reset == 0)
		{
			machine_reset();
		}
		m_reset = 1; // Inhibit multiple resets
	}

		// Poll the artwork SHIFT/* key
	else if ( (m_io_line4->read() & 0x08) )
	{
		LOG( ("%s", !m_shift ? "SHIFT is set\n" : "") );
		m_shift = 1;
		output().set_led_value(m_led, m_shift); // For mp68a only
	}
	else
	{
		if (m_reset == 1)
		{
			m_reset = 0; // Enable reset again
		}
	}
}

/* Fake clock values until we TODO: figure out how the PTM generates the clocks */
#define CAN09T_BAUDGEN_CLOCK XTAL_1_8432MHz
#define CAN09T_ACIA_CLOCK (CAN09T_BAUDGEN_CLOCK / 12)

static MACHINE_CONFIG_START( can09t, can09t_state )
	MCFG_CPU_ADD("maincpu", M6809, XTAL_4_9152MHz) // IPL crystal
	MCFG_CPU_PROGRAM_MAP(can09t_map)

	/* --PIA inits----------------------- */
	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0) // CPU board
	MCFG_PIA_READPA_HANDLER(READ8(can09t_state, pia1_A_r))
	MCFG_PIA_READPB_HANDLER(READ8(can09t_state, pia1_B_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(can09t_state, pia1_B_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(can09t_state, pia1_cb2_w))
	/* 0xE1FB 0xB112 (PIA1 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xE1FB 0xB113 (PIA1 Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xE203 0xB110 (PIA1 DDR A)     = 0x00 - Port A all inputs */
	/* 0xE203 0xB111 (PIA1 DDR B)     = 0x7F - Port B mixed mode */
	/* 0xE20A 0xB112 (PIA1 Control A) = 0x05 - IRQ A enabled on falling transition on CA2 */
	/* 0xE20A 0xB113 (PIA1 Control B) = 0x34 - CB2 is low and lock DDRB */
	/* 0xE20E 0xB111 (PIA1 port B)    = 0x10 - Data to port B */
	MCFG_DEVICE_ADD(PIA2_TAG, PIA6821, 0) // CPU board
	MCFG_PIA_CB2_HANDLER(WRITELINE(can09t_state, pia2_cb2_w))
	/* 0xE212 0xB122 (PIA2 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xE212 0xB123 (PIA2 Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xE215 0xB120 (PIA2 DDR A)     = 0x00 - Port A all inputs */
	/* 0xE215 0xB121 (PIA2 DDR B)     = 0xFF - Port B all outputs */
	/* 0xE21A 0xB122 (PIA2 Control A) = 0x34 - CA2 is low and lock DDRB */
	/* 0xE21A 0xB123 (PIA2 Control B) = 0x34 - CB2 is low and lock DDRB */
	MCFG_DEVICE_ADD(PIA3_TAG, PIA6821, 0) // ROM board
	MCFG_DEVICE_ADD(PIA4_TAG, PIA6821, 0) // ROM board

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)

	/* RS232 usage: mame can09t -window -debug -rs232 terminal */
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE ("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE ("rs232", rs232_port_device, write_rts))
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE ("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE ("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD ("acia_clock", CLOCK, CAN09T_ACIA_CLOCK)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE (can09t_state, write_acia_clock))
MACHINE_CONFIG_END

#define CAN09_X1_CLOCK XTAL_22_1184MHz        /* UKI 22118.40 Khz */
#define CAN09_CPU_CLOCK (CAN09_X1_CLOCK / 16) /* ~1.38MHz Divider needs to be check but is the most likelly */
static MACHINE_CONFIG_START( can09, can09_state )
	MCFG_CPU_ADD("maincpu", M6809E, CAN09_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(can09_map)

	/* RAM banks */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("768K")

	// CRTC  init
	MCFG_MC6845_ADD("crtc", H46505, "screen", CAN09_CPU_CLOCK) // TODO: Check actual clock source, An 8MHz UKI crystal is also nearby
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	//MCFG_MC6845_UPDATE_ROW_CB(can09_state, crtc_update_row) // not written yet

	/* Setup loop from data table in ROM: 0xFFCB 0xE020 (CRTC register number), 0xFFD0 0xE021 (CRTC register value)
	    Reg  Value Comment
	    0x00 0x55  Horizontal Total number of characters,
	    0x01 0x40  Horizontal Displayed number of characters
	    0x02 0x43  Horizontal Sync Position, character number
	    0x03 0x03  Horizontal Sync width, number of charcters
	    0x04 0x50  Vertical Total number of characters
	    0x05 0x09  Vertical Total Adjust number of characters
	    0x06 0x48  Vertical Displayed number of characters
	    0x07 0x4B  Vertical Sync Position, character number
	    0x08 0x00  Interlace Mode/Scew, Non-Interlaced
	    0x09 0x03  Max Scan Line Address Register
	    0x0A 0x00  Cursor Start
	    0x0B 0x0A  Cursor End
	    0x0C 0x00  Start Address hi
	    0x0D 0x00  Start Address lo
	    0x0E 0x00  Cursor hi
	    0x0F 0x00  Cursor lo
	    Note - no init of Light Pen registers
	*/



	/* screen - totally faked value for now */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_RAW_PARAMS(XTAL_4MHz/2, 512, 0, 512, 576, 0, 576)
	MCFG_SCREEN_UPDATE_DRIVER(can09_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* Floppy */
	MCFG_WD1770_ADD("wd1770", XTAL_8MHz ) // TODO: Verify 8MHz UKI crystal assumed to be used
#if 0
	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", candela_floppies, "35dd", floppy_image_device::default_floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("flop525_list", "candela")
#endif

	/* --PIA inits----------------------- */
	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0) // CPU board
	MCFG_PIA_READPA_HANDLER(READ8(can09_state, pia1_A_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(can09_state, pia1_A_w))
	MCFG_PIA_READPB_HANDLER(READ8(can09_state, pia1_B_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(can09_state, pia1_B_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(can09_state, pia1_cb2_w))
	/* 0xFF7D 0xE035 (PIA1 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xFF81 0xE037 (PIA1 Control B) = 0x00 - Channel A IRQ disabled */
	/* 0xFF85 0xE034 (PIA1 DDR A)     = 0x1F - Port A mixed mode */
	/* 0xFF89 0xE036 (PIA1 DDR B)     = 0x79 - Port B mixed mode */
	/* 0xFF8D 0xE035 (PIA1 Control A) = 0x04 - Channel A lock DDR */
	/* 0xFF8F 0xE037 (PIA1 Control B) = 0x04 - Channel B lock DDR */
	/* 0xFF93 0xE034 (PIA1 Port B)    = 0x18 - Write Data on Port B */

#if 1
	MCFG_DEVICE_ADD(PIA2_TAG, PIA6821, 0) // CPU board
	MCFG_DEVICE_ADD("acia1", ACIA6850, 0) // CPU board
	MCFG_DEVICE_ADD("acia2", ACIA6850, 0) // CPU board
#endif
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( e100, e100_state )
	MCFG_CPU_ADD("maincpu", M6802, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(e100_map)

	/* Devices */
	MCFG_DEVICE_ADD("kbd_74145", TTL74145, 0)

	/* --PIA inits----------------------- */
	/* 0xF883 0xC818 (PIA1 DDR A)     = 0x00 - Port A all inputs */
	/* 0xF883 0xC818 (PIA2 DDR A)     = 0x00 - Port A all inputs */
	/* 0xF883 0xC818 (PIA1 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xF883 0xC818 (PIA2 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xF886 0xC81A (PIA1 DDR B)     = 0x00 - Port B all inputs */
	/* 0xF886 0xC81A (PIA2 DDR B)     = 0x00 - Port B all inputs */
	/* 0xF886 0xC81A (PIA1 Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xF886 0xC81A (PIA2 Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xF88e 0xC80A (PIA1 DDR B)     = 0x4F - Port B 5 outputs set to 0 */
	/* 0xF890 0xC812 (PIA2 DDR B)     = 0xFF - Port B all outputs set to 0 */
	/* 0xF894 0xC818 (PIA1 Control A) = 0x34 - CA2 is low and lock DDRA */
	/* 0xF894 0xC818 (PIA2 Control A) = 0x34 - CA2 is low and lock DDRA */
	/* 0xF896 0xC818 (PIA1 Control B) = 0x34 - CB2 is low and lock DDRB */
	/* 0xF896 0xC818 (PIA2 Control B) = 0x34 - CB2 is low and lock DDRB */
	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(e100_state, pia1_kbA_w))
	MCFG_PIA_READPA_HANDLER(READ8(e100_state, pia1_kbA_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(e100_state, pia1_kbB_w))
	MCFG_PIA_READPB_HANDLER(READ8(e100_state, pia1_kbB_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(e100_state, pia1_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(e100_state, pia1_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(e100_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(e100_state, pia1_cb2_w))

	/* The optional second PIA enables the expansion port on CA1 and a software RTC with 50Hz resolution */
	MCFG_DEVICE_ADD(PIA2_TAG, PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE))

	/* Serial port support */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)

	/* Cassette support - E100 uses 300 baud Kansas City Standard with 1200/2400 Hz modulation */
	/* NOTE on usage: mame e100 -window -cass <wav file> -ui_active
	 * Once running enable/disable internal UI by pressing Scroll Lock in case it interferes with target keys
	 * Open the internal UI by pressing TAB and then select 'Tape Control' or use F2/Shift F2 for PLAY/PAUSE
	 * In order to use a wav file it has first to be created using TAB and select the 'File manager'
	 * Once created it may be given on the commandline or mounted via TAB and select
	 * E100 supports cassette through the 'LOAD' and 'SAVE' commands with no arguments
	 */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_MUTED | CASSETTE_MOTOR_ENABLED)

	/* screen TODO: simplify the screen config, look at zx.cpp */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_4MHz/2, 265, 0, 265, 265, 0, 265)
	MCFG_SCREEN_UPDATE_DRIVER(e100_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* There is a 50Hz signal from the video circuit to CA1 which generates interrupts and drives a software RTC */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("video50hz", e100_state, rtc_w, attotime::from_hz(100)) /* Will be divided by two through toggle in the handler */
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( md6802, md6802_state )
	MCFG_CPU_ADD("maincpu", M6802, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(md6802_map)
	MCFG_DEFAULT_LAYOUT(layout_md6802)

	/* Devices */
	MCFG_DEVICE_ADD("tb16_74145", TTL74145, 0)
	/* PIA #1 0xA000-0xA003 - used differently by laborations and loaded software */
	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0)

	/* PIA #2 Keyboard & Display 0xC000-0xC003 */
	MCFG_DEVICE_ADD(PIA2_TAG, PIA6821, 0)
	/* --PIA init----------------------- */
	/* 0xE007 0xC002 (DDR B)     = 0xFF - Port B all outputs and set to 0 (zero) */
	/* 0xE00B 0xC000 (DDR A)     = 0x70 - Port A three outputs and set to 0 (zero) */
	/* 0xE00F 0xC001 (Control A) = 0x3C - */
	/* 0xE013 0xC003 (Control B) = 0x3C - */
	/* --execution-wait for key loop-- */
	/* 0xE026 0xC000             = (Reading Port A)  */
	/* 0xE033 0xC000             = (Reading Port A)  */
	/* 0xE068 0xC000 (Port A)    = 0x60 */
	/* 0xE08A 0xC002 (Port B)    = 0xEE - updating display */
	/* 0xE090 0xC000 (Port A)    = 0x00 - looping in 0x10,0x20,0x30,0x40,0x50 */
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(md6802_state, pia2_kbA_w))
	MCFG_PIA_READPA_HANDLER(READ8(md6802_state, pia2_kbA_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(md6802_state, pia2_kbB_w))
	MCFG_PIA_READPB_HANDLER(READ8(md6802_state, pia2_kbB_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(md6802_state, pia2_ca2_w))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", md6802_state, scan_artwork, attotime::from_hz(10))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mp68a, mp68a_state )
	// Clock source is based on a N9602N Dual Retriggerable Resettable Monostable Multivibrator oscillator at aprox 505KHz.
	// Trimpot seems broken/stuck at 5K Ohm thu. ROM code 1Ms delay loops suggest 1MHz+
	MCFG_CPU_ADD("maincpu", M6800, 505000)
	MCFG_CPU_PROGRAM_MAP(mp68a_map)
	MCFG_DEFAULT_LAYOUT(layout_mp68a)

	/* Devices */
	/* PIA #1 0x500-0x503 - used differently by laborations and loaded software */
	MCFG_DEVICE_ADD(PIA1_TAG, PIA6820, 0)

	/* PIA #2 Keyboard & Display 0x600-0x603 */
	MCFG_DEVICE_ADD(PIA2_TAG, PIA6820, 0)
	/* --PIA inits----------------------- */
	/* 0x0BAF 0x601 (Control A) = 0x30 - CA2 is low and enable DDRA */
	/* 0x0BB1 0x603 (Control B) = 0x30 - CB2 is low and enable DDRB */
	/* 0x0BB5 0x600 (DDR A)     = 0xFF - Port A all outputs and set to 0 (zero) */
	/* 0x0BB9 0x602 (DDR B)     = 0x50 - Port B two outputs and set to 0 (zero) */
	/* 0x0BBD 0x601 (Control A) = 0x34 - CA2 is low and lock DDRA */
	/* 0x0BBF 0x603 (Control B) = 0x34 - CB2 is low and lock DDRB */
	/* 0x0BC3 0x602 (Port B)    = 0x40 - Turn on display via RBI* on  */
	/* --execution-wait for key loop-- */
	/* 0x086B Update display sequnc, see below                            */
	/* 0x0826 CB1 read          = 0x603 (Control B)  - is a key pressed? */
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mp68a_state, pia2_kbA_w))
	MCFG_PIA_READPA_HANDLER(READ8(mp68a_state, pia2_kbA_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mp68a_state, pia2_kbB_w))
	MCFG_PIA_READPB_HANDLER(READ8(mp68a_state, pia2_kbB_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(mp68a_state, pia2_cb1_r))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE)) /* Not used by ROM. Combined trace to CPU IRQ with IRQB */
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("maincpu", M6800_IRQ_LINE)) /* Not used by ROM. Combined trace to CPU IRQ with IRQA */

	/* Display - sequence outputting all '0':s at start */
	/* 0x086B 0x600 (Port A)    = 0x00 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	/* 0x086B 0x600 (Port A)    = 0x10 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	/* 0x086B 0x600 (Port A)    = 0x20 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	/* 0x086B 0x600 (Port A)    = 0x30 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	/* 0x086B 0x600 (Port A)    = 0x40 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	/* 0x086B 0x600 (Port A)    = 0x50 */
	/* 0x086B 0x600 (Port A)    = 0x70 */
	MCFG_DEVICE_ADD("digit0", DM9368, 0)
	MCFG_OUTPUT_INDEX(0)
	MCFG_DEVICE_ADD("digit1", DM9368, 0)
	MCFG_OUTPUT_INDEX(1)
	MCFG_DEVICE_ADD("digit2", DM9368, 0)
	MCFG_OUTPUT_INDEX(2)
	MCFG_DEVICE_ADD("digit3", DM9368, 0)
	MCFG_OUTPUT_INDEX(3)
	MCFG_DEVICE_ADD("digit4", DM9368, 0)
	MCFG_OUTPUT_INDEX(4)
	MCFG_DEVICE_ADD("digit5", DM9368, 0)
	MCFG_OUTPUT_INDEX(5)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", mp68a_state, scan_artwork, attotime::from_hz(10))
MACHINE_CONFIG_END

ROM_START( can09t )
	ROM_REGION(0x10000, "roms", 0)
	/* CAN09 v7 and CDBASIC 3.8 */
	ROM_LOAD( "ic2-mon58b-c8d7.bin", 0x0000, 0x8000, CRC(7eabfec6) SHA1(e08e2349035389b441227df903aa54f4c1e4a337) )
	/* Programmable logic for the CAN09 1.4 PCB */
	ROM_REGION(0x1000, "plas", 0)
	ROM_LOAD( "ic10-21.1.bin",       0x0000, 0x20,   CRC(b75ac72d) SHA1(689363200035b11a823d17a8d717f313eeefc3bf) )
ROM_END

ROM_START( can09 )
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD( "ic14-vdu42.bin", 0x0000, 0x2000, CRC(67fc3c8c) SHA1(1474d6259646798377ef4ce7e43d3c8d73858344) )
ROM_END

/* ROM sets from Didact was not versioned in general, so the numbering are just assumptions */
ROM_START( e100 )
	ROM_REGION(0x4000, "roms", 0)
	ROM_DEFAULT_BIOS("rev2 BASIC")

	/* TODO: Get the original ROMs */
	ROM_SYSTEM_BIOS(0, "rev1 BASIC", "Esselte 100 rev1 BASIC")
	ROMX_LOAD( "e100r1U201.bin", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "e100r1U202.bin", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "e100r1U203.bin", 0x2000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "e100r1U204.bin", 0x2800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "e100r1U205.bin", 0x3000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "e100r1U206.bin", 0x3800, 0x0800, NO_DUMP, ROM_BIOS(1) )

	/* This is a prototype ROM, commercial relase not verified. The prototype also have different keyboard and supports
	   more ram so might need to be split out as a clone later */
	ROM_SYSTEM_BIOS(1, "rev2 BASIC", "Esselte 100 rev2 BASIC")
	ROMX_LOAD( "e100r2U201.bin", 0x0000, 0x2000, CRC(53513b67) SHA1(a91c5c32aead82dcc87db5d818ff286a7fc6a5c8), ROM_BIOS(2) )
	ROMX_LOAD( "e100r2U202.bin", 0x2000, 0x2000, CRC(eab3adf2) SHA1(ff3f5f5c8ea8732702a39cff76d0706ab6b751ee), ROM_BIOS(2) )

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "e100U506.bin", 0x0000, 0x0800, CRC(fff9f288) SHA1(2dfb3eb551fe1ef67da328f61ef51ae8d1abdfb8) )
ROM_END

// TODO split ROM image into proper ROM set
ROM_START( md6802 ) // ROM image from http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=135#p1203640
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "DIDACT.bin", 0xe000, 0x0800, CRC(50430b1d) SHA1(8e2172a9ae95b04f20aa14177df2463a286c8465) )
ROM_END

ROM_START( mp68a ) // ROM image from http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=135#p1203640
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "didactA.bin", 0x0800, 0x0200, CRC(aa05e1ce) SHA1(9ce8223efd274045b43ceca3529e037e16e99fdf) )
	ROM_LOAD( "didactB.bin", 0x0a00, 0x0200, CRC(592898dc) SHA1(2962f4817712cae97f3ab37b088fc73e66535ff8) )
ROM_END

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   CLASS            INIT        COMPANY             FULLNAME                     FLAGS
COMP( 1979, mp68a,      0,          0,      mp68a,      mp68a,  driver_device,   0,          "Didact AB",        "mp68a",                     MACHINE_NO_SOUND_HW )
COMP( 1982, e100,       0,          0,      e100,       e100,   driver_device,   0,          "Didact AB",        "Esselte 100",               MACHINE_NO_SOUND_HW )
COMP( 1983, md6802,     0,          0,      md6802,     md6802, driver_device,   0,          "Didact AB",        "Mikrodator 6802",           MACHINE_NO_SOUND_HW )
COMP( 1984, can09,      0,          0,      can09,      can09,  driver_device,   0,          "Candela Data AB",  "Candela CAN09 main unit",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1984, can09t,     0,          0,      can09t,     can09t, driver_device,   0,          "Candela Data AB",  "Candela CAN09 terminal",    MACHINE_NO_SOUND_HW )
