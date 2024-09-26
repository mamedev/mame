// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 *
 * History of Didact and Esselte Studium
 *--------------------------------------
 * Didact Laromedelsproduktion was started in Linkoping in Sweden by Anders Andersson, Arne Kullbjer and
 * Lars Bjorklund. They constructed a series of microcomputers for educational purposes such as "Mikrodator 6802",
 * Esselte 100 and the Candela computer for the swedish schools to educate the students in assembly programming
 * and BASIC for electro mechanical applications such as stepper motors, simple process control, buttons
 * and LED:s. Didact designs were marketed by Esselte Studium to the swedish schools. Late designs like the
 * "Modulab v2" appears to have been owned or licensed to Esselte and enhanced with more modular monitor routines
 * in a project driven by Alf Karlsson.
 *
 * The Esselte 1000 was an educational package based on Apple II plus software and literature
 * but the relation to Didact is at this point unknown so it is probably a pure Esselte software production.
 *
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 * http://elektronikforumet.com/forum/viewtopic.php?f=11&t=51424
 * http://kilroy71.fastmail.fm/gallery/Miscellaneous/20120729_019.jpg
 * http://elektronikforumet.com/forum/download/file.php?id=63988&mode=view
 * http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=150#p1203915
 *
 *  TODO:
 *  Didact designs:    mp68a, md6802, Modulab
 * ------------------------------------------
 *  - Add PCB layouts   OK     OK     OK
 *  - Dump ROM:s,       OK     OK     OK
 *  - Keyboard          OK     OK     OK
 *  - Display/CRT       OK     OK     OK
 *  - Clickable Artwork RQ     RQ     OK
 *  - Sound             NA     NA
 *  - Cassette i/f
 *  - Expansion bus
 *  - Expansion overlay
 *  - Interrupts        OK
 *  - Serial                   XX
 *   XX = needs debug
 *********************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h" // For all boards
#include "machine/6821pia.h" // For all boards
#include "machine/74145.h"   // For the md6802
#include "video/dm9368.h"    // For the mp68a
#include "machine/ins8154.h" // For the modulab
#include "machine/mm74c922.h"// For the modulab
#include "machine/rescap.h"  // For the modulab

// Features
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "speaker.h"

// Generated artwork includes
#include "mp68a.lh"
#include "md6802.lh"
#include "modulab.lh"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_SETUP    (1U << 1)
#define LOG_READ     (1U << 2)
#define LOG_DISPLAY  (1U << 3)
#define LOG_KEYBOARD (1U << 4)

//#define VERBOSE (LOG_KEYBOARD)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,    __VA_ARGS__)
#define LOGREAD(...)    LOGMASKED(LOG_READ,     __VA_ARGS__)
#define LOGDISPLAY(...) LOGMASKED(LOG_DISPLAY,  __VA_ARGS__)
#define LOGKBD(...)     LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


namespace {

#define PIA1_TAG "pia1"
#define PIA2_TAG "pia2"
#define PIA3_TAG "pia3"
#define PIA4_TAG "pia4"
#define MM74C923_TAG "74c923"

/* Didact base class */
class didact_state : public driver_device
{
	public:
	didact_state(const machine_config &mconfig, device_type type, const char * tag)
		: driver_device(mconfig, type, tag)
		, m_cass(*this, "cassette")
		, m_io_lines(*this, "LINE%u", 0U)
		, m_lines{ 0, 0, 0, 0 }
		, m_rs232(*this, "rs232")
		, m_led(*this, "led1")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_shift);
protected:
	virtual void machine_start() override { m_led.resolve(); }

	optional_device<cassette_image_device> m_cass;
	required_ioport_array<5> m_io_lines;
	uint8_t m_lines[4]{};
	uint8_t m_reset = 0;
	uint8_t m_shift = 0;
	optional_device<rs232_port_device> m_rs232;
	output_finder<> m_led;
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
		, m_maincpu(*this, "maincpu")
		, m_tb16_74145(*this, "tb16_74145")
		, m_pia1(*this, PIA1_TAG)
		, m_pia2(*this, PIA2_TAG)
		, m_7segs(*this, "digit%u", 0U)
		, m_segments(0)
	{ }

	void md6802(machine_config &config);

protected:
	uint8_t pia2_kbA_r();
	void pia2_kbA_w(uint8_t data);
	uint8_t pia2_kbB_r();
	void pia2_kbB_w(uint8_t data);
	void pia2_ca2_w(int state);

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void md6802_map(address_map &map) ATTR_COLD;

private:
	required_device<m6802_cpu_device> m_maincpu;
	required_device<ttl74145_device> m_tb16_74145;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	output_finder<6> m_7segs;
	uint8_t m_segments;
};

/* Keyboard */
uint8_t md6802_state::pia2_kbA_r()
{
	uint8_t ls145;
	uint8_t pa = 0xff;

	// Read out the selected column
	ls145 = m_tb16_74145->read() & 0x0f;

	// read out the artwork, line04 is handled by the timer
	for (unsigned i = 0U; 4U > i; ++i)
	{
		m_lines[i] = m_io_lines[i]->read();

		// Mask out those rows that has a button pressed
		pa &= ~(((~m_lines[i] & ls145) != 0) ? (1 << i) : 0);
	}

	if (m_shift)
	{
		pa &= 0x7f;   // Clear shift bit if button being pressed (PA7) to ground (internal pullup)
		LOGKBD("SHIFT is pressed\n");
	}

	// Serial IN - needs debug/verification
	pa &= (m_rs232->rxd_r() != 0 ? 0xff : 0x7f);

	return pa;
}

/* Pull the cathodes low enabling the correct digit and lit the segments held by port B */
void md6802_state::pia2_kbA_w(uint8_t data)
{
//  LOG("--->%s(%02x)\n", FUNCNAME, data);

	uint8_t const digit_nbr((data >> 4) & 0x07);
	m_tb16_74145->write(digit_nbr);
	if (digit_nbr < 6)
		m_7segs[digit_nbr] = m_segments;
}

/* PIA 2 Port B is all outputs to drive the display so it is very unlikely that this function is called */
uint8_t md6802_state::pia2_kbB_r()
{
	LOG("Warning, trying to read from Port B designated to drive the display, please check why\n");
	logerror("Warning, trying to read from Port B designated to drive the display, please check why\n");
	return 0;
}

/* Port B is fully used outputting the segment pattern to the display */
void  md6802_state::pia2_kbB_w(uint8_t data)
{
//  LOG("--->%s(%02x)\n", FUNCNAME, data);

	/* Store the segment pattern but do not lit up the digit here, done by pulling the correct cathode low on Port A */
	m_segments = bitswap<8>(data, 0, 4, 5, 3, 2, 1, 7, 6);
}

void md6802_state::pia2_ca2_w(int state)
{
	LOGKBD("--->%s(%02x) LED is connected through resisitor to +5v so logical 0 will lit it\n", FUNCNAME, state);
	m_led = state ? 0 :1;

	// Serial Out - needs debug/verification
	m_rs232->write_txd(state);

	m_shift = !state;
}

void md6802_state::machine_start()
{
	LOG("--->%s()\n", FUNCNAME);

	didact_state::machine_start();
	m_7segs.resolve();

	save_item(NAME(m_reset));
	save_item(NAME(m_shift));
}

void md6802_state::machine_reset()
{
	LOG("--->%s()\n", FUNCNAME);
	m_maincpu->reset();
}

// This address map is traced from schema
void md6802_state::md6802_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().mirror(0x1800);
	map(0xa000, 0xa003).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).mirror(0x1ffc);
	map(0xc000, 0xc003).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).mirror(0x1ffc);
	map(0xe000, 0xe7ff).rom().mirror(0x1800).region("maincpu", 0xe000);
}

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
// 2019-07-27 Cassette added: saves ok, load is unreliable, probably an original design problem.
#define pia6820_device pia6821_device
#define PIA6820 PIA6821
class mp68a_state : public didact_state
{
	public:
	mp68a_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
		, m_7segs(*this, "digit%u", 0U)
		, m_pia1(*this, PIA1_TAG)
		, m_pia2(*this, PIA2_TAG)
	{ }

	required_device<m6800_cpu_device> m_maincpu;

	// The display segment driver device (there is actually just one, needs rewrite to be correct)
	required_device_array<dm9368_device, 6> m_digits;
	output_finder<6> m_7segs;

	uint8_t pia2_kbA_r();
	void pia2_kbA_w(uint8_t data);
	uint8_t pia2_kbB_r();
	void pia2_kbB_w(uint8_t data);
	int pia2_cb1_r();
	template <unsigned N> void digit_w(uint8_t data) { m_7segs[N] = data; }

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void mp68a(machine_config &config);
	void mp68a_map(address_map &map) ATTR_COLD;
protected:
	required_device<pia6820_device> m_pia1;
	required_device<pia6820_device> m_pia2;
};

INPUT_CHANGED_MEMBER(didact_state::trigger_shift)
{
	if (newval == CLEAR_LINE)
	{
		LOGKBD("SHIFT is released\n");
	}
	else
	{
		LOGKBD("SHIFT is pressed\n");
		m_shift = 1;
		m_led = 1;
	}
}

uint8_t mp68a_state::pia2_kbA_r()
{
	LOG("--->%s\n", FUNCNAME);

	return 0;
}

void mp68a_state::pia2_kbA_w(uint8_t data)
{
	/* Display memory is at $702 to $708 in AAAADD format (A=address digit, D=Data digit)
	   but we are using data read from the port. */
	uint8_t const digit_nbr = (data >> 4) & 0x07;

	/* There is actually only one 9368 and a 74145 to drive the cathode of the right digit low */
	/* This can be emulated by pretending there are one 9368 per digit, at least for now      */
	switch (digit_nbr)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		m_digits[digit_nbr]->a_w(data & 0x0f);
		break;
	case 7: // used as an 'unselect' by the ROM between digit accesses.
		break;
	default:
		logerror("Invalid digit index %d\n", digit_nbr);
	}
}

uint8_t mp68a_state::pia2_kbB_r()
{
	uint8_t a012, line, pb;

	LOGKBD("--->%s %02x %02x %02x %02x %02x => ", FUNCNAME, m_lines[0], m_lines[1], m_lines[2], m_lines[3], m_shift);

	a012 = 0;
	if ((line = (m_lines[0] | m_lines[1])) != 0)
	{
		a012 = 8;
		while (a012 > 0 && !(line & (1 << --a012)));
		a012 += 8;
	}
	if ( a012 == 0 && (line = ((m_lines[2]) | m_lines[3])) != 0)
	{
		a012 = 8;
		while (a012 > 0 && !(line & (1 << --a012)));
	}

	pb  = a012;       // A0-A2 -> PB0-PB3

	if (m_shift)
	{
		pb |= 0x80;   // Set shift bit (PB7)
		m_shift = 0;  // Reset flip flop
		m_led = 0;
		LOGKBD(" SHIFT is released\n");
	}

	pb |= (m_cass->input() < 0.04) ? 0x20 : 0;
	LOGKBD("%02x\n", pb);

	return pb;
}

void mp68a_state::pia2_kbB_w(uint8_t data)
{
	LOG("--->%s(%02x)\n", FUNCNAME, data);
	m_cass->output(BIT(data, 4) ? -1.0 : +1.0);
}

int mp68a_state::pia2_cb1_r()
{
	for (unsigned i = 0U; 4U > i; ++i)
		m_lines[i] = m_io_lines[i]->read();

	if ((VERBOSE & LOG_GENERAL) && (m_lines[0] | m_lines[1] | m_lines[2] | m_lines[3]))
		LOG("%s()-->%02x %02x %02x %02x\n", FUNCNAME, m_lines[0], m_lines[1], m_lines[2], m_lines[3]);

	return (m_lines[0] | m_lines[1] | m_lines[2] | m_lines[3]) ? 0 : 1;
}

void mp68a_state::machine_reset()
{
	LOG("--->%s()\n", FUNCNAME);
	m_maincpu->reset();
}

void mp68a_state::machine_start()
{
	LOG("--->%s()\n", FUNCNAME);

	didact_state::machine_start();
	m_7segs.resolve();

	/* register for state saving */
	save_item(NAME(m_shift));
	save_item(NAME(m_reset));
}

// This address map is traced from pcb
void mp68a_state::mp68a_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0xf000);
	map(0x0500, 0x0503).rw(m_pia1, FUNC(pia6820_device::read), FUNC(pia6820_device::write)).mirror(0xf0fc);
	map(0x0600, 0x0603).rw(m_pia2, FUNC(pia6820_device::read), FUNC(pia6820_device::write)).mirror(0xf0fc);
	map(0x0700, 0x07ff).ram().mirror(0xf000);
	map(0x0800, 0x0bff).rom().mirror(0xf400).region("maincpu", 0x0800);
}

//===================

/*   The Modulab CPU board, by Didact/Esselte ca 1984
 *  __________________________________________________________________________________________
 * |                                                    ADRESS               DATA             |
 * |              PORT A                      +-_--++-_--++-_--++-_--+   +-_--++-_--+   VCC   |
 * |    o   o   o   o   o   o   o   o         || | ||| | ||| | ||| | |   || | ||| | |    O    |
 * |    7   6   5   4   3   2   1   0         | -  || -  || -  || -  |   | -  || -  |         |
 * |    o   o   o   o   o   o   o   o         ||_|.|||_|.|||_|.|||_|.|   ||_|.|||_|.|   GND   |
 * |              PORT B                      +----++----++----++----+   +----++----+    O    |
 * |  o VCC                                    +--+  +--+  +--+  +--+     +--+  +--+          |
 * |                                           |LS|  |LS|  |LS|  |LS|     |LS|  |LS|          |
 * |  o GND                                    |164  |164  |164  |164     |164  |164          |
 * \\                                          |-5|<-|-4|<-|-3|<-|-2| <-  |-1|<-|-0|<- DB0    |
 * |\\ ____                                    +--+  +--+  +--+  +--+     +--+  +--+          |
 * | \/o  O|                                          +-------+-------+-------+-------+-------+
 * | |     |E           +--------------------+ +--+   |       |       |       |       |       |
 * | |     |X   +----+  |  PIA + 128x8 SRAM  | |LS|   |  RUN  |  ADS  |  FWD  | C/B   | RESET |
 * | |     |P   |4MHz|  |  INS8154N          | |14|   |       |       |       |       |       |
 * | |     |A   |XTAL|  +--------------------+ |  |   +-------+-------+-------+-------+-------+
 * | |     |N   |____|                         +--+   |       |       |       |       |       |
 * | |__   |S    |  |   +--------------------+ +--+   |   C   |   D   |   E   |   F   |       |
 * |  __|  |I           |  CPU               | |LS|   |       |       |       |       |       |
 * | |     |O           |  MC6802P           | |138   +-------+-------+-------+-------+       |
 * | |     |N           +--------------------+ |  |   |       |       |       |       |       |
 * | |     |B                                  +--+   |   8   |   9   |   A   |   B   |       |
 * | |     |U    IRQ    +-------------+        +--+   |       |       |       |       |       |
 * | |     |S    o      |  EPROM      |        |74|   +-------+-------+-------+-------+       |
 * | /\o  O|            |  2764       |        |C |   |       |       |       |       |       |
 * |// ----             +-------------+        |923   |   4   |   5   |   6   |   7   |       |
 * //                     +-----------+        |  |   |       |       |       |       |       |
 * |                      | 2KB SRAM  |        +--+   +-------+-------+-------+-------+       |
 * |                      | 6116      |        +--+   |       |       |       |       |       |
 * |                      +-----------+        |LS|   |   0   |   1   |   2   |   3   |       |
 * | ESSELTE       +-------+ +---+ +--------+  |138   |       |       |       |       |       |
 * | STUDIUM       |74LS123| |TRM| |SN74367 |  |  |   +-------+-------+-------+-------+       |
 * |               +-------+ +---+ +--------+  +--+
 * |__________________________________________________________________________________________|
 *
 */

/* Didact modulab driver class */
class modulab_state : public didact_state
{
	public:
	modulab_state(const machine_config &mconfig, device_type type, const char * tag)
		: didact_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_7segs(*this, "digit%u", 0U)
		, m_pia1(*this, PIA1_TAG)
		, m_kb(*this, MM74C923_TAG)
		, m_da(0)
	{ }

	required_device<m6802_cpu_device> m_maincpu;

	output_finder<6> m_7segs;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void modulab(machine_config &config);
protected:
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);
	void da_w(int state);
private:
	void modulab_map(address_map &map) ATTR_COLD;
	// Offsets for display and keyboard i/o
	enum
	{
		DISPLAY = 0,
		KEY_DATA = 2,
		KEY_STROBE = 3
	};

	// Simple emulation of 6 cascaded 74164 that drives the AAAADD BCD display elements, right to left
	class shift8
	{
	public:
		shift8(){ byte = 0; }
		void shiftIn(uint8_t in){ byte = ((byte << 1) & 0xfe) | (in & 1 ? 1 : 0); }
		uint8_t byte;
	};
	shift8 m_74164[6];

	required_device<ins8154_device> m_pia1;
	required_device<mm74c922_device> m_kb;
	uint8_t m_da;
};

void modulab_state::da_w(int state)
{
	LOG("--->%s()\n", FUNCNAME);
	m_da = state == CLEAR_LINE ? 0 : 1; // Capture data available signal
}

uint8_t modulab_state::io_r(offs_t offset)
{
	switch (offset)
	{
	case 3: // Poll Data available signal
		return m_da & 0x01; // Data Available signal gated by an 8097 hexbuffer to DB0
		break;
	case 2:
		LOG("--->%s Read Keyboard @ %04x\n", FUNCNAME, offset);
		return m_kb->read();
		break;
	default:
		LOG("--->%s BAD access @ %04x\n", FUNCNAME, offset);
		break;
	}
	return 0;
}

void modulab_state::io_w(offs_t offset, u8 data)
{
	LOG("--->%s()\n", FUNCNAME);
	uint8_t b = data & 1;
	switch (offset)
	{
	case DISPLAY:
		// Update the BCD elements with a data bit b shifted in right to left, CS is used as clock for all 164's
		for (int i = 0; i < 6; i++)
		{
			uint8_t c = (m_74164[i].byte & 0x80) ? 1 : 0; // Bit 7 is connected to the next BCD right to left
			m_74164[i].shiftIn(b);
			m_7segs[i] = ~m_74164[i].byte & 0x7f;  // Bit 0 to 6 drives the 7 seg display
			b = c; // bit 7 prior shift will be shifted in next (simultaneous in real life)
		}
		LOGDISPLAY("Shifted: %02x %02x %02x %02x %02x %02x\n",
				~m_74164[0].byte & 0x7f, ~m_74164[1].byte & 0x7f, ~m_74164[2].byte & 0x7f,
				~m_74164[3].byte & 0x7f, ~m_74164[4].byte & 0x7f, ~m_74164[5].byte & 0x7f);
		break;
	default:
		break;
	};
}

void modulab_state::machine_reset()
{
	LOG("--->%s()\n", FUNCNAME);

	m_maincpu->reset();
}

void modulab_state::machine_start()
{
	LOG("--->%s()\n", FUNCNAME);

	didact_state::machine_start();
	m_7segs.resolve();

	/* register for state saving */
	save_item(NAME(m_shift));
	save_item(NAME(m_reset));
}

// This address map is traced from pcb
void modulab_state::modulab_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().mirror(0xe000); // RAM0 always present 2114
	map(0x0400, 0x07ff).ram().mirror(0xe000); // RAM1 optional 2114
	// map(0x0800, 0x13ff).ram().mirror(0xe000); // expansion port area consisting of 3 chip selects each selecting 0x3ff byte addresses
	map(0x1400, 0x17ff).rom().mirror(0xe000).region("maincpu", 0x0000);
	map(0x1800, 0x187f).rw(FUNC(modulab_state::io_r), FUNC(modulab_state::io_w)).mirror(0xe200);
	map(0x1900, 0x197f).rw(m_pia1, FUNC(ins8154_device::read_io), FUNC(ins8154_device::write_io)).mirror(0xe200);
	map(0x1980, 0x19ff).rw(m_pia1, FUNC(ins8154_device::read_ram), FUNC(ins8154_device::write_ram)).mirror(0xe200);
	map(0x1c00, 0x1fff).rom().mirror(0xe000).region("maincpu", 0x0400);
}

//===================

static INPUT_PORTS_START( modulab )
	PORT_START("LINE0") // X1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("LINE1") // X2
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ADS") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("LINE2") // X3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FWD") PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("LINE3") // X4
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C/B") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("LINE4") /* Special KEY ROW for reset key */
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, modulab_state, trigger_reset, 0)
	PORT_BIT(0xfb, 0x00, IPT_UNUSED )
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
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, md6802_state, trigger_shift, 0)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, md6802_state, trigger_reset, 0)
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

	PORT_START("LINE4") /* Special KEY ROW for reset and Shift/'*' keys, they are hard wired */
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR('*') PORT_CHANGED_MEMBER(DEVICE_SELF, mp68a_state, trigger_shift, 0)
	//PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, mp68a_state, trigger_reset, 0)
	PORT_BIT(0xf3, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(didact_state::trigger_reset)
{
	if (newval == CLEAR_LINE)
	{
		LOGKBD("RESET is released, resetting the CPU\n");
		machine_reset();
		m_shift = 0;
		m_led = 0;
	}
}


void modulab_state::modulab(machine_config &config)
{
	m6802_cpu_device &maincpu(M6802(config, m_maincpu, XTAL(4'000'000)));
	maincpu.set_ram_enable(false); // Schematics holds RAM enable low so that the M6802 internal RAM is disabled.
	maincpu.set_addrmap(AS_PROGRAM, &modulab_state::modulab_map);
	config.set_default_layout(layout_modulab);

	/* Devices */
	MM74C923(config, m_kb, 0);
	m_kb->set_cap_osc(CAP_U(0.10));
	m_kb->set_cap_debounce(CAP_U(1));
	m_kb->da_wr_callback().set(FUNC(modulab_state::da_w));
	m_kb->x1_rd_callback().set_ioport("LINE0");
	m_kb->x2_rd_callback().set_ioport("LINE1");
	m_kb->x3_rd_callback().set_ioport("LINE2");
	m_kb->x4_rd_callback().set_ioport("LINE3");

	/* PIA #1 0x????-0x??? -  */
	INS8154(config, m_pia1);
	//m_ins8154->in_a().set(FUNC(modulab_state::ins8154_pa_r));
	//m_ins8154->out_a().set(FUNC(modulab_state::ins8154_pa_w));

	//RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}

void md6802_state::md6802(machine_config &config)
{
	m6802_cpu_device &maincpu(M6802(config, m_maincpu, XTAL(4'000'000)));
	maincpu.set_ram_enable(false);
	maincpu.set_addrmap(AS_PROGRAM, &md6802_state::md6802_map);
	config.set_default_layout(layout_md6802);

	/* Devices */
	TTL74145(config, m_tb16_74145, 0);
	/* PIA #1 0xA000-0xA003 - used differently by laborations and loaded software */
	PIA6821(config, m_pia1);

	/* PIA #2 Keyboard & Display 0xC000-0xC003 */
	PIA6821(config, m_pia2);
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
	m_pia2->writepa_handler().set(FUNC(md6802_state::pia2_kbA_w));
	m_pia2->readpa_handler().set(FUNC(md6802_state::pia2_kbA_r));
	m_pia2->writepb_handler().set(FUNC(md6802_state::pia2_kbB_w));
	m_pia2->readpb_handler().set(FUNC(md6802_state::pia2_kbB_r));
	m_pia2->ca2_handler().set(FUNC(md6802_state::pia2_ca2_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}

void mp68a_state::mp68a(machine_config &config)
{
	// Clock source is based on a N9602N Dual Retriggerable Resettable Monostable Multivibrator oscillator at aprox 505KHz.
	// Trimpot seems broken/stuck at 5K Ohm thu. ROM code 1Ms delay loops suggest 1MHz+
	M6800(config, m_maincpu, 505000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mp68a_state::mp68a_map);
	config.set_default_layout(layout_mp68a);

	/* Devices */
	/* PIA #1 0x500-0x503 - used differently by laborations and loaded software */
	PIA6820(config, m_pia1, 0);

	/* PIA #2 Keyboard & Display 0x600-0x603 */
	PIA6820(config, m_pia2, 0);
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
	m_pia2->writepa_handler().set(FUNC(mp68a_state::pia2_kbA_w));
	m_pia2->readpa_handler().set(FUNC(mp68a_state::pia2_kbA_r));
	m_pia2->writepb_handler().set(FUNC(mp68a_state::pia2_kbB_w));
	m_pia2->readpb_handler().set(FUNC(mp68a_state::pia2_kbB_r));
	m_pia2->readcb1_handler().set(FUNC(mp68a_state::pia2_cb1_r));
	m_pia2->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE); /* Not used by ROM. Combined trace to CPU IRQ with IRQB */
	m_pia2->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE); /* Not used by ROM. Combined trace to CPU IRQ with IRQA */

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
	DM9368(config, m_digits[0], 0).update_cb().set(FUNC(mp68a_state::digit_w<0>));
	DM9368(config, m_digits[1], 0).update_cb().set(FUNC(mp68a_state::digit_w<1>));
	DM9368(config, m_digits[2], 0).update_cb().set(FUNC(mp68a_state::digit_w<2>));
	DM9368(config, m_digits[3], 0).update_cb().set(FUNC(mp68a_state::digit_w<3>));
	DM9368(config, m_digits[4], 0).update_cb().set(FUNC(mp68a_state::digit_w<4>));
	DM9368(config, m_digits[5], 0).update_cb().set(FUNC(mp68a_state::digit_w<5>));

	/* Cassette */
	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

ROM_START( modulab )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_DEFAULT_BIOS("modulabvl")

	ROM_SYSTEM_BIOS(0, "modulabv1", "Modulab Version 1")
	ROMX_LOAD( "mlab1_00.bin", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "modulabv2", "Modulab Version 2")
	ROMX_LOAD( "mlab2_00.bin", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "modulabvl", "Modulab Prototype")
	ROMX_LOAD( "modulab_levererad.bin", 0x0000, 0x0800, CRC(40774ef4) SHA1(9cf188342993fbcff13dbbecc62d1ee49010d6f4), ROM_BIOS(2) )
ROM_END

// TODO split ROM image into proper ROM set
ROM_START( md6802 ) // ROM image from http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=135#p1203640
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "didact.bin", 0xe000, 0x0800, CRC(50430b1d) SHA1(8e2172a9ae95b04f20aa14177df2463a286c8465) )
ROM_END

ROM_START( mp68a ) // ROM image from http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=135#p1203640
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "didacta.bin", 0x0800, 0x0200, CRC(aa05e1ce) SHA1(9ce8223efd274045b43ceca3529e037e16e99fdf) )
	ROM_LOAD( "didactb.bin", 0x0a00, 0x0200, CRC(592898dc) SHA1(2962f4817712cae97f3ab37b088fc73e66535ff8) )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME           FLAGS
COMP( 1979, mp68a,   0,      0,      mp68a,   mp68a,   mp68a_state,   empty_init, "Didact AB",          "mp68a",           MACHINE_NO_SOUND_HW )
COMP( 1983, md6802,  0,      0,      md6802,  md6802,  md6802_state,  empty_init, "Didact AB",          "Mikrodator 6802", MACHINE_NO_SOUND_HW )
COMP( 1984, modulab, 0,      0,      modulab, modulab, modulab_state, empty_init, "Esselte Studium AB", "Modulab",         MACHINE_NO_SOUND_HW )
