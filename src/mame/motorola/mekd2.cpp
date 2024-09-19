// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Robbbert
/******************************************************************************
    Motorola Evaluation Kit 6800 D2
    MEK6800D2

    system driver

    Juergen Buchmueller, Jan 2000
    2013-06-16 Working driver [Robbbert]

    memory map

    range       short   description
    0000-00ff   RAM     256 bytes RAM
    0100-01ff   RAM     optional 256 bytes RAM
    6000-63ff   PROM    optional PROM
    or
    6000-67ff   ROM     optional ROM
    8004-8007   PIA     expansion port
    8008-8009   ACIA    cassette interface
    8020-8023   PIA     keyboard interface
    a000-a07f   RAM     128 bytes RAM (JBUG scratch)
    c000-c3ff   PROM    optional PROM
    or
    c000-c7ff   ROM     optional ROM
    e000-e3ff   ROM     JBUG monitor program
    e400-ffff   -/-     mirrors of monitor rom


Enter the 4 digit address then the command key:

  - M : Examine and Change Memory (example: E000M, then G to skip to next, ESC to exit)
  - E : Escape (abort) operation (ESC key in our emulation)
  - R : Examine Registers
  - G : Begin execution at specified address
  - P : Punch data from memory to magnetic tape
  - L : Load memory from magnetic tape
  - N : Trace one instruction
  - V : Set (and remove) breakpoints

The keys are laid out as:

  P L N V

  7 8 9 A  M
  4 5 6 B  E
  1 2 3 C  R
  0 F E D  G


Pasting:
        0-F : as is
        NEXT : ^
        MEM : =
        GO : ^

Test Paste:
        HA030=11^22^33^44^55^66^77^88^99^HA030=
        Now press up-arrow to confirm the data has been entered.

        If you wish to follow the tutorial in the manual, here is the test
        program that you need to enter in step 1:
        H0020=8E^00^FF^4F^C6^04^CE^00^10^AB^00^08^5A^26^FA^97^15^3F^H

        Save the above program to tape:
        HA002=00^20^00^32^HP (A002 has start address, A004 has end address, big endian)

TODO
        Display should go blank during cassette operations


******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/timer.h"
#include "speaker.h"

#include "mekd2.lh"


namespace {

#define XTAL_MEKD2 1228800

class mekd2_state : public driver_device
{
public:
	mekd2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia_s(*this, "pia_s")
		, m_pia_u(*this, "pia_u")
		, m_acia(*this, "acia")
		, m_cass(*this, "cassette")
		, m_trace_timer(*this, "trace_timer")
		, m_digits(*this, "digit%u", 0U)
		, m_keyboard(*this, "X%d", 0U)
	{ }

	void mekd2(machine_config &config);

private:
	int key40_r();
	uint8_t key_r();
	void nmi_w(int state);
	void digit_w(uint8_t data);
	void segment_w(uint8_t data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(trace_timer);

	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_cass_data[4]{};
	uint8_t m_segment = 0U;
	uint8_t m_digit = 0U;
	uint8_t m_keydata = 0U;
	bool m_cassbit = 0;
	bool m_cassold = 0;
	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_s;
	required_device<pia6821_device> m_pia_u;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;
	required_device<timer_device> m_trace_timer;
	output_finder<6> m_digits;
	required_ioport_array<6> m_keyboard;
};



/***********************************************************

    Address Map

************************************************************/

void mekd2_state::mem_map(address_map &map)
{
	map(0x0000, 0x00ff).ram(); // user ram
	map(0x8004, 0x8007).rw(m_pia_u, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8008, 0x8009).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8020, 0x8023).rw(m_pia_s, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xa000, 0xa07f).ram(); // system ram
	map(0xe000, 0xe3ff).rom().mirror(0x1c00).region("maincpu",0);   /* JBUG ROM */
}

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START( mekd2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (hex)") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') // save tape
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') // load tape
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') // trace (step)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') // breakpoint

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('=') // memory
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (escape)") PORT_CODE(KEYCODE_ESC) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') // regs
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_UP) PORT_CHAR('^') // go, next
INPUT_PORTS_END


/***********************************************************

    Trace hardware (what happens when N is pressed)

************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(mekd2_state::trace_timer)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void mekd2_state::nmi_w(int state)
{
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	else
		m_trace_timer->adjust(attotime::from_usec(18));
}



/***********************************************************

    Keyboard

************************************************************/

int mekd2_state::key40_r()
{
	return BIT(m_keydata, 6);
}

uint8_t mekd2_state::key_r()
{
	uint8_t i;
	m_keydata = 0xff;

	for (i = 0; i < 6; i++)
	{
		if (BIT(m_digit, i))
		{
			m_keydata &= m_keyboard[i]->read();
		}
	}

	i = 0x80;
	if (m_digit < 0x40)
		i = BIT(m_keydata, 0) ? 0x80 : 0;
	else
	if (m_digit < 0x80)
		i = BIT(m_keydata, 1) ? 0x80 : 0;
	else
	if (m_digit < 0xc0)
		i = BIT(m_keydata, 2) ? 0x80 : 0;
	else
		i = BIT(m_keydata, 3) ? 0x80 : 0;

	return i | m_segment;
}



/***********************************************************

    LED display

************************************************************/

void mekd2_state::segment_w(uint8_t data)
{
	m_segment = data & 0x7f;
}

void  mekd2_state::digit_w(uint8_t data)
{
	if (data < 0x3f)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (BIT(data, i))
				m_digits[i] = ~m_segment & 0x7f;
		}
	}
	m_digit = data;
}



/***********************************************************

    Interfaces

************************************************************/

QUICKLOAD_LOAD_MEMBER(mekd2_state::quickload_cb)
{
	static const char magic[] = "MEK6800D2";
	char buff[9];
	uint16_t addr, size;
	uint8_t ident, *RAM = memregion("maincpu")->base();

	image.fread(buff, sizeof(buff));
	if (memcmp(buff, magic, sizeof(buff)))
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("Magic '%s' not found", magic));
	}
	image.fread(&addr, 2);
	addr = little_endianize_int16(addr);
	image.fread(&size, 2);
	size = little_endianize_int16(size);
	image.fread(&ident, 1);
	logerror("mekd2 rom load: $%04X $%04X $%02X\n", addr, size, ident);
	while (size-- > 0)
		image.fread(&RAM[addr++], 1);

	return std::make_pair(std::error_condition(), std::string());
}

TIMER_DEVICE_CALLBACK_MEMBER(mekd2_state::kansas_w)
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(mekd2_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void mekd2_state::machine_start()
{
	m_digits.resolve();
}

/***********************************************************

    Machine

************************************************************/

void mekd2_state::mekd2(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL_MEKD2 / 2);        /* 614.4 kHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mekd2_state::mem_map);

	config.set_default_layout(layout_mekd2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* Devices */
	PIA6821(config, m_pia_s);
	m_pia_s->readpa_handler().set(FUNC(mekd2_state::key_r));
	m_pia_s->readcb1_handler().set(FUNC(mekd2_state::key40_r));
	m_pia_s->writepa_handler().set(FUNC(mekd2_state::segment_w));
	m_pia_s->writepb_handler().set(FUNC(mekd2_state::digit_w));
	m_pia_s->ca2_handler().set(FUNC(mekd2_state::nmi_w));
	m_pia_s->irqa_handler().set_inputline("maincpu", INPUT_LINE_NMI);
	m_pia_s->irqb_handler().set_inputline("maincpu", INPUT_LINE_NMI);

	PIA6821(config, m_pia_u);
	m_pia_u->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia_u->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &acia_tx_clock(CLOCK(config, "acia_tx_clock", XTAL_MEKD2 / 256)); // 4800Hz
	acia_tx_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));

	clock_device &acia_rx_clock(CLOCK(config, "acia_rx_clock", 300)); // toggled by cassette circuit
	acia_rx_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_rxc));

	TIMER(config, "kansas_w").configure_periodic(FUNC(mekd2_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(mekd2_state::kansas_r), attotime::from_hz(40000));
	TIMER(config, m_trace_timer).configure_generic(FUNC(mekd2_state::trace_timer));

	QUICKLOAD(config, "quickload", "d2", attotime::from_seconds(1)).set_load_callback(FUNC(mekd2_state::quickload_cb));
}

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd2)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("jbug.rom", 0x0000, 0x0400, CRC(5ed08792) SHA1(b06e74652a4c4e67c4a12ddc191ffb8c07f3332e) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY     FULLNAME      FLAGS
COMP( 1977, mekd2,  0,      0,      mekd2,    mekd2, mekd2_state, empty_init, "Motorola", "MEK6800D2" , MACHINE_SUPPORTS_SAVE )
