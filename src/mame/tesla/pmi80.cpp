// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************

Tesla PMI-80

2009-05-12 Skeleton driver.
2011-06-20 Made mostly working
2019-06-27 More work done


ToDo:
- cassette (coded per schematic, but not working ("SPAT" error))
- I button (should cause an interrupt and jump to 0038 - currently nothing happens)


Notes:
- Keyboard consists of 16 black hex keys, and 9 blue function keys
- The hex keys are 0 through 9, A through F on our keyboard
- The function key labels are RE, I, EX, R, BR, M, L, S, =
- The letter M shows as an inverted U in the display
- Turn it on, it says ''PMI -80''
- Press any key, it shows a ? at the left
- Press the function key corresponding to what you want to do
- Press the numbers to select an address or whatever
- For example, press M then enter an address, press =, enter data,
   press =  to increment to next address or to scan through them.
- PPI1 is programmed to mode 0, Port A output, Port B input, PC0-3 output, PC4-7 input.

Paste test:
    Paste --1C00^11^22^33^44^55^66^77^88^99^---1C00^
    Press = to verify the data that was entered.

*******************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "speaker.h"
#include "pmi80.lh"


namespace {

class pmi80_state : public driver_device
{
public:
	pmi80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi1(*this, "ppi1")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void pmi80(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(int_button);

private:
	uint8_t keyboard_r();
	void keyboard_w(uint8_t data);
	void leds_w(uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_keyrow = 0U;
	bool m_ledready = false;
	bool m_cassbit = false, m_cassold = false;
	u16 m_cass_cnt = 0U;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi1;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<9> m_io_keyboard;
	output_finder<9> m_digits;
};


uint8_t pmi80_state::keyboard_r()
{
	u8 data = 0x7f;
	if (m_keyrow > 0xF6)
		data &= m_io_keyboard[m_keyrow-0xf7]->read();

	data |= (m_cassbit) ? 0x80 : 0;
	return data;
}

void pmi80_state::keyboard_w(uint8_t data)
{
	m_keyrow = data;
	m_ledready = true;
}

void pmi80_state::leds_w(uint8_t data)
{
	if (m_ledready)
	{
		m_ledready = false;
		if (m_keyrow > 0xF6)
			m_digits[m_keyrow^0xff] = data^0xff;
	}

	data &= 0xc0;
	m_cass->output((data == 0xc0) ? -1.0 : +1.0);
}

TIMER_DEVICE_CALLBACK_MEMBER( pmi80_state::kansas_r )
{
	// cassette - pulses = 1; no pulses = 0
	m_cass_cnt++;
	bool cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cassold)
	{
		m_cassold = cass_ws;
		m_cassbit = (m_cass_cnt < 40) ? 1 : 0;
		m_cass_cnt = 0;
	}
}


void pmi80_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xdfff); // A13 not used
	map(0x0000, 0x07ff).rom();
	map(0x1c00, 0x1fff).ram();
}

void pmi80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0f);
	map(0x04, 0x07).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write)); // io12, 0xf4-0xf7
	map(0x08, 0x0b).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write)); // io8,  0xf8-0xfb
}

/* Input ports */
static INPUT_PORTS_START( pmi80 )
	PORT_START("SP")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RE") PORT_CODE(KEYCODE_LALT) PORT_CHANGED_MEMBER(DEVICE_SELF, pmi80_state, reset_button, 0) PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHANGED_MEMBER(DEVICE_SELF, pmi80_state, int_button, 0) PORT_CHAR('I')
	PORT_START("X0")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_START("X1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_START("X2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B BC") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 HL") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_START("X3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('-')// Memory mode
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_START("X4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BR") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') // Breakpoint
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D DE") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_START("X5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') // Registers
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EX") PORT_CODE(KEYCODE_G) PORT_CHAR('X') // Go
	PORT_START("X6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') // Load tape
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A AF") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 SP") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_START("X7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') // Save tape
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_START("X8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(pmi80_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, oldval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(pmi80_state::int_button)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, oldval ? ASSERT_LINE : CLEAR_LINE);  // FIXME: jump to 0x0038
}


void pmi80_state::machine_reset()
{
}

void pmi80_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_keyrow));
	save_item(NAME(m_ledready));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cass_cnt));
}


void pmi80_state::pmi80(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(10'000'000)/9);   // 10MHz divided by 9 (by MH8224)
	m_maincpu->set_addrmap(AS_PROGRAM, &pmi80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pmi80_state::io_map);

	I8255A(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(pmi80_state::leds_w));
	m_ppi1->in_pc_callback().set(FUNC(pmi80_state::keyboard_r));
	m_ppi1->out_pc_callback().set(FUNC(pmi80_state::keyboard_w));

	I8255A(config, "ppi2");   // User PPI

	SPEAKER(config, "mono").front_center();

	// cassette
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_r").configure_periodic(FUNC(pmi80_state::kansas_r), attotime::from_hz(40000));

	/* video hardware */
	config.set_default_layout(layout_pmi80);
}

/* ROM definition */
ROM_START( pmi80 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pmi80_monitor.io5", 0x0000, 0x0400, CRC(b93f4407) SHA1(43153441070ed0572f33d2815635eb7bae878e38))
	//ROM_LOAD("expansion.io11", 0x0400, 0x0400, NO_DUMP)   Empty ROM socket
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1982, pmi80, 0,      0,      pmi80,   pmi80, pmi80_state, empty_init, "Tesla", "PMI-80", MACHINE_SUPPORTS_SAVE )
