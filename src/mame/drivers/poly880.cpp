// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

Poly-Computer 880

2009-05-12 Skeleton driver.

http://www.kc85-museum.de/books/poly880/index.html

Initially the screen is blank. The CTC causes a NMI, this autoboots the
system, and then the PIO releases the NMI line.

Pasting:
        0-F : as is
        EXEC : ^
        BACK : V
        MEM : -
        GO : X

Test Paste:
        -4000^11^22^33^44^55^66^77^88^99^-4000
        Now press up-arrow to confirm the data has been entered.

TODO:
    - MCYCL (activate single stepping)
    - CYCL (single step)
    - layout LEDs (address bus, data bus, command bus, MCYCL)
    - RAM expansion


****************************************************************************/

#include "emu.h"
#include "includes/poly880.h"
#include "poly880.lh"


/* Read/Write Handlers */

void poly880_state::update_display()
{
	for (int i = 0; i < 8; i++)
		if (BIT(m_digit, i))
			m_digits[7 - i] = m_segment;
}

WRITE8_MEMBER( poly880_state::cldig_w )
{
	m_digit = data;

	update_display();
}

/* Memory Maps */

void poly880_state::poly880_mem(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0c00).rom();
	map(0x1000, 0x13ff).mirror(0x0c00).rom();
	map(0x2000, 0x23ff).mirror(0x0c00).rom();
	map(0x3000, 0x33ff).mirror(0x0c00).rom();
	map(0x4000, 0x43ff).mirror(0x3c00).ram();
	map(0x8000, 0xffff).bankrw("bank1");
}

void poly880_state::poly880_io(address_map &map)
{
	map.global_mask(0xaf);
	map(0x80, 0x83).rw(Z80PIO1_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x84, 0x87).rw(Z80PIO2_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x88, 0x8b).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xa0, 0xa0).mirror(0x0f).w(FUNC(poly880_state::cldig_w));
}

/* Input Ports */

INPUT_CHANGED_MEMBER( poly880_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( poly880_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( poly880 )
	PORT_START("KI1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FCT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('-')

	PORT_START("KI2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("KI3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MON") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MCYCL") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CYCL") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( poly880_state::ctc_z0_w )
{
	// SEND
	if (!m_nmi && state)
	{
		m_nmi = true;
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER( poly880_state::ctc_z1_w )
{
}

/* Z80-PIO Interface */

WRITE8_MEMBER( poly880_state::pio1_pa_w )
{
	/*

	    bit     signal  description

	    PA0     SD0     segment E
	    PA1     SD1     segment D
	    PA2     SD2     segment C
	    PA3     SD3     segment P
	    PA4     SD4     segment G
	    PA5     SD5     segment A
	    PA6     SD6     segment F
	    PA7     SD7     segment B

	*/

	m_segment = bitswap<8>(data, 3, 4, 6, 0, 1, 2, 7, 5);

	update_display();
}

READ8_MEMBER( poly880_state::pio1_pb_r )
{
	/*

	    bit     signal  description

	    PB0
	    PB1     MIN     tape input
	    PB2
	    PB3     n/c
	    PB4     KI1     key row 1 input
	    PB5     KI2     key row 2 input
	    PB6
	    PB7     KI3     key row 3 input

	*/

	uint8_t data = 0x4c | ((m_cassette->input() < +0.0) << 1);
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(m_digit, i))
		{
			if (BIT(m_ki[0]->read(), i)) data |= 0x10;
			if (BIT(m_ki[1]->read(), i)) data |= 0x20;
			if (BIT(m_ki[2]->read(), i)) data |= 0x80;
		}
	}

	return data;
}

WRITE8_MEMBER( poly880_state::pio1_pb_w )
{
	/*

	    bit     signal  description

	    PB0     TTY     speaker
	    PB1
	    PB2     MOUT    tape output
	    PB3
	    PB4
	    PB5
	    PB6     SCON    release initial NMI
	    PB7

	*/

	m_speaker->level_w( BIT(data, 0));
	/* tape output */
	m_cassette->output( BIT(data, 2) ? +1.0 : -1.0);

	if (m_nmi && BIT(data, 6))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_nmi = false;
	}
}


/* Z80 Daisy Chain */

static const z80_daisy_config poly880_daisy_chain[] =
{
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};


/* Machine Initialization */

void poly880_state::machine_start()
{
	m_digits.resolve();

	/* register for state saving */
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
}

/* Machine Driver */

void poly880_state::poly880(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'372'800)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly880_state::poly880_mem);
	m_maincpu->set_addrmap(AS_IO, &poly880_state::poly880_io);
	m_maincpu->set_daisy_config(poly880_daisy_chain);

	/* video hardware */
	config.set_default_layout(layout_poly880);

	/* devices */
	z80ctc_device& ctc(Z80CTC(config, Z80CTC_TAG, XTAL(7'372'800)/16));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(poly880_state::ctc_z0_w));
	ctc.zc_callback<1>().set(FUNC(poly880_state::ctc_z1_w));
	ctc.zc_callback<2>().set(Z80CTC_TAG, FUNC(z80ctc_device::trg3));

	z80pio_device& pio1(Z80PIO(config, Z80PIO1_TAG, XTAL(7'372'800)/16));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio1.out_pa_callback().set(FUNC(poly880_state::pio1_pa_w));
	pio1.in_pb_callback().set(FUNC(poly880_state::pio1_pb_r));
	pio1.out_pb_callback().set(FUNC(poly880_state::pio1_pb_w));

	z80pio_device& pio2(Z80PIO(config, Z80PIO2_TAG, XTAL(7'372'800)/16));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1K");
}

/* ROMs */

ROM_START( poly880 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "poly880.i5", 0x0000, 0x0400, CRC(b1c571e8) SHA1(85bfe53d39d6690e79999a1e1240789497e72db0) )
	ROM_LOAD( "poly880.i6", 0x1000, 0x0400, CRC(9efddf5b) SHA1(6ffa2f80b2c6f8ec9e22834f739c82f9754272b8) )
ROM_END

/* System Drivers */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME             FLAGS
COMP( 1983, poly880, 0,      0,      poly880, poly880, poly880_state, empty_init, "VEB Polytechnik", "Poly-Computer 880", MACHINE_SUPPORTS_SAVE )
