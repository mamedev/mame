// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Hewlett Packard 49G+ Graphing Calculator

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c24xx.h"
#include "screen.h"

#include <cstdarg>

#define LOG_LCD_SPI		(1U << 1)
#define LOG_LCD_SPI_RD	(1U << 2)
#define LOG_LCD_SPI_WR	(1U << 3)
#define VERBOSE ( 0 )
#include "logmacro.h"

namespace {

struct lcd_spi_t
{
	int l1 = 0;
	int data = 0;
	int l3 = 0;
	uint32_t shift = 0, bits = 0;
};

class hp49gp_state : public driver_device
{
public:
	hp49gp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2410(*this, "s3c2410")
	{ }

	void hp49gp(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(port_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;

	uint32_t gpio_port_r(offs_t offset);
	void gpio_port_w(offs_t offset, uint32_t data);
	void lcd_spi_reset();
	void lcd_spi_line_w( int line, int data);
	int lcd_spi_line_r(int line);

	required_device<cpu_device> m_maincpu;
	required_device<s3c2410_device> m_s3c2410;
	uint32_t m_port[9];
	lcd_spi_t m_lcd_spi;
};

/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// LCD SPI

enum
{
	LCD_SPI_LINE_1,
	LCD_SPI_LINE_DATA,
	LCD_SPI_LINE_3
};

void hp49gp_state::lcd_spi_reset()
{
	LOGMASKED(LOG_LCD_SPI, "lcd_spi_reset\n");
	m_lcd_spi.l1 = 0;
	m_lcd_spi.data = 0;
	m_lcd_spi.l3 = 0;
}

void hp49gp_state::lcd_spi_line_w( int line, int data)
{
	switch (line)
	{
	case LCD_SPI_LINE_1:
		if (data != m_lcd_spi.l1)
		{
			if (data == 0)
			{
				m_lcd_spi.shift = 0;
				m_lcd_spi.bits = 0;
			}
			LOGMASKED(LOG_LCD_SPI_WR, "LCD_SPI_LINE_1 <- %d\n", data);
			m_lcd_spi.l1 = data;
		}
		break;
	case LCD_SPI_LINE_DATA:
		if (data != m_lcd_spi.data)
		{
			LOGMASKED(LOG_LCD_SPI_WR, "LCD_SPI_LINE_DATA <- %d\n", data);
			m_lcd_spi.data = data;
		}
		break;
	case LCD_SPI_LINE_3:
		if (data != m_lcd_spi.l3)
		{
			if ((data != 0) && (m_lcd_spi.l1 == 0))
			{
				LOGMASKED(LOG_LCD_SPI_WR, "LCD SPI write bit %d\n", m_lcd_spi.data ? 1 : 0);
				if (m_lcd_spi.bits < 8)
				{
					m_lcd_spi.shift = (m_lcd_spi.shift << 1) | (m_lcd_spi.data ? 1 : 0);
				}
				m_lcd_spi.bits++;
				if (m_lcd_spi.bits == 8)
				{
					LOGMASKED(LOG_LCD_SPI_WR, "LCD SPI write byte %02X\n", m_lcd_spi.shift);
				}
				else if (m_lcd_spi.bits == 9)
				{
					LOGMASKED(LOG_LCD_SPI_WR, "LCD SPI write ack %d\n", (m_lcd_spi.data ? 1 : 0));
				}
			}
			LOGMASKED(LOG_LCD_SPI_WR, "LCD_SPI_LINE_3 <- %d\n", data);
			m_lcd_spi.l3 = data;
		}
		break;
	}
}

int hp49gp_state::lcd_spi_line_r(int line)
{
	switch (line)
	{
	case LCD_SPI_LINE_1:
		LOGMASKED(LOG_LCD_SPI_RD, "LCD_SPI_LINE_1 -> %d\n", m_lcd_spi.l1);
		return m_lcd_spi.l1;

	case LCD_SPI_LINE_DATA:
		LOGMASKED(LOG_LCD_SPI_RD, "LCD_SPI_LINE_DATA -> %d\n", m_lcd_spi.data);
			return m_lcd_spi.data;

	case LCD_SPI_LINE_3:
		LOGMASKED(LOG_LCD_SPI_RD, "LCD_SPI_LINE_3 -> %d\n", m_lcd_spi.l3);
		return m_lcd_spi.l3;
	}
	return 0;
}

// I/O PORT

uint32_t hp49gp_state::gpio_port_r(offs_t offset)
{
	uint32_t data = m_port[offset];
	switch (offset)
	{
	case s3c24xx_device::GPIO_PORT_C:
		data = data | 0xF000;
		break;
	case s3c24xx_device::GPIO_PORT_D:
		data = data | 0x0008;
		data = data & ~0x3200;
		data |= (lcd_spi_line_r( LCD_SPI_LINE_1) ? 1 : 0) << 9;
		data |= (lcd_spi_line_r( LCD_SPI_LINE_DATA) ? 1 : 0) << 12;
		data |= (lcd_spi_line_r( LCD_SPI_LINE_3) ? 1 : 0) << 13;
		break;
	case s3c24xx_device::GPIO_PORT_E:
		data = data | 0xC000;
		break;
	case s3c24xx_device::GPIO_PORT_F:
		data = data | 0x0008;
		break;
	case s3c24xx_device::GPIO_PORT_G:
		data = data & ~0xFF00;
		if ((data & 0x02) == 0) data |= (ioport("ROW1")->read() << 8);
		if ((data & 0x04) == 0) data |= (ioport("ROW2")->read() << 8);
		if ((data & 0x08) == 0) data |= (ioport("ROW3")->read() << 8);
		if ((data & 0x10) == 0) data |= (ioport("ROW4")->read() << 8);
		if ((data & 0x20) == 0) data |= (ioport("ROW5")->read() << 8);
		if ((data & 0x40) == 0) data |= (ioport("ROW6")->read() << 8);
		if ((data & 0x80) == 0) data |= (ioport("ROW7")->read() << 8);
		break;
	case s3c24xx_device::GPIO_PORT_H:
		data = (data | 0x0080) & ~0x0040;
		break;
	}
	return data;
}

void hp49gp_state::gpio_port_w(offs_t offset, uint32_t data)
{
	m_port[offset] = data;
	switch (offset)
	{
	case s3c24xx_device::GPIO_PORT_D:
		lcd_spi_line_w(LCD_SPI_LINE_1, BIT(data, 9) ? 1 : 0);
		lcd_spi_line_w(LCD_SPI_LINE_DATA, BIT(data, 12) ? 1 : 0);
		lcd_spi_line_w(LCD_SPI_LINE_3, BIT(data, 13) ? 1 : 0);
		break;
	}
}

INPUT_CHANGED_MEMBER(hp49gp_state::port_changed)
{
	m_s3c2410->gpio_set_extint(param, 1);
	m_s3c2410->gpio_set_extint(param, 0);
}

void hp49gp_state::machine_start()
{
	lcd_spi_reset();
}

void hp49gp_state::machine_reset()
{
	m_maincpu->reset();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void hp49gp_state::main_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x08000000, 0x0801ffff).ram();
	map(0x08020000, 0x0803ffff).ram();
	map(0x08040000, 0x0807ffff).ram();
	map(0x40000000, 0x5fffffff).m(m_s3c2410, FUNC(s3c2410_device::map));
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void hp49gp_state::hp49gp(machine_config &config)
{
	ARM9(config, m_maincpu, 400000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp49gp_state::main_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(160, 85);
	screen.set_visarea(0, 131 - 1, 0, 80 - 1);
	screen.set_screen_update("s3c2410", FUNC(s3c2410_device::screen_update));

	S3C2410(config, m_s3c2410, 12000000);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	m_s3c2410->gpio_port_r_callback().set(FUNC(hp49gp_state::gpio_port_r));
	m_s3c2410->gpio_port_w_callback().set(FUNC(hp49gp_state::gpio_port_w));
	m_s3c2410->set_lcd_flags(s3c2410_device::INTERFACE_LCD_REVERSE);
	m_s3c2410->om0_r_callback().set_constant(0); // OM[1:0] = 10: 32-bit ROM, no NAND Flash
	m_s3c2410->om1_r_callback().set_constant(1);
}

static INPUT_PORTS_START( hp49gp )
	PORT_START("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 1) PORT_NAME("F1 | A | Y=") PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 2) PORT_NAME("F2 | B | WIN") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 3) PORT_NAME("F3 | C | GRAPH") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 4) PORT_NAME("F4 | D | 2D/3D") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 5) PORT_NAME("F5 | E | TBLSET") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 6) PORT_NAME("F6 | F | TABLE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hp49gp_state::port_changed), 7) PORT_NAME("APPS | G | FILES | BEGIN") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( hp49gp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "319", "Version 3.19" )
	ROMX_LOAD( "319.bin", 0x0000, 0x4000, CRC(6bb5ebfb) SHA1(089c4e4ee7223b489e2f06c30aab0b89131d1c3c), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "315", "Version 3.15.04" )
	ROMX_LOAD( "31504.bin", 0x0000, 0x4000, CRC(9c71825e) SHA1(0a12b2b70a8573bc90ab5be06e6b2f814b8544ae), ROM_BIOS(1) )
ROM_END

} // anonymous namespace


COMP(2009, hp49gp, 0, 0, hp49gp, hp49gp, hp49gp_state, empty_init, "Hewlett Packard", "HP49G+", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
