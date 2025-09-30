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
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

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
	PORT_3_RXD,
	PORT_3_TXD,
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

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t port1_r();
	void port1_w(uint8_t data);
	uint8_t port3_r();
	void port3_w(uint8_t data);
	uint8_t bus_r(offs_t offset);
	void bus_w(offs_t offset, uint8_t data);

	void servicet_io(address_map &map) ATTR_COLD;
	void servicet_map(address_map &map) ATTR_COLD;

	HD44780_PIXEL_UPDATE(servicet_pixel_update);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<i2cmem_device> m_i2cmem;
	required_device<hd44780_device> m_lcd;
	required_ioport_array<3> m_io_keys;

	uint8_t m_port1 = 0xff;
	uint8_t m_port3 = 0xff;
	uint8_t m_lcd_data = 0;
};

void servicet_state::servicet_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void servicet_state::servicet_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(servicet_state::bus_r), FUNC(servicet_state::bus_w));
}

static INPUT_PORTS_START( servicet )
	PORT_START("IN0") // P1.0
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("OK") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_4WAY

	PORT_START("IN1") // P1.1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_4WAY

	PORT_START("IN2") // P1.2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
INPUT_PORTS_END

void servicet_state::machine_start()
{
	save_item(NAME(m_port1));
	save_item(NAME(m_port3));
	save_item(NAME(m_lcd_data));
}

void servicet_state::machine_reset()
{
	m_port1 = 0xff;
	m_port3 = 0xff;
	m_lcd_data = 0;
}

uint8_t servicet_state::port1_r()
{
	uint8_t data = m_port1; // Start with what was written to port1

	// key matrix scanning seems to be bidirectional
	// CPU drives each line HIGH and checks if connected lines are also HIGH = button pressed

	// Column-to-Row scanning: When columns (P1.0-P1.2) are driven HIGH
	for (int col = 0; col < 3; col++)
	{
		if (BIT(m_port1, col)) // Column is driven HIGH
		{
			uint8_t const keys = m_io_keys[col]->read();
			data |= (keys & 0x70); // Mask to only row bits (4,5,6)
		}
	}

	// Row-to-Column scanning: When rows (P1.4-P1.6) are driven HIGH
	for (int row = 0; row < 3; row++)
	{
		if (BIT(m_port1, row + 4)) // Row is driven HIGH (bits 4,5,6)
		{
			// Check all columns for this row
			for (int col = 0; col < 3; col++)
			{
				uint8_t const keys = m_io_keys[col]->read();
				if (BIT(keys, row + 4)) // Key pressed in this row
				{
					data |= (1 << col); // Set corresponding column bit HIGH
				}
			}
		}
	}

	return data;
}

void servicet_state::port1_w(uint8_t data)
{
	m_port1 = data;
}

uint8_t servicet_state::port3_r()
{
	uint8_t data = m_port3;

	uint8_t const sda = m_i2cmem->read_sda();

	// Clear bit 4 (SDA) and insert actual value from EEPROM
	data = (data & ~(1 << PORT_3_SDA)) | (sda ? (1 << PORT_3_SDA) : 0);

	return data;
}

void servicet_state::port3_w(uint8_t data)
{
	m_port3 = data;

	m_i2cmem->write_sda(BIT(data, PORT_3_SDA));
	m_i2cmem->write_scl(BIT(data, PORT_3_SCL));
}

uint8_t servicet_state::bus_r(offs_t offset)
{
	uint8_t data = 0xff;

	// LCD is mapped to addresses where A6:A4 = 111 (0x70-0x7f)
	if ((offset & 0x70) == 0x70)
	{
		// RS and RW are A1 and A0
		bool rs = BIT(offset, 1);
		bool rw = BIT(offset, 0);

		if (rw)
		{
			m_lcd->rs_w(rs);
			m_lcd->rw_w(1);

			m_lcd->e_w(1);
			data = m_lcd->db_r();
			m_lcd->e_w(0);
		}
		else
		{
			data = m_lcd_data;
		}
	}
	else
	{
		//LOG("Bus read: %02X to %04X\n", offset);
	}

	return data;
}

void servicet_state::bus_w(offs_t offset, uint8_t data)
{
	// LCD is mapped to addresses where A6:A4 = 111 (0x70-0x7f)
	if ((offset & 0x70) == 0x70)
	{
		// RS and RW are A1 and A0
		bool const rs = BIT(offset, 1);
		bool const rw = BIT(offset, 0);

		if (!rw)
		{
			m_lcd_data = data;

			m_lcd->rs_w(rs);
			m_lcd->rw_w(0);
			m_lcd->db_w(data);

			m_lcd->e_w(1);
			m_lcd->e_w(0);
		}
	}
	else if (offset == 0x4000)
	{
		//LOG("GSG write: %02X \n", data, offset);
	}
	else
	{
		//LOG("Bus write: %02X to %04X\n", data, offset);
	}
}

void servicet_state::servicet(machine_config &config)
{
	I80C31(config, m_maincpu, 11.0592_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &servicet_state::servicet_map);
	m_maincpu->set_addrmap(AS_IO, &servicet_state::servicet_io);

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

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 40); // 2 lines, 40 characters
}

ROM_START( servicet )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "service_tastatur_v3.3.u3", 0x0000, 0x8000, CRC(8eb161c4) SHA1(d44f3b38e75e1095487893d8b30c4e3212c1a143) )

	ROM_REGION(0x800, "eeprom", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace

GAME( 1992, servicet, 0, servicet, servicet, servicet_state, empty_init, ROT0, "ADP", u8"Merkur Service Testgerät", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
