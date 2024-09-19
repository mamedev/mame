// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

C-80 Trainer (East Germany)

Pasting:
    0-F : as is
    + (inc) : ^
    - (dec) : V
    M : -
    GO : X

Test Paste:
    -800^11^22^33^44^55^66^77^88^99^-800
    Now press up-arrow to confirm the data has been entered.

Commands:
    R : REGister
    M : MEMory manipulation
    G : GO
  F10 : RESet
  ESC : BRK

Functions (press F1 then the indicated number):
    0 : FILL
    1 : SAVE
    2 : LOAD
    3 : LOADP
    4 : MOVE
    5 : IN
    6 : OUT

When REG is chosen, use UP to scroll through the list of regs,
or press 0 thru C to choose one directly:
    0 : SP
    1 : PC
    2 : AF
    3 : BC
    4 : DE
    5 : HL
    6 : AF'
    7 : BC'
    8 : DE'
    9 : HL'
    A : IFF
    B : IX
    C : IY

When MEM is chosen, enter the address, press UP, enter data, press UP, enter
data of next byte, and so on.

Cassette SAVE: Press F1 1 aaaa DOWN bbbb UP
Cassette LOAD: Press F1 2 aaaa DOWN bbbb UP
(where aaaa = 4-digit beginning address, bbbb = 4-digit ending address)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "speaker.h"
#include "c80.lh"

namespace {

#define Z80_TAG         "d2"
#define Z80PIO1_TAG     "d11"
#define Z80PIO2_TAG     "d12"
// You could use a piezo at 455 kHz, or a crystal 500 to 2500 kHz.
// Cassette successfully tested at 455 kHz
#define MASTER_CLOCK    455000

class c80_state : public driver_device
{
public:
	c80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_pio1(*this, Z80PIO1_TAG)
		, m_pio2(*this, Z80PIO2_TAG)
		, m_cassette(*this, "cassette")
		, m_row0(*this, "ROW0")
		, m_row1(*this, "ROW1")
		, m_row2(*this, "ROW2")
		, m_digits(*this, "digit%d", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );
	void c80(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	output_finder<8> m_digits;

	virtual void machine_start() override ATTR_COLD;

	uint8_t pio1_pa_r();
	void pio1_pa_w(uint8_t data);
	void pio1_pb_w(uint8_t data);
	void pio1_brdy_w(int state);

	/* keyboard state */
	u8 m_keylatch = 0;

	/* display state */
	u8 m_digit = 0;
	bool m_pio1_a5 = false;
	u8 m_pio1_brdy = 0;
	void c80_io(address_map &map) ATTR_COLD;
	void c80_mem(address_map &map) ATTR_COLD;
};


/* Memory Maps */

void c80_state::c80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x0bff).mirror(0x400).ram();
}

void c80_state::c80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x7c, 0x7f).rw(m_pio2, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xbc, 0xbf).rw(m_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/* Input Ports */

INPUT_CHANGED_MEMBER( c80_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( c80_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c80 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FCN") PORT_CODE(KEYCODE_F1)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('-')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, c80_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BRK") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, c80_state, trigger_nmi, 0)
INPUT_PORTS_END

/* Z80-PIO Interface */

uint8_t c80_state::pio1_pa_r()
{
	/*

	    bit     description

	    PA0     keyboard row 0 input
	    PA1     keyboard row 1 input
	    PA2     keyboard row 2 input
	    PA3
	    PA4     _BSTB input
	    PA5     display enable output (0=enabled, 1=disabled)
	    PA6     tape output
	    PA7     tape input

	*/

	uint8_t data = m_pio1_brdy | 0x07;

	for (u8 i = 0; i < 8; i++)
	{
		if (!BIT(m_keylatch, i))
		{
			if (!BIT(m_row0->read(), i)) data &= ~0x01;
			if (!BIT(m_row1->read(), i)) data &= ~0x02;
			if (!BIT(m_row2->read(), i)) data &= ~0x04;
		}
	}

	data |= (m_cassette->input() < +0.0) << 7;

	return data;
}

void c80_state::pio1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     keyboard row 0 input
	    PA1     keyboard row 1 input
	    PA2     keyboard row 2 input
	    PA3
	    PA4     _BSTB input
	    PA5     display enable output (0=enabled, 1=disabled)
	    PA6     tape output
	    PA7     tape input

	*/

	m_pio1_a5 = BIT(data, 5);

	if (!BIT(data, 5))
	{
		m_digit = 0;
	}

	m_cassette->output(BIT(data, 6) ? +1.0 : -1.0);
}

void c80_state::pio1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     VQD30 segment A
	    PB1     VQD30 segment B
	    PB2     VQD30 segment C
	    PB3     VQD30 segment D
	    PB4     VQD30 segment E
	    PB5     VQD30 segment F
	    PB6     VQD30 segment G
	    PB7     VQD30 segment P

	*/

	if (!m_pio1_a5 && (m_digit < 8))
	{
		m_digits[m_digit] = data;
	}

	m_keylatch = data;
}

void c80_state::pio1_brdy_w(int state)
{
	m_pio1_brdy = state ? 0 : 0x10;

	if (state)
	{
		if (!m_pio1_a5)
		{
			m_digit++;
		}

		m_pio1->strobe_b(1);
		m_pio1->strobe_b(0);
	}
}

/* Z80 Daisy Chain */

static const z80_daisy_config c80_daisy_chain[] =
{
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ nullptr }
};

/* Machine Initialization */

void c80_state::machine_start()
{
	m_digits.resolve();
	/* register for state saving */
	save_item(NAME(m_keylatch));
	save_item(NAME(m_digit));
	save_item(NAME(m_pio1_a5));
	save_item(NAME(m_pio1_brdy));
}

/* Machine Driver */

void c80_state::c80(machine_config &config)
{
	// CPU
	Z80(config, m_maincpu, MASTER_CLOCK); // U880D
	m_maincpu->set_addrmap(AS_PROGRAM, &c80_state::c80_mem);
	m_maincpu->set_addrmap(AS_IO, &c80_state::c80_io);
	m_maincpu->set_daisy_config(c80_daisy_chain);

	// video
	config.set_default_layout(layout_c80);

	// devices
	Z80PIO(config, m_pio1, MASTER_CLOCK); // U855D
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio1->in_pa_callback().set(FUNC(c80_state::pio1_pa_r));
	m_pio1->out_pa_callback().set(FUNC(c80_state::pio1_pa_w));
	m_pio1->out_pb_callback().set(FUNC(c80_state::pio1_pb_w));
	m_pio1->out_brdy_callback().set(FUNC(c80_state::pio1_brdy_w));

	Z80PIO(config, m_pio2, MASTER_CLOCK); // U855D
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// cassette
	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROMs */

ROM_START( c80 )
	ROM_REGION( 0x0800, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "c80.d3", 0x0000, 0x0400, CRC(ad2b3296) SHA1(14f72cb73a4068b7a5d763cc0e254639c251ce2e) )
ROM_END

} // Anonymous namespace

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY          FULLNAME  FLAGS */
COMP( 1986, c80,  0,      0,      c80,     c80,   c80_state, empty_init, "Joachim Czepa", "C-80",   MACHINE_SUPPORTS_SAVE )
