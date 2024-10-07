// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Z80 dev board (unknown)

http://retro.hansotten.nl/index.php?page=z80-dev-kit

2010-04-23 Skeleton driver.

Pasting:
        0-F : as is
        MEM --> (inc) : ^
        MEM <-- (dec) : V
        GO : X
        To set an address, paste R0<address>, so R01040 to select 1040

Example paste: R01040^11^22^33^44^55^66^77^88^99^^R01040^
Press the up-arrow key to confirm data has been entered.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "z80dev.lh"

namespace {

class z80dev_state : public driver_device
{
public:
	z80dev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void z80dev(machine_config &config);

private:
	void display_w(offs_t offset, uint8_t data);
	uint8_t test_r();

	virtual void machine_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	output_finder<6> m_digits;
};

void z80dev_state::display_w(offs_t offset, uint8_t data)
{
	// ---- xxxx digit
	// xxxx ---- ???
	static const uint8_t hex_7seg[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};

	m_digits[offset] = hex_7seg[data & 0x0f];
}

uint8_t z80dev_state::test_r()
{
	return machine().rand();
}

void z80dev_state::machine_start()
{
	m_digits.resolve();
}

void z80dev_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x1000, 0x10ff).ram();
}

void z80dev_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x20, 0x20).portr("LINE0");
	map(0x21, 0x21).portr("LINE1");
	map(0x22, 0x22).portr("LINE2");
	map(0x23, 0x23).portr("LINE3");
	map(0x20, 0x25).w(FUNC(z80dev_state::display_w));

	map(0x13, 0x13).r(FUNC(z80dev_state::test_r));
}

/* Input ports */
INPUT_PORTS_START( z80dev )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RUN") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STEP") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BK") PORT_CODE(KEYCODE_K)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEM <--") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MEM -->") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MV") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EI") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I/O") PORT_CODE(KEYCODE_O)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SV") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LD") PORT_CODE(KEYCODE_L)
INPUT_PORTS_END

void z80dev_state::z80dev(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80dev_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80dev_state::io_map);

	/* video hardware */
	config.set_default_layout(layout_z80dev);
}

/* ROM definition */
ROM_START( z80dev )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "z80dev.bin", 0x0000, 0x0800, CRC(dd5b9cd9) SHA1(97c176fcb63674f0592851b7858cb706886b5857))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY      FULLNAME         FLAGS */
COMP( 198?, z80dev, 0,      0,      z80dev,  z80dev, z80dev_state, empty_init, "<unknown>", "Z80 dev board", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
