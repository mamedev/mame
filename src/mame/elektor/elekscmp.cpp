// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Elektor SC/MP

2009-11-22 Skeleton driver.
2012-05-10 Added keyboard [Robbbert]
2019-07-12 Added cassette [Robbbert]

To Use:
- Press MINUS to enter data input mode
- Press UP or DOWN to cycle through addresses

Paste test:
paste this in:  -0F0011^22^33^44^55^66^77^88^99^N-0F00
Now press UP to verify the data that was entered.

It seems the only way to exit each mode is to press NRST.

ToDo:


****************************************************************************/

#include "emu.h"
#include "cpu/scmp/scmp.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "speaker.h"
#include "elekscmp.lh"


namespace {

class elekscmp_state : public driver_device
{
public:
	elekscmp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_digit(*this, "digit%u", 0U)
	{ }

	void elekscmp(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	virtual void machine_start() override ATTR_COLD;

	u8 keyboard_r();
	void hex_display_w(offs_t offset, u8 data);
	int cass_r();
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	u8 convert_key(u8 data);
	bool m_cassinbit = 0, m_cassoutbit = 0, m_cassold = 0;
	u8 m_cass_data[4]{};

	void mem_map(address_map &map) ATTR_COLD;

	required_device<scmp_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<3> m_io_keyboard;
	output_finder<8> m_digit;
};


void elekscmp_state::machine_start()
{
	m_digit.resolve();

	save_item(NAME(m_cassinbit));
	save_item(NAME(m_cassoutbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_cass_data));
}

void elekscmp_state::hex_display_w(offs_t offset, u8 data)
{
	m_digit[offset & 0x7] = data;
}

u8 elekscmp_state::convert_key(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			return i;

	return 0xff;
}

u8 elekscmp_state::keyboard_r()
{
	u8 data = m_io_keyboard[0]->read();
	if (data)
		return 0x80 | convert_key(data);

	data = m_io_keyboard[1]->read();
	if (data)
		return 0x88 | convert_key(data);

	data = m_io_keyboard[2]->read();
	if (data)
		return 0x80 | (convert_key(data) << 4);

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( elekscmp_state::kansas_r )
{
	// no tape - set uart to idle
	m_cass_data[1]++;
	if (m_cass_data[1] > 32)
	{
		m_cass_data[1] = 32;
		m_cassinbit = 1;
	}

	/* cassette - turn 1200/2400Hz to a bit */
	u8 cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cassinbit = (m_cass_data[1] < 12) ? 1 : 0;
		m_cass_data[1] = 0;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( elekscmp_state::kansas_w )
{
	u8 twobit = m_cass_data[3] & 7;
	m_cass_data[3]++;

	if (twobit == 0)
		m_cassold = m_cassoutbit;

	if (m_cassold)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

int elekscmp_state::cass_r()
{
	return m_cassinbit;
}

void elekscmp_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x0fff);
	map(0x000, 0x5ff).rom(); // ROM
	map(0x700, 0x707).w(FUNC(elekscmp_state::hex_display_w));
	map(0x708, 0x70f).r(FUNC(elekscmp_state::keyboard_r));
	map(0x800, 0xfff).ram(); // RAM - up to 2K of RAM
}

/* Input ports */
static INPUT_PORTS_START( elekscmp )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CPU Reg") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Block Transfer") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cassette") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Subtract") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Modify") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Run") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('X')

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NRST") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, elekscmp_state, reset_button, 0) PORT_CHAR('N')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(elekscmp_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

void elekscmp_state::elekscmp(machine_config &config)
{
	/* basic machine hardware */
	INS8060(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &elekscmp_state::mem_map);
	m_maincpu->s_out().set([this] (bool state) { m_cassoutbit = state; });
	m_maincpu->s_in().set(FUNC(elekscmp_state::cass_r));

	/* video hardware */
	config.set_default_layout(layout_elekscmp);

	SPEAKER(config, "mono").front_center();
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(elekscmp_state::kansas_w), attotime::from_hz(4800));
	TIMER(config, "kansas_r").configure_periodic(FUNC(elekscmp_state::kansas_r), attotime::from_hz(40000));
}

/* ROM definition */
ROM_START( elekscmp )
	ROM_REGION( 0x0600, "maincpu", 0 )
	ROM_LOAD( "elbug.001", 0x0000, 0x0200, CRC(f733da28) SHA1(b65d98be03eab80478167964beec26bb327bfdf3))
	ROM_LOAD( "elbug.002", 0x0200, 0x0200, CRC(529c0b88) SHA1(bd72dd890cd974e1744ca70aa3457657374cbf76))
	ROM_LOAD( "elbug.003", 0x0400, 0x0200, CRC(13585ad1) SHA1(93f722b3e84095a1b701b04bf9018c891933b9ff))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY    FULLNAME         FLAGS */
COMP( 1977, elekscmp, 0,      0,      elekscmp, elekscmp, elekscmp_state, empty_init, "Elektor", "Elektor SC/MP", MACHINE_SUPPORTS_SAVE )
