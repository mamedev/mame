// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 *
 * History of Candela Data AB
 *---------------------------
 * The Candela computer was designed to be the big breakthough and developed by Candela Data AB, "a Didact Company".
 * The Candela system was based around a main unit that could run OS-9 or Flex and a terminal unit that had a
 * proprietary software including CDBASIC. The Candela system lost the battle of the Swedish schools to
 * the Compis computer by TeleNova which was based on CP/M initially.  Later both lost to IBM PC as we know.
 * Candela Data continued to sell their system to the swedish industry without major successes despite great
 * innovation and spririt.
 *
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 * http://frakaday.blogspot.com/p/candela.html
 * http://elektronikforumet.com/forum/viewtopic.php?f=11&t=51424
 *
 *  TODO:
 *  Candela designs:   can09t, can09
 * -------------------------------------
 *  - Add PCB layouts   OK
 *  - Dump ROM:s,       OK      OK
 *  - Keyboard
 *  - Display/CRT
 *  - Clickable Artwork
 *  - Sound
 *  - Cassette i/f
 *  - Expansion bus
 *  - Expansion overlay
 *  - Interrupts
 *  - Serial            OK
 ****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

// Features
#include "imagedev/cassette.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_SETUP   (1U << 1)
#define LOG_SCAN    (1U << 2)
#define LOG_BANK    (1U << 3)
#define LOG_SCREEN  (1U << 4)
#define LOG_READ    (1U << 5)
#define LOG_CS      (1U << 6)
#define LOG_PLA     (1U << 7)
#define LOG_PROM    (1U << 8)

//#define VERBOSE (LOG_READ | LOG_GENERAL | LOG_SETUP | LOG_PLA | LOG_BANK)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGSCAN(...)    LOGMASKED(LOG_SCAN,    __VA_ARGS__)
#define LOGBANK(...)    LOGMASKED(LOG_BANK,    __VA_ARGS__)
#define LOGSCREEN(...)  LOGMASKED(LOG_SCREEN,  __VA_ARGS__)
#define LOGR(...)       LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGCS(...)      LOGMASKED(LOG_CS,      __VA_ARGS__)
#define LOGPLA(...)     LOGMASKED(LOG_PLA,     __VA_ARGS__)
#define LOGPROM(...)    LOGMASKED(LOG_PROM,    __VA_ARGS__)

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
 * Candela CAN09 SBC
 * TODO:
 * - Map additional PIA:s on the ROM board
 * - Vram and screen
 * - Keyboard
 * - LCD
 * - AD/DA support for related BASIC functions
 * - Promett rom cartridge support
 */

#define SYSPIA_TAG PIA1_TAG
#define USRPIA_TAG PIA2_TAG

// Start offset for each device segment in bank array
#define RAM0 0x00
#define RAM1 0x08
#define PROM 0x10
#define RAM2 0x18
#define IO   0x1a

/* Candela CAN09 driver class */
  class can09t_state :  public driver_device
{
public:
	can09t_state(const machine_config &mconfig, device_type type, const char * tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_syspia(*this, SYSPIA_TAG)
		, m_usrpia(*this, USRPIA_TAG)
		, m_pia3(*this, PIA3_TAG)
		, m_pia4(*this, PIA4_TAG)
		, m_ptm(*this, "ptm")
		, m_acia(*this, "acia")
		, m_cass(*this, "cassette")
		, m_banksel(1)
	{ }
	required_device<cpu_device> m_maincpu;
	virtual void machine_start() override ATTR_COLD;
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t syspia_A_r();
	uint8_t syspia_B_r();
	void syspia_B_w(uint8_t data);
	void syspia_cb2_w(int state);
	void usrpia_cb2_w(int state);
	void write_acia_clock(int state);
	void can09t(machine_config &config);
	void can09t_map(address_map &map) ATTR_COLD;
protected:
	required_device<pia6821_device> m_syspia;
	required_device<pia6821_device> m_usrpia;
	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;
	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;

	uint8_t m_banksel = 0;
	uint8_t *m_plap = nullptr;
	uint8_t *m_epromp = nullptr;

	uint8_t m_ram0[1024 *  8]; // IC3
	uint8_t m_ram1[1024 * 32]; // IC4
	uint8_t m_ram2[1024 *  8]; // IC5
private:
	enum {
		PLA_EPROM = 0x80,
		PLA_IO1   = 0x40,
		PLA_RAM2  = 0x20,
		PLA_RAM1  = 0x10,
		PLA_RAM0  = 0x08,
		PLA_RAM13 = 0x04,
		PLA_RAM14 = 0x02,
		PLA_MPX   = 0x01
	};

	enum {
		CPU_A4 = 0x010,
		CPU_A5 = 0x020,
		CPU_A6 = 0x040,
		CPU_A7 = 0x080,
		CPU_A8 = 0x100,
		CPU_A9 = 0x200,
	};

	enum { // 74138 outputs, naming after schematics
		X0XX = 0x0000,
		X1XX = 0x0010,
		X2XX = 0x0020,
		X3XX = 0x0030,
		X4XX = 0x0080,
		X5XX = 0x0090,
		X6XX = 0x00a0,
		X7XX = 0x00b0,
	};
};

void can09t_state::machine_start()
{
	LOG("%s()\n", FUNCNAME);

	/* TBP18S030 32 bytes fuse PROM controls the entire memory map in two parts/map-banks
	   BANK 0: 6f 6f 69 69 7b 7b ee ee e8 e8 ec bc ea ea 7f 7f
	   BANK 1: f1 f1 f5 f5 f3 f3 f7 f7 dc dc ec bc ea ea 7f 7f

	   Connected as follows:
	                  A12 -- A0  Y0 -- MPX* - latch for screen RAM
	                  A13 -- A1  Y1 -- RAM14
	                  A14 -- A2  Y2 -- RAM13
	                  A15 -- A3  Y3 -- RAM0
	   SYSPIA PB5 -- BANK -- A4  Y4 -- RAM1
	                 E+Q  -- S*  Y5 -- RAM2
	                             Y6 -- IO1
	                             Y7 -- EPROM

	   The PAL outputs are used as follows:
	   --------------------------------
	   Y0 IC12 MUX 74245: D0-D7 + MPX + RW => BD0-BD7
	   Y1 RAM14 is used together with the other outputs
	   Y2 RAM13 is used together with the other outputs
	   Y3 IC3 SRAM  6264: A0-A12 + RAM13:CS2 + RAM14:NC + RAM0:OE:CE + jumper B2 RW/pulled high:RW + VCC:BATTERY
	   Y4 IC4 SRAM 62256: A0-A12 + RAM13 + RAM14 + RW + RAM1:OE:CE + BD0-BD7
	   Y5 IC5 SRAM  6264: A0-A12 + RAM13:CS2 + RAM14:NC + RW + RAM2:OE:CE + BD0-BD7
	   Y6 IC11 DMX 74138: A4:A A5:B A7:C A8:G1 IO1:G2A A9:G2B => x0xx-x7xx see bellow:
	   Y7 IC2 PROM 27256: A0-A14 + EPROM

	   The 74138 demuxer outputs are used as follows:
	    A B C (A4 A5 and A7 respectivelly, not A6!)
	   ----------------------------------------------
	    0 0 0 Y0 x0xx ACIA   0xB100-B101
	    0 0 1 Y1 x1xx SYSPIA 0xB110-B113
	    0 1 0 Y2 x2xx USRPIA 0xB120-B123
	    0 1 1 Y3 x3xx PTM    0xB130-B137
	    1 0 0 Y4 x4xx        0xB180-B18F
	    1 0 1 Y5 x5xx J1, J2 0xB190-B19F
	    1 1 0 Y6 x6xx
	    1 1 1 Y7 x7xx J1, J2 0xB1B0-B1BF
	 */
	m_plap = (uint8_t*)(memregion ("plas")->base ());
	m_epromp = (uint8_t*)(memregion ("roms")->base ());
	memset(m_ram0, 0, sizeof(m_ram0));
	memset(m_ram1, 0, sizeof(m_ram1));
	memset(m_ram2, 0, sizeof(m_ram2));
};

uint8_t can09t_state::read(offs_t offset)
{
	LOG("%s %02x\n", FUNCNAME, offset);
	uint8_t pla_offset = 0;
	uint8_t pla_data = 0;
	uint8_t byte = 0;

	// Form the offset into the PLA (see above for explanation on PLA hookup)
	pla_offset = ((offset >> 12) & 0x0f) | (m_banksel ? 0x10 : 0x00);

	// Get the PLA data byte
	pla_data = m_plap[pla_offset & 0x1f];
	LOGPLA("PLA[%02x] read: %02x Bank%d\n", pla_offset, pla_data, m_banksel != 0 ? 0 : 1);

	// Find the device
	// IC2 EPROM - PAL Y7 PROM 27256: A0-A14 + EPROM
	if (~pla_data & PLA_EPROM)
	{
		byte = m_epromp[(offset & 0x7fff) % memregion("roms")->bytes()];
		LOGPROM("- EPROM %04x[%04x]->%02x\n", offset, (offset & 0x7fff) % memregion("roms")->bytes(), byte);
	}

	// IC3 RAM0 - PAL Y3 SRAM 6264: A0-A12 + RAM13:1 (RAM14 connected to NC)
	if ( (~pla_data & PLA_RAM0) && (pla_data & PLA_RAM13))
	{
		byte = m_ram0[offset % sizeof(m_ram0)];
		LOGPLA("- RAM0 %04x->%02x\n", offset, byte);
	}

	// IC4 RAM1 PAL Y4 SRAM 62256: A0-A12 + banked with RAM13/RAM14 data buffer 74LS245 enabled through PAL Y0
	if ( (~pla_data & PLA_RAM1) && (~pla_data & PLA_MPX) )
	{
		uint16_t ofs = (offset & 0x1fff) | ((pla_data & PLA_RAM13) ? 0x2000 : 0) | ((pla_data & PLA_RAM14) ? 0x4000 : 0);
		byte = m_ram1[ofs % sizeof(m_ram1)];
		LOGPLA("- RAM1 %04x->%02x\n", ofs, byte);
	}

	// IC5 RAM2 - PAL Y5 SRAM 6264: A0-A12 + RAM13:1 (RAM14 connected to NC) data buffer 74LS245 enabled through PAL Y0
	if ( (~pla_data & PLA_RAM0) && (pla_data & PLA_RAM13) && (~pla_data & PLA_MPX) )
	{
		byte = m_ram2[offset % sizeof(m_ram2)];
		LOGPLA("- RAM2 %04x->%02x\n", offset, byte);
	}

	// IC11 74138 Y6 demultiplexer : A5, A5, A7 + A8:1 + A9:0 maps in the i/o area at some
	if ( (~pla_data & PLA_IO1) && (~pla_data & PLA_MPX) && (offset & CPU_A8) && (~offset & CPU_A9) )
	{
		switch (offset & 0x00f0)
		{
		case X0XX: // ACIA
			LOGPLA("-- ACIA\n");
			byte = m_acia->read(offset & 1);
			break;
		case X1XX: // SYSPIA
			LOGPLA("-- SYSPIA\n");
			byte = m_syspia->read_alt(offset & 3);
			break;
		case X2XX: // USRPIA
			LOGPLA("-- USRPIA\n");
			byte = m_usrpia->read_alt(offset & 3);
			break;
		case X3XX: // PTM
			LOGPLA("-- PTM\n");
			byte = m_ptm->read(offset & 7);
			break;
		case X4XX: //
			LOGPLA("-- XX4X\n");
			break;
		case X5XX: // J1, J2
			LOGPLA("-- XX5X\n");
			break;
		case X6XX: //
			LOGPLA("-- XX6X\n");
			break;
		case X7XX: // J1, J2
			LOGPLA("-- XX7X\n");
			break;
		}
	}
	LOGR("- %04x <- %02x\n\n", offset, byte);
	return byte;
}

void can09t_state::write(offs_t offset, uint8_t data)
{
	LOG("%s() %04x : %02x\n", FUNCNAME, offset, data);
	uint8_t pla_offset = 0;
	uint8_t pla_data = 0;

	// Form the offset into the PLA (see above for explanation on PLA hookup)
	pla_offset = ((offset >> 12) & 0x0f) | (m_banksel ? 0x10 : 0x00);

	// Get the PLA data byte
	pla_data = m_plap[pla_offset & 0x1f];
	LOGPLA("PLA[%02x] write: %02x Bank%d\n", pla_offset, pla_data, m_banksel != 0 ? 0 : 1);

	// Find the device
	// IC2 EPROM - PAL Y7 PROM 27256: A0-A14 + EPROM*
	if (~pla_data & PLA_EPROM)
	{
		LOGPLA("- EPROM (ignored) %04x<-%02x\n", offset, data);
	}

	// IC3 RAM0 - PAL Y3 SRAM 6264: A0-A12 + RAM13:1 (RAM14 connected to NC)
	if ( (~pla_data & PLA_RAM0) && (pla_data & PLA_RAM13))
	{
		m_ram0[offset % sizeof(m_ram0)] = data;
		LOGPLA("- RAM0 %04x<-%02x\n", offset, data);
	}

	// IC4 RAM1 Y4 SRAM 62256: A0-A12 + banked with RAM13/RAM14 data buffer 74LS245 enabled through PAL Y0
	if ( (~pla_data & PLA_RAM1) && (~pla_data & PLA_MPX) )
	{
		uint16_t ofs = (offset & 0x1fff) | ((pla_data & PLA_RAM13) ? 0x2000 : 0) | ((pla_data & PLA_RAM14) ? 0x4000 : 0);
		m_ram1[ofs % sizeof(m_ram1)] = data;
		LOGPLA("- RAM1 %04x<-%02x\n", ofs, data);
	}

	// IC5 RAM2 - PAL Y5 SRAM 6264: A0-A12 + RAM13:1 (RAM14 connected to NC) data buffer 74LS245 enabled through PAL Y0
	if ( (~pla_data & PLA_RAM0) && (pla_data & PLA_RAM13) && (~pla_data & PLA_MPX) )
	{
		m_ram2[offset % sizeof(m_ram2)] = data;
		LOGPLA("- RAM2 %04x->%02x\n", offset, data);
	}

	// IC11 74138 Y6 demultiplexer : A5, A5, A7 + A8:1 + A9:0 maps in the i/o area at some
	if ( (~pla_data & PLA_IO1) && (~pla_data & PLA_MPX) && (offset & CPU_A8) && (~offset & CPU_A9) )
	{
		LOGPLA("- IO1 %04x<-%02x\n", offset, data);
		switch (offset & 0x00f0)
		{
		case X0XX: // ACIA
			LOGPLA("-- ACIA\n");
			m_acia->write(offset & 1, data);
			break;
		case X1XX: // SYSPIA
			LOGPLA("-- SYSPIA\n");
			m_syspia->write_alt(offset & 3, data);
			break;
		case X2XX: // USRPIA
			LOGPLA("-- USRPIA\n");
			m_usrpia->write_alt(offset & 3, data);
			break;
		case X3XX: // PTM
			LOGPLA("-- PTM\n");
			m_ptm->write(offset & 7, data);
			break;
		case X4XX: //
			LOGPLA("-- XX4X\n");
			break;
		case X5XX: // J1, J2
			LOGPLA("-- XX5X\n");
			break;
		case X6XX: //
			LOGPLA("-- XX6X\n");
			break;
		case X7XX: // J1, J2
			LOGPLA("-- XX7X\n");
			break;
		}
	}
}

uint8_t can09t_state::syspia_A_r()
{
	LOG("%s()\n", FUNCNAME);
	return 0;
}

uint8_t can09t_state::syspia_B_r()
{
	LOG("%s()\n", FUNCNAME);
	u8 data = (m_cass->input() > 0.04) ? 0x80 : 0;
	return data;
}

void can09t_state::syspia_B_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);

	m_banksel = (data & 0x20) ? 0x10 : 0;
	LOGBANK("Bank select: %d", (m_banksel >> 4) & 1);
	m_cass->output(BIT(data, 6) ? 1.0 : -1.0);
}

void can09t_state::syspia_cb2_w(int state)
{
	LOG("%s(%02x)\n", FUNCNAME, state);
}

void can09t_state::usrpia_cb2_w(int state)
{
	LOG("%s(%02x)\n", FUNCNAME, state);
}

void can09t_state::write_acia_clock(int state)
{
		m_acia->write_txc (state);
		m_acia->write_rxc (state);
}

/*
 * Address map from documentation based on specific PAL
 *
 *   BANK 0                            BANK 1
 *   0x0000-0x34ff PROM VDU-program    0x0000-0x34ff RAM0 user applications (0)
 *   0x3500-0x5fff PROM simulator      0x3500-0x5fff RAM0 user applications (0)
 *   0x6000-0x7fff RAM screen memory   0x6000-0x7fff RAM0 user applications (0)
 *   0x8000-0x9fff RAM screen memory   0x8000-0x9fff PROM user applications (2)
 *  *0xa000-0xafff RAM Stack, system   0xa000-0xafff RAM Stack, system
 *  *0xb000-0xb0ff Free                0xb000-0xb0ff Free
 *  *0xb100-0xb101 ACIA                0xb100-0xb101 ACIA
 *  *0xb110-0xb113 PIA System          0xb110-0xb113 PIA System
 *  *0xb120-0xb123 PIA User apps       0xb120-0xb123 PIA User apps
 *  *0xb130-0xb137 PTM                 0xb130-0xb137 PTM
 *  *0xb180-0xb1bf Free                0xb180-0xb1bf Free
 *  *0xb200-0xb3ff Free                0xb200-0xb3ff Free
 *  *0xc000-0xdfff RAM User apps (1)   0xc000-0xdfff RAM User apps (1)
 *  *0xe000-0xffff PROM monitor        0xe000-0xffff PROM monitor
 */

void can09t_state::can09t_map(address_map &map)
{
// Everything is dynamically and asymetrically mapped through the PAL decoded by read/write
	map(0x0000, 0xffff).rw(FUNC(can09t_state::read), FUNC(can09t_state::write));
}

static INPUT_PORTS_START( can09t )
INPUT_PORTS_END

/*
 * Candela Main Unit
 * TODO:
 * - Map PIA:S
 * - ROM/RAM paging by using the PIAs and the myriad of 74138s on the board
 * - Vram and screen for the 6845 CRTC
 * - Check actual clock source for CRTC. An 8MHz UKI crystal is also nearby
 * - Keyboard
 * - Serial port
 * - Floppy controller
 * - Split out commonalities in a candela_state base class for can09 and can09t
 */
/* Candela main unit driver class */
  class can09_state :  public driver_device
{
public:
	can09_state(const machine_config &mconfig, device_type type, const char * tag)
		: driver_device(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		,m_pia1(*this, PIA1_TAG)
		,m_ram(*this, RAM_TAG)
		,m_bank1(*this, "bank1")
		,m_crtc(*this, "crtc")
	{ }

	void can09(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint8_t pia1_A_r();
	void pia1_A_w(uint8_t data);
	uint8_t pia1_B_r();
	void pia1_B_w(uint8_t data);
	void pia1_cb2_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void can09_map(address_map &map) ATTR_COLD;
	required_device<pia6821_device> m_pia1;
	required_device<ram_device> m_ram;
	required_memory_bank m_bank1;
	required_device<hd6845s_device> m_crtc;
};

void can09_state::machine_reset()
{
	LOG("%s()\n", FUNCNAME);
	m_bank1->set_entry(0);
}

void can09_state::machine_start()
{
	LOG("%s()\n", FUNCNAME);
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

	LOGSCREEN("%s()\n", FUNCNAME);
	//  vramad = 0;
	for (int row = 0; row < 72 * 8; row += 8)
	{
		for (int col = 0; col < 64 * 8; col += 8)
		{
#if 0
			/* look up the character data */
			charcode = m_vram[vramad];
			if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n %c at X=%d Y=%d: ", charcode, col, row);
			chardata = &m_char_ptr[(charcode * 8)];
#endif
			/* plot the character */
			for (y = 0; y < 8; y++)
			{
				//              if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n  %02x: ", *chardata);
				for (x = 0; x < 8; x++)
				{
					//                  if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(" %02x: ", *chardata);
					bitmap.pix(row + y, col + x) = x & 1; //(*chardata & (1 << x)) ? 1 : 0;
				}
				//              chardata++;
			}
			//          vramad++;
		}
		//      if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN("\n");
	}

	return 0;
}

uint8_t can09_state::pia1_A_r()
{
	LOG("%s()\n", FUNCNAME);
	return 0x40;
}

void can09_state::pia1_A_w(uint8_t data)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
}

uint8_t can09_state::pia1_B_r()
{
	LOG("%s()\n", FUNCNAME);
	return 0;
}

void can09_state::pia1_B_w(uint8_t data)
{
	//  UINT8 *RAM = memregion("maincpu")->base();
	LOG("%s(%02x)\n", FUNCNAME, data);
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

void can09_state::pia1_cb2_w(int state)
{
	LOG("%s(%02x)\n", FUNCNAME, state);
}

static INPUT_PORTS_START( can09 )
	PORT_START("LINE0")
	PORT_START("LINE1")
	PORT_START("LINE2")
	PORT_START("LINE3")
	PORT_START("LINE4")
INPUT_PORTS_END

// traced and guessed from pcb images and debugger
// It is very likely that this is a PIA based dynamic address map, needs more analysis
void can09_state::can09_map(address_map &map)
{
/*
 * Port A=0x18 B=0x20 erase 0-7fff
 * Port A=0x18 B=0x30 erase 0-7fff
 * Port A=0x18 B=0x00
 * Port A=0x10 B=
*/
//  map(0x0000, 0x7fff).ram();
	map(0x0000, 0x7fff).ram().bankrw("bank1");
	map(0xe000, 0xffff).rom().region("roms", 0);
	map(0xe020, 0xe020).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0xe021, 0xe021).w(m_crtc, FUNC(hd6845s_device::register_w));
	map(0xe034, 0xe037).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));

#if 0
	map(0xb100, 0xb101).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xb110, 0xb113).rw(m_pia1, FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xb120, 0xb123).rw(PIA2_TAG, FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xb130, 0xb137).rw("ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xb200, 0xc1ff).rom().region("roms", 0x3200);
	map(0xc200, 0xdfff).ram(); /* Needed for BASIC etc */
#endif
}

#ifdef UNUSED_VARIABLE
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END
#endif

/* Fake clock values until we TODO: figure out how the PTM generates the clocks */
#define CAN09T_BAUDGEN_CLOCK 1.8432_MHz_XTAL
#define CAN09T_ACIA_CLOCK (CAN09T_BAUDGEN_CLOCK / 12)

void can09t_state::can09t(machine_config &config)
{
	MC6809(config, m_maincpu, 4.9152_MHz_XTAL); // IPL crystal
	m_maincpu->set_addrmap(AS_PROGRAM, &can09t_state::can09t_map);

	/* --PIA inits----------------------- */
	PIA6821(config, m_syspia); // CPU board
	m_syspia->readpa_handler().set(FUNC(can09t_state::syspia_A_r));
	m_syspia->readpb_handler().set(FUNC(can09t_state::syspia_B_r));
	m_syspia->writepb_handler().set(FUNC(can09t_state::syspia_B_w));
	m_syspia->cb2_handler().set(FUNC(can09t_state::syspia_cb2_w));
	/* 0xE1FB 0xB112 (SYSPIA Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xE1FB 0xB113 (SYSPIA Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xE203 0xB110 (SYSPIA DDR A)     = 0x00 - Port A all inputs */
	/* 0xE203 0xB111 (SYSPIA DDR B)     = 0x7F - Port B mixed mode */
	/* 0xE20A 0xB112 (SYSPIA Control A) = 0x05 - IRQ A enabled on falling transition on CA2 */
	/* 0xE20A 0xB113 (SYSPIA Control B) = 0x34 - CB2 is low and lock DDRB */
	/* 0xE20E 0xB111 (SYSPIA port B)    = 0x10 - Data to port B */

	PIA6821(config, m_usrpia); // CPU board
	m_usrpia->cb2_handler().set(FUNC(can09t_state::usrpia_cb2_w));
	/* 0xE212 0xB122 (USRPIA Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xE212 0xB123 (USRPIA Control B) = 0x00 - Channel B IRQ disabled */
	/* 0xE215 0xB120 (USRPIA DDR A)     = 0x00 - Port A all inputs */
	/* 0xE215 0xB121 (USRPIA DDR B)     = 0xFF - Port B all outputs */
	/* 0xE21A 0xB122 (USRPIA Control A) = 0x34 - CA2 is low and lock DDRB */
	/* 0xE21A 0xB123 (USRPIA Control B) = 0x34 - CB2 is low and lock DDRB */
	PIA6821(config, m_pia3); // ROM board
	PIA6821(config, m_pia4); // ROM board

	PTM6840(config, "ptm", 0);

	/* RS232 usage: mame can09t -window -debug -rs232 terminal */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	CLOCK(config, "acia_clock", CAN09T_ACIA_CLOCK).signal_handler().set(FUNC(can09t_state::write_acia_clock));

	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

#define CAN09_X1_CLOCK 22.1184_MHz_XTAL        /* UKI 22118.40 Khz */
#define CAN09_CPU_CLOCK (CAN09_X1_CLOCK / 16) /* ~1.38MHz Divider needs to be check but is the most likelly */
void can09_state::can09(machine_config &config)
{
	MC6809E(config, m_maincpu, CAN09_CPU_CLOCK); // MC68A09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &can09_state::can09_map);

	/* RAM banks */
	RAM(config, RAM_TAG).set_default_size("768K");

	// CRTC init
	hd6845s_device &crtc(HD6845S(config, "crtc", CAN09_CPU_CLOCK)); // HD46505SP-1 (HD68A45SP)
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	//crtc.set_update_row_callback(FUNC(can09_state::crtc_update_row), this); // not written yet

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
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_raw(4_MHz_XTAL / 2, 512, 0, 512, 576, 0, 576);
	screen.set_screen_update(FUNC(can09_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Floppy */
	WD1770(config, "wd1770", 8_MHz_XTAL); // TODO: Verify 8MHz UKI crystal assumed to be used
#if 0
	FLOPPY_CONNECTOR(config, "wd1770:0", candela_floppies, "3dd", floppy_image_device::default_mfm_floppy_formats);
	SOFTWARE_LIST(config, "flop3_list").set_original("candela");
#endif

	/* --PIA inits----------------------- */
	PIA6821(config, m_pia1); // CPU board
	m_pia1->readpa_handler().set(FUNC(can09_state::pia1_A_r));
	m_pia1->writepa_handler().set(FUNC(can09_state::pia1_A_w));
	m_pia1->readpb_handler().set(FUNC(can09_state::pia1_B_r));
	m_pia1->writepb_handler().set(FUNC(can09_state::pia1_B_w));
	m_pia1->cb2_handler().set(FUNC(can09_state::pia1_cb2_w));
	/* 0xFF7D 0xE035 (PIA1 Control A) = 0x00 - Channel A IRQ disabled */
	/* 0xFF81 0xE037 (PIA1 Control B) = 0x00 - Channel A IRQ disabled */
	/* 0xFF85 0xE034 (PIA1 DDR A)     = 0x1F - Port A mixed mode */
	/* 0xFF89 0xE036 (PIA1 DDR B)     = 0x79 - Port B mixed mode */
	/* 0xFF8D 0xE035 (PIA1 Control A) = 0x04 - Channel A lock DDR */
	/* 0xFF8F 0xE037 (PIA1 Control B) = 0x04 - Channel B lock DDR */
	/* 0xFF93 0xE034 (PIA1 Port B)    = 0x18 - Write Data on Port B */

#if 1
	PIA6821(config, PIA2_TAG); // CPU board
	ACIA6850(config, "acia1", 0); // CPU board
	ACIA6850(config, "acia2", 0); // CPU board
#endif
}

ROM_START( can09t ) /* The smaller grey computer */
	ROM_REGION(0x10000, "roms", 0)
	/* CAN09 v7 and CDBASIC 3.8 */
	ROM_LOAD( "ic2-mon58b-c8d7.bin", 0x0000, 0x8000, CRC(7eabfec6) SHA1(e08e2349035389b441227df903aa54f4c1e4a337) )

	ROM_REGION(0x1000, "plas", 0)
	/* Programmable logic for the CAN09 1.4 PCB (CAN21.1) */
	ROM_LOAD( "ic10-21.1.bin",       0x0000, 0x20,   CRC(b75ac72d) SHA1(689363200035b11a823d17a8d717f313eeefc3bf) )
ROM_END

ROM_START( can09 ) /* The bigger black computer CAN v1 */
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD( "ic14-vdu42.bin", 0x0000, 0x2000, CRC(67fc3c8c) SHA1(1474d6259646798377ef4ce7e43d3c8d73858344) )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY            FULLNAME            FLAGS
COMP( 1984, can09,  0,      0,      can09,   can09,  can09_state,  empty_init, "Candela Data AB", "Candela CAN09 v1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_GRAPHICS)
COMP( 1984, can09t, 0,      0,      can09t,  can09t, can09t_state, empty_init, "Candela Data AB", "Candela CAN09",    MACHINE_NO_SOUND_HW )
