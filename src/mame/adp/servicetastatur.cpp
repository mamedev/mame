// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

ADP
Profitech 3000 Servicetastatur

Hardware:
- CPU: 80C52 @ 11.0592MHz
- LCD: LCD4002A
- Memory: 27C256 EPROM (32KB), 24CS16 I2C EEPROM (2KB)

Key Matrix Layout:
Col 0 (P1.0): OK, F4, UP
Col 1 (P1.1): RIGHT, LEFT, DOWN
Col 2 (P1.2): F3, F1, F2

GSG pinout to machine:
GND
Data Out
Enable
Data Clock
Data In
5V

Output is done by 74HC165.
Input is done by the 2 74HC4094.
D7 is connected to QP0 and D0 to QP7.
U19 has D1-D3 reversed from this.

    _____________________________________
   | 11.059     24CS16           TL7705  |
___| XTAL  80C31          +KEYPAD+       |__
|74HC00                               +    |
|          74HC165 74HC4094           G    |
|27C128                               S    |
|74LS573   74HC238 74HC4094           G    |
|___   +DISPLAY+  MC34063             + ___|
   |___________________________________|
*/

#include "emu.h"

#include "cpu/mcs51/i80c51.h"
#include "machine/i2cmem.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"

#include "servicet.lh"

namespace {

enum
{
	PORT_1_COL0,
	PORT_1_COL1,
	PORT_1_COL2,
	PORT_1_NC3,
	PORT_1_ROW0,
	PORT_1_ROW1,
	PORT_1_ROW2,
	PORT_1_NC7
};

enum
{
	PORT_3_RXD, //NC
	PORT_3_TXD, //NC
	PORT_3_INT0,
	PORT_3_INT1,
	PORT_3_SDA,
	PORT_3_SCL,
	PORT_3_WR,
	PORT_3_RD
};

class servicet_state : public driver_device
{
public:
	servicet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i2cmem(*this, "eeprom"),
		m_lcd(*this, "hd44780"),
		m_io_keys(*this, "IN%u", 0U)

	{ }

	void servicet(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(en_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t port1_r();
	void port1_w(uint8_t data);

	uint8_t port3_r();
	void port3_w(uint8_t data);

	uint8_t gsg_r_lower();
	uint8_t gsg_r_upper();
	void gsg_w(uint8_t data);

	void enable_in(int state);

	void servicet_data(address_map &map) ATTR_COLD;
	void servicet_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette);

	HD44780_PIXEL_UPDATE(servicet_pixel_update);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<i2cmem_device> m_i2cmem;
	required_device<hd44780_device> m_lcd;
	required_ioport_array<3> m_io_keys;

	bool m_datain = 0;

	uint8_t m_port1 = 0xff;
	uint8_t m_port3 = 0xff;

	uint16_t m_input = 0xffff;
	uint8_t m_output = 0xff;	
};

void servicet_state::servicet_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void servicet_state::servicet_data(address_map &map)
{
	// U16 74HC238
	map(0x0010, 0x001f).nopw(); //NC
	map(0x0020, 0x002f).nopw(); //NC
	map(0x0030, 0x003f).nopw(); //NC
	map(0x0040, 0x004f).r(FUNC(servicet_state::gsg_r_upper));
	map(0x0050, 0x005f).r(FUNC(servicet_state::gsg_r_lower));
	map(0x0060, 0x006f).w(FUNC(servicet_state::gsg_w));
	map(0x0070, 0x0070).w(m_lcd, FUNC(hd44780_device::control_w));
	map(0x0071, 0x0071).r(m_lcd, FUNC(hd44780_device::control_r));
	map(0x0072, 0x0072).w(m_lcd, FUNC(hd44780_device::data_w));
	map(0x0073, 0x0073).r(m_lcd, FUNC(hd44780_device::data_r));
	map(0x4000, 0x4000).nopw();
	map(0x8000, 0x8001).nopw();
}

static INPUT_PORTS_START( servicet )
	PORT_START("IN0") // P1.0
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START)       PORT_NAME("OK")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4)     PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1") // P1.1
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2") // P1.2
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("F3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("F1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("F2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P3") // P3
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("INT0") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(servicet_state::en_w), 0) //MCS51_INT0_LINE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("INT1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(servicet_state::en_w), 0) //MCS51_INT1_LINE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(i2cmem_device::read_sda))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void servicet_state::machine_start()
{
	save_item(NAME(m_port1));
	save_item(NAME(m_port3));

	save_item(NAME(m_input));
	save_item(NAME(m_output));
}

void servicet_state::machine_reset()
{
	m_port1 = 0xff;
	m_port3 = 0xff;

	m_input = 0xffff;
	m_output = 0xff;
}

uint8_t servicet_state::port1_r()
{
	/*
	* the keypad scan works by writing 0xfb,0xfd,0xfe (pulling rows low) to port 1
	* send seeing which bits end up low instead of high
	*/
	uint8_t data = m_port1;

	if (BIT(m_port1, 0)==0) data &= m_io_keys[0]->read();
	if (BIT(m_port1, 1)==0) data &= m_io_keys[1]->read();
	if (BIT(m_port1, 2)==0) data &= m_io_keys[2]->read();

	return data;
}


void servicet_state::port1_w(uint8_t data)
{
	m_port1 = data;
}

INPUT_CHANGED_MEMBER(servicet_state::en_w)
{
	enable_in(newval);
}

uint8_t servicet_state::port3_r()
{
	uint8_t data = ioport("P3")->read();

	uint8_t const SDA = m_i2cmem->read_sda();

	// Clear bit 4 (SDA) and insert actual value from EEPROM
	data = (data & ~(1 << PORT_3_SDA)) | (SDA ? (1 << PORT_3_SDA) : 0);

	return data;
}

void servicet_state::port3_w(uint8_t data)
{
	m_port3 = data;

	m_i2cmem->write_sda(BIT(data, PORT_3_SDA));
	m_i2cmem->write_scl(BIT(data, PORT_3_SCL));
}

uint8_t servicet_state::gsg_r_lower()
{
	// U20 74HC4094
	const uint8_t lower = m_input & 0xff;
	return bitswap<8>(lower, 0, 3, 2, 1, 4, 5, 6, 7); //reversed and D1+D3 swapped
}

uint8_t servicet_state::gsg_r_upper()
{
	// U19 74HC4094
	const uint8_t upper = (m_input >> 8) & 0xff;
	return bitswap<8>(upper, 0, 1, 2, 3, 4, 5, 6, 7); //reversed
}

void servicet_state::gsg_w(uint8_t data)
{
	// U13 74HC165
	m_output = data;
}

void servicet_state::enable_in(int newval)
{
	//strobe u19 and u20
	m_maincpu->set_input_line(MCS51_INT1_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(MCS51_INT0_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

void servicet_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(65, 165, 115));
	palette.set_pen_color(1, rgb_t(0, 50, 25));
}

void servicet_state::servicet(machine_config &config)
{
	I80C31(config, m_maincpu, 11.0592_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &servicet_state::servicet_map);
	m_maincpu->set_addrmap(AS_DATA, &servicet_state::servicet_data);

	m_maincpu->port_in_cb<1>().set(FUNC(servicet_state::port1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(servicet_state::port1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(servicet_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(servicet_state::port3_w));

	// I2C EEPROM: 24C16 (2KB) - connected to P3.4 (SDA) and P3.5 (SCL)
	I2C_24C16(config, m_i2cmem);

	// LCD4002A
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t(6, 120, 245));
	screen.set_physical_aspect(7*40, 10*2);
	screen.set_refresh_hz(72);
	screen.set_size(6*40, 9*2);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(servicet_state::palette_init), 2);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 40); // 2 lines, 40 characters
}

ROM_START( servicet )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "service_tastatur_v3.3.u3", 0x0000, 0x8000, CRC(8eb161c4) SHA1(d44f3b38e75e1095487893d8b30c4e3212c1a143) )

	ROM_REGION(0x800, "eeprom", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace

GAMEL( 1992, servicet, 0, servicet, servicet, servicet_state, empty_init, ROT0, "ADP", u8"Merkur Service Testgerät", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW, layout_servicet )
