// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Telmac 2000

PCB Layout
----------

|-----------------------------------------------------------|
|                                                           |
|    |                  4051    4042        2114    2114    |
|   CN1                                                     |
|    |                  4051    4042        2114    2114    |
|                                                           |
|   4011                4013    |--------|  2114    2114    |
|                               |  4515  |                  |
|                       4013    |--------|  2114    2114    |
|                               4042                        |
|                       4011                2114    2114    |
|                               4011                        |
|                       4011                2114    2114    |
|SW1                                                        |
|                       4011    |--------|  2114    2114    |
|                               |  PROM  |                  |
|                       40107   |--------|  2114    2114    |
|   4050    1.75MHz                                         |
|                       |-------------|     2114    2114    |
|                       |   CDP1802   |                     |
|                       |-------------|     2114    2114    |
|           4049                                            |
|                       |-------------|     2114    2114    |
|       4.43MHz         |   CDP1864   |                     |
|                       |-------------|     2114    2114    |
|                                                           |
|               741             4502        2114    2114    |
|  -                                                        |
|  |                            2114        2114    2114    |
| CN2                                                       |
|  |            4001            4051        2114    2114    |
|  -                                                        |
|                                           2114    2114    |
|-----------------------------------------------------------|

Notes:
    All IC's shown.

    PROM    - MMI6341
    2114    - 2114 4096 Bit (1024x4) NMOS Static RAM
    CDP1802 - RCA CDP1802 CMOS 8-Bit Microprocessor @ 1.75 MHz
    CDP1864 - RCA CDP1864CE COS/MOS PAL Compatible Color TV Interface @ 1.75 MHz
    CN1     - keyboard connector
    CN2     - ASTEC RF modulator connector
    SW1     - Run/Reset switch

*/

/*

OSCOM Nano

PCB Layout
----------

OK 30379

|-------------------------------------------------|
|   CN1     CN2     CN3                 7805      |
|                       1.75MHz                   |
|                               741               |
|                       |-------------|         - |
|   741     741   4011  |   CDP1864   |         | |
|                       |-------------|         | |
|                 4013  |-------------|         | |
|                       |   CDP1802   |         | |
|                 4093  |-------------|         | |
|                                               C |
|            4051   4042    4017        4042    N |
|                           |-------|           4 |
|                           |  ROM  |   14556   | |
|                           |-------|           | |
|                           2114    2114        | |
|                                               | |
|                           2114    2114        | |
|                                               - |
|                           2114    2114          |
|                                                 |
|                           2114    2114          |
|-------------------------------------------------|

Notes:
    All IC's shown.

    ROM     - Intersil 5504?
    2114    - 2114UCB 4096 Bit (1024x4) NMOS Static RAM
    CDP1802 - RCA CDP1802E CMOS 8-Bit Microprocessor @ 1.75 MHz
    CDP1864 - RCA CDP1864CE COS/MOS PAL Compatible Color TV Interface @ 1.75 MHz
    CN1     - tape connector
    CN2     - video connector
    CN3     - power connector
    CN4     - expansion connector

Usage:
    Enter operating system with MONITOR key

    Commands:
    B N         Load N pages (of 256 bytes) from tape to current address, e.g. "MONITOR 0000 B 4" loads 4 pages to 0x0000
    F N         Save N pages (of 256 bytes) to tape starting from current address, e.g. "MONITOR 0200 F 4" saves 4 pages starting from 0x0200
    0           Write memory, e.g. "MONITOR 0400 0 A1 A3" writes 0xA1 to 0x0400 and 0xA3 to 0x0401
    A           Read memory, e.g. "MONITOR 0400 A 0 0 0" shows memory contents for 0x0400, 0x0401, 0x0402, 0x0403

    Loading software from tape:
    - Press MONITOR
    - Enter loading address (0000)
    - Press B
    - Enter length of program in pages (4)
    - Press RUN after program has loaded

Demo tape contents:
    Side A

    1. Esittelyohjelma (Demonstration Program) [length A pages]
    2. Reaktioaikatesti (Reaction Time Test) [4]
    3. Kaleidoskooppi (Kaleidoscope) [3] [CHIP-8]
    4. Labyrinttiohjelma (Labyrinth Program) [4]
    5. Labyrinttipeli (Labyrinth Game) [4] [CHIP-8]
    6. Yhteenlaskuohjelma (Addition Program) [4] [CHIP-8]
    7. Miinakenttä (Minefield) [4] [CHIP-8]
    8. Herästyskello (Alarm Clock) [4] [CHIP-8]
    9. Move Loop eli ansapeli [4]
    10. Pingis (Ping Pong) [6] [CHIP-8]
    11. Numeron arvaus (Number Guess) [3] [CHIP-8]
    12. Numeroiden kaato [4] [CHIP-8]
    13. Pyyhkäisypeli, yksinpelattava (Sweeping Game Single Player) [3] [CHIP-8]
    14. Pyyhkäisypeli, kaksinpelattava (Sweeping Game Dual Player) [6] [CHIP-8]
    15. Tikkupeli (Stick Game) [8] [CHIP-8]

    Side B

    16. Ampujaukko (Shooter Man) [4] [CHIP-8]
    17. Ufojen ammunta (UFO Shootout) [3]
    18. Jätkän shakki (Noughs and Crosses) [4] [CHIP-8]
    19. Jackpot [4]  [CHIP-8]
    20. Tankki ja ohjus (Tank and Missile) [5] [CHIP-8]
    21. Parien etsintä (Find the Pairs) [4] [CHIP-8]
    22. Tähtien ammunta (Star Shootout) [4] [CHIP-8]
    23. Vedonlyöntipeli (Betting Game) [4] [CHIP-8]
    24. Päättelytehtävä (Master Mind) [4] [CHIP-8]
    25. Piirtelyohjelma (Doodle) [4]  [CHIP-8]
    26. Säkkijärven polkka [4]
    27. Heksadesimaalikoodien harjoittelu (Hexadecimal Practice) [3]
    28. Histogrammaohjelma (Histogram) [4] [CHIP-8]
    29. M2-ohjelmointikieli (M2 Programming Language) [F]
    30. RAM-muistin testiohjelma (RAM Test) [?]

*/

/*

    TODO:

    - tape input/output
    - tmc2000: add missing keys
    - tmc2000: TOOL-2000 rom banking
    - nano: correct time constant for EF4 RC circuit


    Usage:
    - Same as VIP except the machine begins in the stopped mode.
    - So, to enter the monitor, hold C and press R
    - The support for chip-8 is not yet written, due to missing roms.
    - The screen for nano should be white not red (caused by using only the
      red output of a colour crt controller)
    - The monitor of the tmc1800 is difficult to read because the colour ram
      contains random values.
    - Both nano and tmc1800 seem to "work", but there's insufficient software
      to test with.

*/

#include "emu.h"

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/beep.h"
#include "sound/cdp1864.h"
#include "video/cdp1861.h"
#include "screen.h"
#include "speaker.h"

#define TMC2000_COLORRAM_SIZE   0x200

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "cdp1802"
#define CDP1861_TAG     "cdp1861"
#define CDP1864_TAG     "m3"

namespace {


class tmc1800_base_state : public driver_device
{
public:
	tmc1800_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_cassette(*this, "cassette")
		, m_rom(*this, CDP1802_TAG)
		, m_run(*this, "RUN")
		, m_ram(*this, RAM_TAG)
		, m_beeper(*this, "beeper")
	{ }

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	required_device<cosmac_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_rom;
	required_ioport m_run;
	required_device<ram_device> m_ram;
	optional_device<beep_device> m_beeper;
};

class tmc1800_state : public tmc1800_base_state
{
public:
	tmc1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_vdc(*this, CDP1861_TAG)
	{ }

	void keylatch_w(uint8_t data);
	uint8_t dispon_r();
	void dispoff_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

	void tmc1800(machine_config &config) ATTR_COLD;
	void tmc1800_io_map(address_map &map) ATTR_COLD;
	void tmc1800_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cdp1861_device> m_vdc;
	/* keyboard state */
	int m_keylatch = 0;
};

class osc1000b_state : public tmc1800_base_state
{
public:
	osc1000b_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
	{ }


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void keylatch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);

	void osc1000b(machine_config &config) ATTR_COLD;
	void osc1000b_io_map(address_map &map) ATTR_COLD;
	void osc1000b_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	/* keyboard state */
	int m_keylatch = 0;
};

class tmc2000_state : public tmc1800_base_state
{
public:
	tmc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_cti(*this, CDP1864_TAG)
		, m_colorram(*this, "color_ram", TMC2000_COLORRAM_SIZE, ENDIANNESS_LITTLE)
		, m_key_row(*this, {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"})
		, m_led(*this, "led1")
	{ }

	void keylatch_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void dma_w(offs_t offset, uint8_t data);
	int rdata_r();
	int bdata_r();
	int gdata_r();
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );

	void bankswitch();

	void tmc2000(machine_config &config) ATTR_COLD;
	void tmc2000_io_map(address_map &map) ATTR_COLD;
	void tmc2000_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cdp1864_device> m_cti;
	memory_share_creator<uint8_t> m_colorram;
	required_ioport_array<8> m_key_row;
	output_finder<> m_led;

	// memory
	int m_rac = 0;
	int m_roc = 0;

	/* video state */
	uint8_t m_color = 0;

	/* keyboard state */
	int m_keylatch = 0;
};

class nano_state : public tmc1800_base_state
{
public:
	nano_state(const machine_config &mconfig, device_type type, const char *tag)
		: tmc1800_base_state(mconfig, type, tag)
		, m_cti(*this, CDP1864_TAG)
		, m_ny0(*this, "NY0")
		, m_ny1(*this, "NY1")
		, m_monitor(*this, "MONITOR")
		, m_led(*this, "led1")
	{ }

	void keylatch_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER( run_pressed );
	DECLARE_INPUT_CHANGED_MEMBER( monitor_pressed );

	void nano(machine_config &config) ATTR_COLD;
	void nano_io_map(address_map &map) ATTR_COLD;
	void nano_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(assert_ef4);

	required_device<cdp1864_device> m_cti;
	required_ioport m_ny0;
	required_ioport m_ny1;
	required_ioport m_monitor;
	output_finder<> m_led;

	emu_timer *m_ef4_timer = nullptr;

	/* keyboard state */
	int m_keylatch = 0;
};


/* Video Handlers */

// Telmac 2000

int tmc2000_state::rdata_r()
{
	return BIT(m_color, 2);
}

int tmc2000_state::bdata_r()
{
	return BIT(m_color, 1);
}

int tmc2000_state::gdata_r()
{
	return BIT(m_color, 0);
}

// OSM-200

uint32_t osc1000b_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/* Read/Write Handlers */

void tmc1800_state::keylatch_w(uint8_t data)
{
	m_keylatch = data;
}

void osc1000b_state::keylatch_w(uint8_t data)
{
	m_keylatch = data;
}

void tmc2000_state::keylatch_w(uint8_t data)
{
	/*

	    bit     description

	    0       X0
	    1       X1
	    2       X2
	    3       Y0
	    4       Y1
	    5       Y2
	    6       EXP1
	    7       EXP2

	*/

	m_keylatch = data & 0x3f;
}

void nano_state::keylatch_w(uint8_t data)
{
	/*

	    bit     description

	    0       A
	    1       B
	    2       C
	    3       NY0
	    4       NY1
	    5
	    6
	    7

	*/

	m_keylatch = data & 0x1f;
}

void tmc2000_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();
	uint8_t *rom = m_rom->base();

	if (m_roc)
	{
		// monitor ROM
		program.install_rom(0x0000, 0x01ff, 0x7e00, rom);
	}
	else
	{
		// RAM
		switch (m_ram->size())
		{
		case 4 * 1024:
			program.install_ram(0x0000, 0x0fff, 0x7000, ram);
			break;

		case 16 * 1024:
			program.install_ram(0x0000, 0x3fff, 0x4000, ram);
			break;

		case 32 * 1024:
			program.install_ram(0x0000, 0x7fff, ram);
			break;
		}
	}

	if (m_rac)
	{
		// color RAM
		program.install_ram(0x8000, 0x81ff, 0x7e00, m_colorram);
		program.unmap_read(0x8000, 0xffff);
	}
	else
	{
		// monitor ROM
		program.install_rom(0x8000, 0x81ff, 0x7e00, rom);
	}
}

void tmc2000_state::bankswitch_w(uint8_t data)
{
	m_roc = 0;
	m_rac = BIT(data, 0);
	bankswitch();

	m_cti->tone_latch_w(data);
}

void nano_state::bankswitch_w(uint8_t data)
{
	/* enable RAM */
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();
	program.install_ram(0x0000, 0x0fff, 0x7000, ram);

	/* write to CDP1864 tone latch */
	m_cti->tone_latch_w(data);
}

uint8_t tmc1800_state::dispon_r()
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);

	return 0xff;
}

void tmc1800_state::dispoff_w(uint8_t data)
{
	m_vdc->disp_off_w(1);
	m_vdc->disp_off_w(0);
}

/* Memory Maps */

// Telmac 1800

void tmc1800_state::tmc1800_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x7800).ram();
	map(0x8000, 0x81ff).mirror(0x7e00).rom().region(CDP1802_TAG, 0);
}

void tmc1800_state::tmc1800_io_map(address_map &map)
{
	map(0x01, 0x01).rw(FUNC(tmc1800_state::dispon_r), FUNC(tmc1800_state::dispoff_w));
	map(0x02, 0x02).w(FUNC(tmc1800_state::keylatch_w));
}

// OSCOM 1000B

void osc1000b_state::osc1000b_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x7800).ram();
	map(0x8000, 0x81ff).mirror(0x7e00).rom().region(CDP1802_TAG, 0);
}

void osc1000b_state::osc1000b_io_map(address_map &map)
{
	map(0x02, 0x02).w(FUNC(osc1000b_state::keylatch_w));
}

// Telmac 2000

void tmc2000_state::tmc2000_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff); // RAM / monitor ROM
	map(0x8000, 0xffff); // color RAM / monitor ROM
}

void tmc2000_state::tmc2000_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x01, 0x01).rw(m_cti, FUNC(cdp1864_device::dispon_r), FUNC(cdp1864_device::step_bgcolor_w));
	map(0x02, 0x02).w(FUNC(tmc2000_state::keylatch_w));
	map(0x04, 0x04).r(m_cti, FUNC(cdp1864_device::dispoff_r)).w(FUNC(tmc2000_state::bankswitch_w));
}

// OSCOM Nano

void nano_state::nano_map(address_map &map)
{
	map(0x0000, 0x7fff); // RAM / monitor ROM
	map(0x8000, 0x81ff).mirror(0x7e00).rom().region(CDP1802_TAG, 0);
}

void nano_state::nano_io_map(address_map &map)
{
	map(0x01, 0x01).rw(m_cti, FUNC(cdp1864_device::dispon_r), FUNC(cdp1864_device::step_bgcolor_w));
	map(0x02, 0x02).w(FUNC(nano_state::keylatch_w));
	map(0x04, 0x04).r(m_cti, FUNC(cdp1864_device::dispoff_r)).w(FUNC(nano_state::bankswitch_w));
}

/* Input Ports */

static INPUT_PORTS_START( tmc1800 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( tmc2000_state::run_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();
	}
}

static INPUT_PORTS_START( tmc2000 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tmc2000_state::run_pressed), 0)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( nano_state::run_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();
	}
}

INPUT_CHANGED_MEMBER( nano_state::monitor_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();

		m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF4, CLEAR_LINE);
	}
	else if (!oldval && newval)
	{
		// TODO: what are the correct values?
		int t = RES_K(27) * CAP_U(1) * 1000; // t = R26 * C1
		m_ef4_timer->adjust(attotime::from_msec(t));
	}
}

static INPUT_PORTS_START( nano )
	PORT_START("NY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("NY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nano_state::run_pressed), 0)

	PORT_START("MONITOR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MONITOR") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nano_state::monitor_pressed), 0)
INPUT_PORTS_END

/* CDP1802 Interfaces */

// Telmac 1800

int tmc1800_state::clear_r()
{
	return BIT(m_run->read(), 0);
}

int tmc1800_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int tmc1800_state::ef3_r()
{
	return CLEAR_LINE; // TODO
}

void tmc1800_state::q_w(int state)
{
	m_cassette->output(state ? 1.0 : -1.0);
}

// Oscom 1000B

int osc1000b_state::clear_r()
{
	return BIT(m_run->read(), 0);
}

int osc1000b_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int osc1000b_state::ef3_r()
{
	return CLEAR_LINE; // TODO
}

void osc1000b_state::q_w(int state)
{
	m_cassette->output(state ? 1.0 : -1.0);
}

// Telmac 2000

int tmc2000_state::clear_r()
{
	return BIT(m_run->read(), 0);
}

int tmc2000_state::ef2_r()
{
	return (m_cassette)->input() < 0;
}

int tmc2000_state::ef3_r()
{
	uint8_t data = ~m_key_row[m_keylatch / 8]->read();

	return BIT(data, m_keylatch % 8);
}

void tmc2000_state::q_w(int state)
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* set Q led status */
	m_led = state ? 1 : 0;

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

void tmc2000_state::dma_w(offs_t offset, uint8_t data)
{
	m_color = ~(m_colorram[offset & 0x1ff]) & 0x07;

	m_cti->con_w(0); // HACK
	m_cti->dma_w(data);
}

// OSCOM Nano

int nano_state::clear_r()
{
	int run = BIT(m_run->read(), 0);
	int monitor = BIT(m_monitor->read(), 0);

	return run && monitor;
}

int nano_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int nano_state::ef3_r()
{
	uint8_t data = 0xff;

	if (!BIT(m_keylatch, 3)) data &= m_ny0->read();
	if (!BIT(m_keylatch, 4)) data &= m_ny1->read();

	return !BIT(data, m_keylatch & 0x07);
}

void nano_state::q_w(int state)
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* set Q led status */
	m_led = state ? 1 : 0;

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

/* Machine Initialization */

// Telmac 1800

void tmc1800_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}

void tmc1800_state::machine_reset()
{
	/* reset CDP1861 */
	m_vdc->reset();

	/* initialize beeper */
	m_beeper->set_state(0);
	m_beeper->set_clock(0);
}

// OSCOM 1000B

void osc1000b_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}

void osc1000b_state::machine_reset()
{
}

// Telmac 2000

void tmc2000_state::machine_start()
{
	// randomize color RAM contents
	for (uint16_t addr = 0; addr < TMC2000_COLORRAM_SIZE; addr++)
	{
		m_colorram[addr] = machine().rand() & 0xff;
	}

	// state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_rac));
	save_item(NAME(m_roc));
}

void tmc2000_state::machine_reset()
{
	// reset CDP1864
	m_cti->reset();

	// banking
	m_roc = 1;
	m_rac = 0;
	bankswitch();
}

// OSCOM Nano

TIMER_CALLBACK_MEMBER(nano_state::assert_ef4)
{
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF4, ASSERT_LINE);
}

void nano_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));

	/* allocate timers */
	m_ef4_timer = timer_alloc(FUNC(nano_state::assert_ef4), this);
}

void nano_state::machine_reset()
{
	/* assert EF4 */
	m_ef4_timer->adjust(attotime::zero);

	/* reset CDP1864 */
	m_cti->reset();

	/* enable ROM */
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *rom = m_rom->base();
	program.install_rom(0x0000, 0x01ff, 0x7e00, rom);
}

/* Machine Drivers */

QUICKLOAD_LOAD_MEMBER(tmc1800_base_state::quickload_cb)
{
	int const size = image.length();

	if (size > m_ram->size()) // FIXME: comparing size to RAM size, but loading to ROM - seems incorrect
	{
		return std::make_pair(image_error::INVALIDLENGTH, std::string());
	}

	uint8_t *const ptr = m_rom->base();
	image.fread(ptr, size);

	return std::make_pair(std::error_condition(), std::string());
}

void tmc1800_state::tmc1800(machine_config &config)
{
	// basic system hardware
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmc1800_state::tmc1800_map);
	m_maincpu->set_addrmap(AS_IO, &tmc1800_state::tmc1800_io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(tmc1800_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(tmc1800_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(tmc1800_state::ef3_r));
	m_maincpu->q_cb().set(FUNC(tmc1800_state::q_w));
	m_maincpu->dma_wr_cb().set(m_vdc, FUNC(cdp1861_device::dma_w));

	// video hardware
	CDP1861(config, m_vdc, XTAL(1'750'000)).set_screen(SCREEN_TAG);
	m_vdc->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_vdc->dma_out_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_DMAOUT);
	m_vdc->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(tmc1800_base_state::quickload_cb));
	// The following can be enabled when the missing bios rom is found
	//quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin,c8", attotime::from_seconds(2)));
	//quickload.set_load_callback(FUNC(tmc1800_base_state_state::quickload_cb));
	//quickload.set_interface("chip8quik");
	//SOFTWARE_LIST(config, "quik_list").set_original("chip8_quik").set_filter("T"); // filter unknown until it can be tested

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("2K").set_extra_options("4K");
}

void osc1000b_state::osc1000b(machine_config &config)
{
	// basic system hardware
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &osc1000b_state::osc1000b_map);
	m_maincpu->set_addrmap(AS_IO, &osc1000b_state::osc1000b_io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(osc1000b_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(osc1000b_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(osc1000b_state::ef3_r));
	m_maincpu->q_cb().set(FUNC(osc1000b_state::q_w));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(osc1000b_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_size(320, 200);
	screen.set_visarea(0, 319, 0, 199);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(tmc1800_base_state::quickload_cb));
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("2K").set_extra_options("4K");
}

void tmc2000_state::tmc2000(machine_config &config)
{
	// basic system hardware
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmc2000_state::tmc2000_map);
	m_maincpu->set_addrmap(AS_IO, &tmc2000_state::tmc2000_io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(tmc2000_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(tmc2000_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(tmc2000_state::ef3_r));
	m_maincpu->q_cb().set(FUNC(tmc2000_state::q_w));
	m_maincpu->dma_wr_cb().set(FUNC(tmc2000_state::dma_w));

	// video hardware
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);
	SPEAKER(config, "mono").front_center();
	CDP1864(config, m_cti, XTAL(1'750'000)).set_screen(SCREEN_TAG);
	m_cti->inlace_cb().set_constant(0);
	m_cti->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_cti->dma_out_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_DMAOUT);
	m_cti->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	m_cti->rdata_cb().set(FUNC(tmc2000_state::rdata_r));
	m_cti->bdata_cb().set(FUNC(tmc2000_state::bdata_r));
	m_cti->gdata_cb().set(FUNC(tmc2000_state::gdata_r));
	m_cti->set_chrominance(RES_K(1.21), RES_K(2.05), RES_K(2.26), RES_K(3.92)); // RL64, RL63, RL61, RL65 (also RH62 (2K pot) in series, but ignored here)
	m_cti->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(tmc1800_base_state::quickload_cb));
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("4K").set_extra_options("16K,32K");
}

void nano_state::nano(machine_config &config)
{
	// basic system hardware
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &nano_state::nano_map);
	m_maincpu->set_addrmap(AS_IO, &nano_state::nano_io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(nano_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(nano_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(nano_state::ef3_r));
	m_maincpu->q_cb().set(FUNC(nano_state::q_w));
	m_maincpu->dma_wr_cb().set(m_cti, FUNC(cdp1864_device::dma_w));

	// video hardware
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);
	SPEAKER(config, "mono").front_center();
	CDP1864(config, m_cti, XTAL(1'750'000)).set_screen(SCREEN_TAG);
	m_cti->inlace_cb().set_constant(0);
	m_cti->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_cti->dma_out_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_DMAOUT);
	m_cti->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	m_cti->rdata_cb().set_constant(1);
	m_cti->bdata_cb().set_constant(1);
	m_cti->gdata_cb().set_constant(1);
	m_cti->set_chrominance(RES_K(1.21), RES_INF, RES_INF, 0); // R18 (unconfirmed)
	m_cti->add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(tmc1800_base_state::quickload_cb));
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("4K");
}

/* ROMs */

ROM_START( tmc1800 )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6341-1.ic2", 0x000, 0x200, NO_DUMP ) // equivalent to 82S141
ROM_END

ROM_START( osc1000b )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6341-1.ic2", 0x000, 0x0200, NO_DUMP ) // equivalent to 82S141

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "mmi6349.5d", 0x000, 0x200, NO_DUMP ) // equivalent to 82S147
	ROM_LOAD( "mmi6349.5c", 0x200, 0x200, NO_DUMP ) // equivalent to 82S147
ROM_END

ROM_START( tmc2000 )
	ROM_REGION( 0x800, CDP1802_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "prom200", "PROM N:o 200" )
	ROMX_LOAD( "200.m5",    0x000, 0x200, BAD_DUMP CRC(79da3221) SHA1(008da3ef4f69ab1a493362dfca856375b19c94bd), ROM_BIOS(0) ) // typed in from the manual
	ROM_SYSTEM_BIOS( 1, "prom202", "PROM N:o 202" )
	ROMX_LOAD( "202.m5",    0x000, 0x200, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tool2000", "TOOL-2000" )
	ROMX_LOAD( "tool2000",  0x000, 0x800, NO_DUMP, ROM_BIOS(2) )
ROM_END

ROM_START( nano )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6349.ic", 0x000, 0x200, BAD_DUMP CRC(1ec1b432) SHA1(ac41f5e38bcd4b80bd7a5b277a2c600899fd5fb8) ) // equivalent to 82S141
ROM_END

} // anonymous namespace

/* System Drivers */

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS           INIT          COMPANY        FULLNAME       FLAGS
COMP( 1977, tmc1800,  0,       0,      tmc1800,  tmc1800, tmc1800_state,  empty_init,   "Telercas Oy", "Telmac 1800", MACHINE_NOT_WORKING )
COMP( 1977, osc1000b, tmc1800, 0,      osc1000b, tmc1800, osc1000b_state, empty_init,   "OSCOM Oy",    "OSCOM 1000B", MACHINE_NOT_WORKING )
COMP( 1980, tmc2000,  0,       0,      tmc2000,  tmc2000, tmc2000_state,  empty_init,   "Telercas Oy", "Telmac 2000", MACHINE_SUPPORTS_SAVE )
COMP( 1980, nano,     tmc2000, 0,      nano,     nano,    nano_state,     empty_init,   "OSCOM Oy",    "OSCOM Nano",  MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
