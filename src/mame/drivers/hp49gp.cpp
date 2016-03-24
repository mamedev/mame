// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Hewlett Packard 49G+ Graphing Calculator

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2410.h"
#include "rendlay.h"

#define VERBOSE_LEVEL ( 0 )

struct lcd_spi_t
{
	int l1;
	int data;
	int l3;
	UINT32 shift, bits;
};

class hp49gp_state : public driver_device
{
public:
	hp49gp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_s3c2410(*this, "s3c2410"),
		m_steppingstone(*this, "steppingstone"),
		m_maincpu(*this, "maincpu") { }

	UINT32 m_port[9];
	required_device<s3c2410_device> m_s3c2410;
	required_shared_ptr<UINT32> m_steppingstone;
	lcd_spi_t m_lcd_spi;
	DECLARE_DRIVER_INIT(hp49gp);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_INPUT_CHANGED_MEMBER(port_changed);
	DECLARE_READ32_MEMBER(s3c2410_gpio_port_r);
	DECLARE_WRITE32_MEMBER(s3c2410_gpio_port_w);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	void lcd_spi_reset( );
	void lcd_spi_init( );
	void lcd_spi_line_w( int line, int data);
	int lcd_spi_line_r( int line);
	required_device<cpu_device> m_maincpu;
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

inline void hp49gp_state::verboselog(int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine().describe_context( ), buf);
	}
}

void hp49gp_state::lcd_spi_reset( )
{
	verboselog( 5, "lcd_spi_reset\n");
	m_lcd_spi.l1 = 0;
	m_lcd_spi.data = 0;
	m_lcd_spi.l3 = 0;
}

void hp49gp_state::lcd_spi_init( )
{
	verboselog( 5, "lcd_spi_init\n");
	lcd_spi_reset();
}

void hp49gp_state::lcd_spi_line_w( int line, int data)
{
	switch (line)
	{
		case LCD_SPI_LINE_1 :
		{
			if (data != m_lcd_spi.l1)
			{
				if (data == 0)
				{
					m_lcd_spi.shift = 0;
					m_lcd_spi.bits = 0;
				}
				verboselog( 5, "LCD_SPI_LINE_1 <- %d\n", data);
				m_lcd_spi.l1 = data;
			}
		}
		break;
		case LCD_SPI_LINE_DATA :
		{
			if (data != m_lcd_spi.data)
			{
				verboselog( 5, "LCD_SPI_LINE_DATA <- %d\n", data);
				m_lcd_spi.data = data;
			}
		}
		break;
		case LCD_SPI_LINE_3 :
		{
			if (data != m_lcd_spi.l3)
			{
				if ((data != 0) && (m_lcd_spi.l1 == 0))
				{
					verboselog( 5, "LCD SPI write bit %d\n", m_lcd_spi.data ? 1 : 0);
					if (m_lcd_spi.bits < 8)
					{
						m_lcd_spi.shift = (m_lcd_spi.shift << 1) | (m_lcd_spi.data ? 1 : 0);
					}
					m_lcd_spi.bits++;
					if (m_lcd_spi.bits == 8)
					{
						verboselog( 5, "LCD SPI write byte %02X\n", m_lcd_spi.shift);
					}
					else if (m_lcd_spi.bits == 9)
					{
						verboselog( 5, "LCD SPI write ack %d\n", (m_lcd_spi.data ? 1 : 0));
					}
				}
				verboselog( 5, "LCD_SPI_LINE_3 <- %d\n", data);
				m_lcd_spi.l3 = data;
			}
		}
		break;
	}
}

int hp49gp_state::lcd_spi_line_r( int line)
{
	switch (line)
	{
		case LCD_SPI_LINE_1 :
		{
			verboselog( 7, "LCD_SPI_LINE_1 -> %d\n", m_lcd_spi.l1);
			return m_lcd_spi.l1;
		}
		case LCD_SPI_LINE_DATA :
		{
			verboselog( 7, "LCD_SPI_LINE_DATA -> %d\n", m_lcd_spi.data);
			return m_lcd_spi.data;
		}
		case LCD_SPI_LINE_3 :
		{
			verboselog( 7, "LCD_SPI_LINE_3 -> %d\n", m_lcd_spi.l3);
			return m_lcd_spi.l3;
		}
	}
	return 0;
}

// I/O PORT

READ32_MEMBER(hp49gp_state::s3c2410_gpio_port_r)
{
	UINT32 data = m_port[offset];
	switch (offset)
	{
		case S3C2410_GPIO_PORT_C :
		{
			data = data | 0xF000;
		}
		break;
		case S3C2410_GPIO_PORT_D :
		{
			data = data | 0x0008;
			data = data & ~0x3200;
			data |= (lcd_spi_line_r( LCD_SPI_LINE_1) ? 1 : 0) << 9;
			data |= (lcd_spi_line_r( LCD_SPI_LINE_DATA) ? 1 : 0) << 12;
			data |= (lcd_spi_line_r( LCD_SPI_LINE_3) ? 1 : 0) << 13;
		}
		break;
		case S3C2410_GPIO_PORT_E :
		{
			data = data | 0xC000;
		}
		break;
		case S3C2410_GPIO_PORT_F :
		{
			data = data | 0x0008;
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			data = data & ~0xFF00;
			if ((data & 0x02) == 0) data |= (ioport( "ROW1")->read() << 8);
			if ((data & 0x04) == 0) data |= (ioport( "ROW2")->read() << 8);
			if ((data & 0x08) == 0) data |= (ioport( "ROW3")->read() << 8);
			if ((data & 0x10) == 0) data |= (ioport( "ROW4")->read() << 8);
			if ((data & 0x20) == 0) data |= (ioport( "ROW5")->read() << 8);
			if ((data & 0x40) == 0) data |= (ioport( "ROW6")->read() << 8);
			if ((data & 0x80) == 0) data |= (ioport( "ROW7")->read() << 8);
		}
		break;
		case S3C2410_GPIO_PORT_H :
		{
			data = (data | 0x0080) & ~0x0040;
		}
		break;
	}
	return data;
}

WRITE32_MEMBER(hp49gp_state::s3c2410_gpio_port_w)
{
	m_port[offset] = data;
	switch (offset)
	{
		case S3C2410_GPIO_PORT_D :
		{
			lcd_spi_line_w( LCD_SPI_LINE_1, BIT( data, 9) ? 1 : 0);
			lcd_spi_line_w( LCD_SPI_LINE_DATA, BIT( data, 12) ? 1 : 0);
			lcd_spi_line_w( LCD_SPI_LINE_3, BIT( data, 13) ? 1 : 0);
		}
		break;
	}
}

// ...

INPUT_CHANGED_MEMBER(hp49gp_state::port_changed)
{
	m_s3c2410->s3c2410_request_eint( (FPTR)param + 8);
}

// ...

void hp49gp_state::machine_start()
{
}

void hp49gp_state::machine_reset()
{
	m_maincpu->reset();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( hp49gp_map, AS_PROGRAM, 32, hp49gp_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x08000000, 0x0801ffff) AM_RAM
	AM_RANGE(0x08020000, 0x0803ffff) AM_RAM
	AM_RANGE(0x08040000, 0x0807ffff) AM_RAM
	AM_RANGE(0x40000000, 0x40000fff) AM_RAM AM_SHARE("steppingstone")
ADDRESS_MAP_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

DRIVER_INIT_MEMBER(hp49gp_state,hp49gp)
{
	UINT8 *rom = (UINT8 *)memregion( "maincpu")->base();
	memcpy( m_steppingstone, rom, 1024);
	lcd_spi_init();
}

static MACHINE_CONFIG_START( hp49gp, hp49gp_state )
	MCFG_CPU_ADD("maincpu", ARM9, 400000000)
	MCFG_CPU_PROGRAM_MAP(hp49gp_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(160, 85)
	MCFG_SCREEN_VISIBLE_AREA(0, 131 - 1, 0, 80 - 1)
	MCFG_SCREEN_UPDATE_DEVICE("s3c2410", s3c2410_device, screen_update)

	MCFG_DEFAULT_LAYOUT(layout_lcd)


	MCFG_DEVICE_ADD("s3c2410", S3C2410, 12000000)
	MCFG_S3C2410_PALETTE("palette")
	MCFG_S3C2410_GPIO_PORT_R_CB(READ32(hp49gp_state, s3c2410_gpio_port_r))
	MCFG_S3C2410_GPIO_PORT_W_CB(WRITE32(hp49gp_state, s3c2410_gpio_port_w))
	MCFG_S3C2410_LCD_FLAGS(S3C24XX_INTERFACE_LCD_REVERSE)
MACHINE_CONFIG_END

static INPUT_PORTS_START( hp49gp )
	PORT_START("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)1) PORT_NAME("F1 | A | Y=") PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)2) PORT_NAME("F2 | B | WIN") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)3) PORT_NAME("F3 | C | GRAPH") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)4) PORT_NAME("F4 | D | 2D/3D") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)5) PORT_NAME("F5 | E | TBLSET") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)6) PORT_NAME("F6 | F | TABLE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, hp49gp_state, port_changed, (void *)7) PORT_NAME("APPS | G | FILES | BEGIN") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( hp49gp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "319", "Version 3.19" )
	ROMX_LOAD( "319.bin", 0x0000, 0x4000, CRC(6bb5ebfb) SHA1(089c4e4ee7223b489e2f06c30aab0b89131d1c3c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "315", "Version 3.15.04" )
	ROMX_LOAD( "31504.bin", 0x0000, 0x4000, CRC(9c71825e) SHA1(0a12b2b70a8573bc90ab5be06e6b2f814b8544ae), ROM_BIOS(2) )
ROM_END

COMP(2009, hp49gp, 0, 0, hp49gp, hp49gp, hp49gp_state, hp49gp, "Hewlett Packard", "HP49G+", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
