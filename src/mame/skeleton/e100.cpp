// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 *
 * History of Didact
 *------------------
 * See didact.cpp
 *
 * The Esselte 100 was an original design with a CRT and a full Keyboard that also had a BASIC interpreter
 * extended with commands suitable for educational experiments using the exapansion bus and its built in
 * io control capabilities.
 *
 * The Esselte 1000 was an educational package based on Apple II plus software and literature but the relation
 * to Didact is at this point unknown so it is probably a pure Esselte software production. If this branded
 * distribution is recovered it will be added as a clone of the Apple II driver or just as softlist item.
 *
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 * http://elektronikforumet.com/forum/download/file.php?id=63988&mode=view
 * http://elektronikforumet.com/forum/viewtopic.php?f=2&t=79576&start=150#p1203915
 *
 *  TODO:
 * -------------------------------
 *  - Dump more ROM:s
 *  - Keyboard for early rev PCB
 *  - Expansion bus
 *  - Expansion overlay
 *  - Serial
 ****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/74145.h"
#include "machine/timer.h"

// Features
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "speaker.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_SETUP   (1U << 1)
#define LOG_SCAN    (1U << 2)
#define LOG_BANK    (1U << 3)
#define LOG_SCREEN  (1U << 4)
#define LOG_READ    (1U << 5)
#define LOG_CS      (1U << 6)

//#define VERBOSE (LOG_READ | LOG_GENERAL | LOG_SETUP | LOG_BANK)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGSCAN(...)    LOGMASKED(LOG_SCAN,    __VA_ARGS__)
#define LOGBANK(...)    LOGMASKED(LOG_BANK,    __VA_ARGS__)
#define LOGSCREEN(...)  LOGMASKED(LOG_SCREEN,  __VA_ARGS__)
#define LOGR(...)       LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGCS(...)      LOGMASKED(LOG_CS,      __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define PIA1_TAG "pia1"
#define PIA2_TAG "pia2"

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
 * rev2 board had 4Kb more ROM memory, 2 x 2764 instead of the 6 x 2716 (note the rev1 piggyback on rightmost 2716) with funny address decoding.
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


 namespace {

/* Esselte 100 driver class */
class e100_state : public driver_device
{
public:
	e100_state(const machine_config &mconfig, device_type type, const char * tag)
		: driver_device(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_kbd_74145(*this, "kbd_74145")
		,m_vram(*this, "vram")
		,m_cassette(*this, "cassette")
		,m_rs232(*this, "rs232")
		,m_chargen(*this, "chargen")
		,m_io_line0(*this, "LINE0")
		,m_io_line1(*this, "LINE1")
		,m_io_line2(*this, "LINE2")
		,m_io_line3(*this, "LINE3")
		,m_io_line4(*this, "LINE4")
		,m_io_line5(*this, "LINE5")
		,m_io_line6(*this, "LINE6")
		,m_io_line7(*this, "LINE7")
		,m_io_line8(*this, "LINE8")
		,m_io_line9(*this, "LINE9")
		,m_pia1(*this, PIA1_TAG)
		,m_pia2(*this, PIA2_TAG)
		,m_pia1_B(0)
		,m_50hz(0)
		{ }

	void e100(machine_config &config);

private:
	required_device<m6802_cpu_device> m_maincpu;
	required_device<ttl74145_device> m_kbd_74145;
	required_shared_ptr<uint8_t> m_vram;
	required_device<cassette_image_device> m_cassette;
	optional_device<rs232_port_device> m_rs232;
	required_region_ptr<uint8_t> m_chargen;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_reset() override { m_maincpu->reset(); LOG("--->%s()\n", FUNCNAME); };
	virtual void machine_start() override ATTR_COLD;
	uint8_t pia_r(offs_t offset);
	void pia_w(offs_t offset, uint8_t data);
	uint8_t pia1_kbA_r();
	void pia1_kbA_w(uint8_t data);
	uint8_t pia1_kbB_r();
	void pia1_kbB_w(uint8_t data);
	void pia1_ca2_w(int state);
	void pia1_cb2_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(rtc_w);
	void e100_map(address_map &map) ATTR_COLD;

	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_line5;
	required_ioport m_io_line6;
	required_ioport m_io_line7;
	required_ioport m_io_line8;
	required_ioport m_io_line9;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	uint8_t m_pia1_B;
	uint8_t m_50hz;
};

TIMER_DEVICE_CALLBACK_MEMBER(e100_state::rtc_w)
{
	m_pia2->ca1_w((m_50hz++ & 1));
}

void e100_state::machine_start()
{
	LOG("%s()\n", FUNCNAME);

	/* register for state saving */
	save_item(NAME(m_50hz));
	save_item(NAME(m_pia1_B));
}

uint32_t e100_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int vramad;
	uint8_t *chardata;
	uint8_t charcode;

	LOGSCREEN("%s()\n", FUNCNAME);
	vramad = 0;
	for (int row = 0; row < 32 * 8; row += 8)
	{
		for (int col = 0; col < 32 * 8; col += 8)
		{
			/* look up the character data */
			charcode = m_vram[vramad];
			if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n %c at X=%d Y=%d: ", charcode, col, row);
			chardata = &m_chargen[(charcode * 8)];
			/* plot the character */
			for (y = 0; y < 8; y++)
			{
				if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n  %02x: ", *chardata);
				for (x = 0; x < 8; x++)
				{
					if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(" %02x: ", *chardata);
					bitmap.pix(row + y, col + x) = (*chardata & (1 << x)) ? 1 : 0;
				}
				chardata++;
			}
			vramad++;
		}
		if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n");
	}

	return 0;
}

/* PIA write - the Esselte 100 allows the PIA:s to be accessed simultaneously */
void e100_state::pia_w(offs_t offset, uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
	if ((offset & 0x08) == 0x08)
	{
		LOG("- PIA1\n");
		m_pia1->write(offset, data);
	}
	if ((offset & 0x10) == 0x10)
	{
		LOG("- PIA2\n");
		m_pia2->write(offset, data);
	}
	if (VERBOSE && (offset & 0x18) == 0x18)
	{
		LOGCS("- Dual device write access!\n");
	}
	if (VERBOSE && (offset & 0x18) == 0x00)
	{
		logerror("- Funny write at offset %02x!\n", offset);
	}
}

/* PIA read  - the Esselte 100 allows the PIA:s to be accessed simultaneously */
uint8_t e100_state::pia_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x18)
	{
	case 0x18: // read PIA1 and PIA2 at the same time, should really only happen for writes...
		{
			uint8_t data1 =  m_pia1->read(offset);
			uint8_t data2 =  m_pia2->read(offset);
			logerror("%s: Dual device read may have caused unpredictable results on real hardware\n", FUNCNAME);
			data = data1 & data2; // We assume that the stable behaviour is that data lines with a low level by either device succeeds
			LOGCS("%s %s[%02x] %02x & %02x -> %02x Dual device read!!\n", PIA1_TAG "/" PIA2_TAG, FUNCNAME, offset, data1, data2, data);
		}
		break;
	case 0x08: // PIA1
		data = m_pia1->read(offset);
		LOGCS("%s %s(%02x)\n", PIA1_TAG, FUNCNAME, data);
		break;
	case 0x10: // PIA2
		data = m_pia2->read(offset);
		LOGCS("%s %s(%02x)\n", PIA2_TAG, FUNCNAME, data);
		break;
	default: // None of the devices are selected
		logerror("%s: Funny read at offset %02x\n", FUNCNAME, offset);
	}
	return data;
}

void e100_state::pia1_kbA_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
}

uint8_t e100_state::pia1_kbA_r()
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
	if (VERBOSE && ls145 && pa) LOGSCAN("%s  [%03x]%04x\n", FUNCNAME, ls145, pa);

	return ~pa;
}

/*
  PB0-PB3 is connected to U601 (74LS145) which select a column to scan
  PB4-PB5 together with CA1, CA2, CB1 and CB2 are used for the printer interface
  PB6-PB7 forms the cassette interface

  The serial bitbanging performs unreliably atm, can be poor original code or inexact CPU timing.
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
void e100_state::pia1_kbB_w(uint8_t data)
{
	uint8_t col;

	// Keyboard
	//  if (VERBOSE && data != m_pia1_B) LOGSCAN("%s(%02x)\n", FUNCNAME, data);
	m_pia1_B = data;
	col = data & 0x0f;
	m_kbd_74145->write( col );

	// Cassette
	m_cassette->output(data & CASS_OUT ? 1.0 : -1.0);

	// Serial
	m_rs232->write_txd(data & SERIAL_OUT ? 0 : 1);
}

uint8_t e100_state::pia1_kbB_r()
{
	m_pia1_B &= ~(CASS_IN|SERIAL_IN);

	m_pia1_B |= (m_cassette->input() > 0.03 ? CASS_IN : 0x00);

	m_pia1_B |= (m_rs232->rxd_r() != 0 ? SERIAL_IN : 0x00);

	return m_pia1_B;
}

void e100_state::pia1_ca2_w(int state)
{
	// TODO: Make this a slot device to trigger time meassurements
}

void e100_state::pia1_cb2_w(int state)
{
	m_rs232->write_txd(!state);
}

// This map is derived from info in "TEMAL 100 - teknisk manual Esselte 100"
void e100_state::e100_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x8000, 0x87ff).rom().region("roms", 0);
	map(0xc000, 0xc3ff).ram().share(m_vram);
	map(0xc800, 0xc81f).rw(FUNC(e100_state::pia_r), FUNC(e100_state::pia_w)).mirror(0x07e0);
	map(0xd000, 0xffff).rom().region("roms", 0x1000);
}

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

void e100_state::e100(machine_config &config)
{
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &e100_state::e100_map);

	/* Devices */
	TTL74145(config, m_kbd_74145, 0);

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
	PIA6821(config, m_pia1);
	m_pia1->writepa_handler().set(FUNC(e100_state::pia1_kbA_w));
	m_pia1->readpa_handler().set(FUNC(e100_state::pia1_kbA_r));
	m_pia1->writepb_handler().set(FUNC(e100_state::pia1_kbB_w));
	m_pia1->readpb_handler().set(FUNC(e100_state::pia1_kbB_r));
	m_pia1->ca1_w(ASSERT_LINE); // TODO: Make this a slot device for time meassurements. Default is handshake for serial port TODO: Fix RS 232 handshake as default
	m_pia1->ca2_handler().set(FUNC(e100_state::pia1_ca2_w));
	m_pia1->cb2_handler().set(FUNC(e100_state::pia1_cb2_w));

	/* The optional second PIA enables the expansion port on CA1 and a software RTC with 50Hz resolution */
	PIA6821(config, m_pia2);
	m_pia2->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	/* Serial port support */
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_pia1, FUNC(pia6821_device::cb1_w));

	SPEAKER(config, "mono").front_center();
	/* Cassette support - E100 uses 300 baud Kansas City Standard with 1200/2400 Hz modulation */
	/* NOTE on usage: mame e100 -window -cass <wav file> -ui_active
	 * Once running enable/disable internal UI by pressing Scroll Lock in case it interferes with target keys
	 * Open the internal UI by pressing TAB and then select 'Tape Control' or use F2/Shift F2 for PLAY/PAUSE
	 * In order to use a wav file it has first to be created using TAB and select the 'File manager'
	 * Once created it may be given on the commandline or mounted via TAB and select
	 * E100 supports cassette through the 'LOAD' and 'SAVE' commands with no arguments
	 */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* screen TODO: simplify the screen config, look at zx.cpp */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(4'000'000)/2, 265, 0, 265, 265, 0, 265);
	screen.set_screen_update(FUNC(e100_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* There is a 50Hz signal from the video circuit to CA1 which generates interrupts and drives a software RTC */
	TIMER(config, "video50hz").configure_periodic(FUNC(e100_state::rtc_w), attotime::from_hz(100)); /* Will be divided by two through toggle in the handler */
}

/* ROM sets from Didact was not versioned in general, so the numbering are just assumptions */
ROM_START( e100 )
	ROM_REGION(0x4000, "roms", 0)
	ROM_DEFAULT_BIOS("rev2-basic")

	/* TODO: Get the original ROMs */
	ROM_SYSTEM_BIOS(0, "rev1-basic", "Esselte 100 rev1 BASIC")
	ROMX_LOAD( "e100r1u201.bin", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "e100r1u202.bin", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "e100r1u203.bin", 0x2000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "e100r1u204.bin", 0x2800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "e100r1u205.bin", 0x3000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "e100r1u206.bin", 0x3800, 0x0800, NO_DUMP, ROM_BIOS(0) )

	/* This is a prototype ROM, commercial relase not verified. The prototype also have different keyboard and supports
	   more ram so might need to be split out as a clone later */
	ROM_SYSTEM_BIOS(1, "rev2-basic", "Esselte 100 rev2 BASIC")
	ROMX_LOAD( "e100r2u201.bin", 0x0000, 0x2000, CRC(53513b67) SHA1(a91c5c32aead82dcc87db5d818ff286a7fc6a5c8), ROM_BIOS(1) )
	ROMX_LOAD( "e100r2u202.bin", 0x2000, 0x2000, CRC(eab3adf2) SHA1(ff3f5f5c8ea8732702a39cff76d0706ab6b751ee), ROM_BIOS(1) )

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "e100u506.bin", 0x0000, 0x0800, CRC(fff9f288) SHA1(2dfb3eb551fe1ef67da328f61ef51ae8d1abdfb8) )
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY      FULLNAME       FLAGS
COMP( 1982, e100, 0,      0,      e100,    e100,  e100_state, empty_init, "Didact AB", "Esselte 100", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE)
