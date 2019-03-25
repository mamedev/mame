// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Elektor SC/MP

2009-11-22 Skeleton driver.
2012-05-10 Added keyboard [Robbbert]

To Use:
- Press MINUS to enter data input mode
- Press UP or DOWN to cycle through addresses

At the moment Paste cannot be tested, but if it worked, you could
paste this in:  -0F0011^22^33^44^55^66^77^88^99^

It seems the only way to exit each mode is to do a Soft Reset.

ToDo:
- Add Cassette
- Verify that ROMS are good (they seem to be)


****************************************************************************/

#include "emu.h"
#include "cpu/scmp/scmp.h"
#include "elekscmp.lh"


class elekscmp_state : public driver_device
{
public:
	elekscmp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_x(*this, "X%u", 0U)
		, m_digit(*this, "digit%u", 0U)
	{ }

	void elekscmp(machine_config &config);

private:
	virtual void machine_start() override;

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(hex_display_w);
	uint8_t convert_key(uint8_t data);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_x;
	output_finder<8> m_digit;
};


void elekscmp_state::machine_start()
{
	m_digit.resolve();
}

WRITE8_MEMBER(elekscmp_state::hex_display_w)
{
	m_digit[offset & 0x7] = data;
}

uint8_t elekscmp_state::convert_key(uint8_t data)
{
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			return i;

	return 0xff;
}

READ8_MEMBER(elekscmp_state::keyboard_r)
{
	uint8_t data;

	data = m_x[0]->read();
	if (data)
		return 0x80 | convert_key(data);

	data = m_x[1]->read();
	if (data)
		return 0x88 | convert_key(data);

	data = m_x[2]->read();
	if (data)
		return 0x80 | (convert_key(data) << 4);

	data = m_x[3]->read();
	if (data)
		m_maincpu->reset();

	return 0;
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

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RST") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void elekscmp_state::elekscmp(machine_config &config)
{
	/* basic machine hardware */
	INS8060(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &elekscmp_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_elekscmp);
}

/* ROM definition */
ROM_START( elekscmp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// Too many possible errors, few found and fixed, but not sure if there are more
	ROM_LOAD( "elbug.001", 0x0000, 0x0200, BAD_DUMP CRC(f733da28) SHA1(b65d98be03eab80478167964beec26bb327bfdf3))
	ROM_LOAD( "elbug.002", 0x0200, 0x0200, BAD_DUMP CRC(529c0b88) SHA1(bd72dd890cd974e1744ca70aa3457657374cbf76))
	ROM_LOAD( "elbug.003", 0x0400, 0x0200, BAD_DUMP CRC(13585ad1) SHA1(93f722b3e84095a1b701b04bf9018c891933b9ff))
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                FULLNAME         FLAGS */
COMP( 1977, elekscmp, 0,      0,      elekscmp, elekscmp, elekscmp_state, empty_init, "Elektor Electronics", "Elektor SC/MP", MACHINE_NO_SOUND_HW)
