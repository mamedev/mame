// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

VEB Polytechnik Poly-Computer 880

http://www.kc85-museum.de/books/poly880/index.html

Initially the screen is blank. The CTC causes a NMI, this autoboots the system,
and then the PIO releases the NMI line.

Pasting:
    0-F : as is
    EXEC : X
    BACK : K
    MEM : M
    GO : G

Test Paste:
    M4000X11X22X33X44X55X66X77X88X99XM4000
    Now press X to confirm the data has been entered.


The SC1 version is a modification that turns it into a chesscomputer.
Not to be confused with the prequel to SC2, but more likely a different
version of SLC1, without the "Lern" part. Just like SLC1, the chess engine
was copied from Fidelity's Sensory Chess Challenger 8.

SC1-SLC1 Keypad Reference:
    1-8 = A1-H8
    C = C (back)
    D = O (option)
    E = St (clear)
    F = Z (enter)


TODO:
- MCYCL (activate single stepping)
- CYCL (single step)
- layout LEDs (address bus, data bus, command bus, MCYCL)
- optional 32KB RAM expansion @ 0x8000
- who made poly880s? slc1 is very similar, it's by the same person?

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "poly880.lh"


namespace {

class poly880_state : public driver_device
{
public:
	poly880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio%u", 0)
		, m_ctc(*this, "ctc")
		, m_display(*this, "display")
		, m_cassette(*this, "cassette")
		, m_inputs(*this, "IN.%u", 0U)
	{ }

	void poly880(machine_config &config);
	void poly880s(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<pwm_display_device> m_display;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<3> m_inputs;

	u8 m_matrix = 0;
	bool m_nmi = false;

	void poly880_io(address_map &map) ATTR_COLD;
	void poly880_mem(address_map &map) ATTR_COLD;
	void poly880s_mem(address_map &map) ATTR_COLD;

	void cldig_w(u8 data);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void pio1_pa_w(u8 data);
	u8 pio1_pb_r();
	void pio1_pb_w(u8 data);
};


// Read/Write Handlers

void poly880_state::cldig_w(u8 data)
{
	m_display->write_my(data);
	m_matrix = data;
}


// Memory Maps

void poly880_state::poly880_mem(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0c00).rom();
	map(0x1000, 0x13ff).mirror(0x0c00).rom();
	map(0x2000, 0x23ff).mirror(0x0c00).rom();
	map(0x3000, 0x33ff).mirror(0x0c00).rom();
	map(0x4000, 0x43ff).mirror(0x3c00).ram();
}

void poly880_state::poly880s_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0x3c00).ram();
}

void poly880_state::poly880_io(address_map &map)
{
	map.global_mask(0xaf);
	map(0x80, 0x83).rw(m_pio[0], FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x84, 0x87).rw(m_pio[1], FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x88, 0x8b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xa0, 0xa0).mirror(0x0f).w(FUNC(poly880_state::cldig_w));
}


// Input Ports

INPUT_CHANGED_MEMBER(poly880_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(poly880_state::trigger_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( poly880 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("FCT") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MON") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MCYCL") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CYCL") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END


// Z80-CTC Interface

void poly880_state::ctc_z0_w(int state)
{
	// SEND
	if (!m_nmi && state)
	{
		m_nmi = true;
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

void poly880_state::ctc_z1_w(int state)
{
}


// Z80-PIO Interface

void poly880_state::pio1_pa_w(u8 data)
{
	/*

	    bit     signal  description

	    PA0     SD0     segment E
	    PA1     SD1     segment D
	    PA2     SD2     segment C
	    PA3     SD3     segment DP
	    PA4     SD4     segment G
	    PA5     SD5     segment A
	    PA6     SD6     segment F
	    PA7     SD7     segment B

	*/

	m_display->write_mx(bitswap<8>(data, 3, 4, 6, 0, 1, 2, 7, 5));
}

u8 poly880_state::pio1_pb_r()
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

	u8 data = 0x4c | ((m_cassette->input() < +0.0) << 1);

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_matrix, i))
		{
			if (BIT(m_inputs[0]->read(), i)) data |= 0x10;
			if (BIT(m_inputs[1]->read(), i)) data |= 0x20;
			if (BIT(m_inputs[2]->read(), i)) data |= 0x80;
		}
	}

	return data;
}

void poly880_state::pio1_pb_w(u8 data)
{
	/*

	    bit     signal  description

	    PB0     TTY     teletype
	    PB1
	    PB2     MOUT    tape output
	    PB3
	    PB4
	    PB5
	    PB6     SCON    release initial NMI
	    PB7

	*/

	// tape output
	m_cassette->output(BIT(data, 2) ? +1.0 : -1.0);

	if (m_nmi && BIT(data, 6))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_nmi = false;
	}
}


// Z80 Daisy Chain

static const z80_daisy_config poly880_daisy_chain[] =
{
	{ "pio0" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};


// Machine Initialization

void poly880_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_matrix));
	save_item(NAME(m_nmi));
}


// Machine Driver

void poly880_state::poly880(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(7'372'800)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly880_state::poly880_mem);
	m_maincpu->set_addrmap(AS_IO, &poly880_state::poly880_io);
	m_maincpu->set_daisy_config(poly880_daisy_chain);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_poly880);

	// devices
	Z80CTC(config, m_ctc, XTAL(7'372'800)/8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(poly880_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(poly880_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio[0], XTAL(7'372'800)/8);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[0]->out_pa_callback().set(FUNC(poly880_state::pio1_pa_w));
	m_pio[0]->in_pb_callback().set(FUNC(poly880_state::pio1_pb_r));
	m_pio[0]->out_pb_callback().set(FUNC(poly880_state::pio1_pb_w));

	Z80PIO(config, m_pio[1], XTAL(7'372'800)/8);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	SPEAKER(config, "cass_output").front_center(); // on data recorder
	m_cassette->add_route(ALL_OUTPUTS, "cass_output", 0.05);
}

void poly880_state::poly880s(machine_config &config)
{
	poly880(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &poly880_state::poly880s_mem);
}


// ROMs

ROM_START( poly880 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bm039.i5", 0x0000, 0x0400, CRC(b1c571e8) SHA1(85bfe53d39d6690e79999a1e1240789497e72db0) )
	ROM_LOAD( "bm040.i6", 0x1000, 0x0400, CRC(9efddf5b) SHA1(6ffa2f80b2c6f8ec9e22834f739c82f9754272b8) )
ROM_END

ROM_START( poly880s )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf) )
ROM_END

} // anonymous namespace


// System Drivers

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
COMP( 1983, poly880,  0,       0,      poly880,  poly880, poly880_state, empty_init, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Computer 880", MACHINE_SUPPORTS_SAVE )
COMP( 1983, poly880s, poly880, 0,      poly880s, poly880, poly880_state, empty_init, "hack", "Poly-Computer 880 (SC1)", MACHINE_SUPPORTS_SAVE )
