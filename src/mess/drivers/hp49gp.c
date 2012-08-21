/*******************************************************************************

    Hewlett Packard 49G+ Graphing Calculator

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2410.h"
#include "rendlay.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

typedef struct
{
	int l1;
	int data;
	int l3;
	UINT32 shift, bits;
} lcd_spi_t;

class hp49gp_state : public driver_device
{
public:
	hp49gp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_steppingstone(*this, "steppingstone"){ }

	UINT32 m_port[9];
	device_t *m_s3c2410;
	required_shared_ptr<UINT32> m_steppingstone;
	lcd_spi_t m_lcd_spi;
	DECLARE_DRIVER_INIT(hp49gp);
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

static void lcd_spi_reset( running_machine &machine)
{
	hp49gp_state *hp49gp = machine.driver_data<hp49gp_state>();
	verboselog( machine, 5, "lcd_spi_reset\n");
	hp49gp->m_lcd_spi.l1 = 0;
	hp49gp->m_lcd_spi.data = 0;
	hp49gp->m_lcd_spi.l3 = 0;
}

static void lcd_spi_init( running_machine &machine)
{
	verboselog( machine, 5, "lcd_spi_init\n");
	lcd_spi_reset( machine);
}

static void lcd_spi_line_w( running_machine &machine, int line, int data)
{
	hp49gp_state *hp49gp = machine.driver_data<hp49gp_state>();
	switch (line)
	{
		case LCD_SPI_LINE_1 :
		{
			if (data != hp49gp->m_lcd_spi.l1)
			{
				if (data == 0)
				{
					hp49gp->m_lcd_spi.shift = 0;
					hp49gp->m_lcd_spi.bits = 0;
				}
				verboselog( machine, 5, "LCD_SPI_LINE_1 <- %d\n", data);
				hp49gp->m_lcd_spi.l1 = data;
			}
		}
		break;
		case LCD_SPI_LINE_DATA :
		{
			if (data != hp49gp->m_lcd_spi.data)
			{
				verboselog( machine, 5, "LCD_SPI_LINE_DATA <- %d\n", data);
				hp49gp->m_lcd_spi.data = data;
			}
		}
		break;
		case LCD_SPI_LINE_3 :
		{
			if (data != hp49gp->m_lcd_spi.l3)
			{
				if ((data != 0) && (hp49gp->m_lcd_spi.l1 == 0))
				{
					verboselog( machine, 5, "LCD SPI write bit %d\n", hp49gp->m_lcd_spi.data ? 1 : 0);
					if (hp49gp->m_lcd_spi.bits < 8)
					{
						hp49gp->m_lcd_spi.shift = (hp49gp->m_lcd_spi.shift << 1) | (hp49gp->m_lcd_spi.data ? 1 : 0);
					}
					hp49gp->m_lcd_spi.bits++;
					if (hp49gp->m_lcd_spi.bits == 8)
					{
						verboselog( machine, 5, "LCD SPI write byte %02X\n", hp49gp->m_lcd_spi.shift);
					}
					else if (hp49gp->m_lcd_spi.bits == 9)
					{
						verboselog( machine, 5, "LCD SPI write ack %d\n", (hp49gp->m_lcd_spi.data ? 1 : 0));
					}
				}
				verboselog( machine, 5, "LCD_SPI_LINE_3 <- %d\n", data);
				hp49gp->m_lcd_spi.l3 = data;
			}
		}
		break;
	}
}

static int lcd_spi_line_r( running_machine &machine, int line)
{
	hp49gp_state *hp49gp = machine.driver_data<hp49gp_state>();
	switch (line)
	{
		case LCD_SPI_LINE_1 :
		{
			verboselog( machine, 7, "LCD_SPI_LINE_1 -> %d\n", hp49gp->m_lcd_spi.l1);
			return hp49gp->m_lcd_spi.l1;
		}
		break;
		case LCD_SPI_LINE_DATA :
		{
			verboselog( machine, 7, "LCD_SPI_LINE_DATA -> %d\n", hp49gp->m_lcd_spi.data);
			return hp49gp->m_lcd_spi.data;
		}
		break;
		case LCD_SPI_LINE_3 :
		{
			verboselog( machine, 7, "LCD_SPI_LINE_3 -> %d\n", hp49gp->m_lcd_spi.l3);
			return hp49gp->m_lcd_spi.l3;
		}
		break;
	}
	return 0;
}

// I/O PORT

static UINT32 s3c2410_gpio_port_r( device_t *device, int port, UINT32 mask)
{
	hp49gp_state *hp49gp = device->machine().driver_data<hp49gp_state>();
	UINT32 data = hp49gp->m_port[port];
	switch (port)
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
			data |= (lcd_spi_line_r( device->machine(), LCD_SPI_LINE_1) ? 1 : 0) << 9;
			data |= (lcd_spi_line_r( device->machine(), LCD_SPI_LINE_DATA) ? 1 : 0) << 12;
			data |= (lcd_spi_line_r( device->machine(), LCD_SPI_LINE_3) ? 1 : 0) << 13;
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
			if ((data & 0x02) == 0) data |= (device->machine().root_device().ioport( "ROW1")->read() << 8);
			if ((data & 0x04) == 0) data |= (device->machine().root_device().ioport( "ROW2")->read() << 8);
			if ((data & 0x08) == 0) data |= (device->machine().root_device().ioport( "ROW3")->read() << 8);
			if ((data & 0x10) == 0) data |= (device->machine().root_device().ioport( "ROW4")->read() << 8);
			if ((data & 0x20) == 0) data |= (device->machine().root_device().ioport( "ROW5")->read() << 8);
			if ((data & 0x40) == 0) data |= (device->machine().root_device().ioport( "ROW6")->read() << 8);
			if ((data & 0x80) == 0) data |= (device->machine().root_device().ioport( "ROW7")->read() << 8);
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

static void s3c2410_gpio_port_w( device_t *device, int port, UINT32 mask, UINT32 data)
{
	hp49gp_state *hp49gp = device->machine().driver_data<hp49gp_state>();
	hp49gp->m_port[port] = data;
	switch (port)
	{
		case S3C2410_GPIO_PORT_D :
		{
			lcd_spi_line_w( device->machine(), LCD_SPI_LINE_1, BIT( data, 9) ? 1 : 0);
			lcd_spi_line_w( device->machine(), LCD_SPI_LINE_DATA, BIT( data, 12) ? 1 : 0);
			lcd_spi_line_w( device->machine(), LCD_SPI_LINE_3, BIT( data, 13) ? 1 : 0);
		}
		break;
	}
}

// ...

static INPUT_CHANGED( port_changed )
{
	hp49gp_state *hp49gp = field.machine().driver_data<hp49gp_state>();
	s3c2410_request_eint( hp49gp->m_s3c2410, (FPTR)param + 8);
}

// ...

static MACHINE_START( hp49gp )
{
	hp49gp_state *hp49gp = machine.driver_data<hp49gp_state>();
	hp49gp->m_s3c2410 = machine.device( "s3c2410");
}

static MACHINE_RESET( hp49gp )
{
//  hp49gp_state *hp49gp = machine.driver_data<hp49gp_state>();
	devtag_reset( machine, "maincpu");
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
	UINT8 *rom = (UINT8 *)machine().root_device().memregion( "maincpu")->base();
	memcpy( m_steppingstone, rom, 1024);
	lcd_spi_init( machine());
}

static S3C2410_INTERFACE( hp49gp_s3c2410_intf )
{
	// CORE (pin read / pin write)
	{ NULL, NULL },
	// GPIO (port read / port write)
	{ s3c2410_gpio_port_r, s3c2410_gpio_port_w },
	// I2C (scl write / sda read / sda write)
	{ NULL, NULL, NULL },
	// ADC (data read)
	{ NULL },
	// I2S (data write)
	{ NULL },
	// NAND (command write / address write / data read / data write)
	{ NULL, NULL, NULL, NULL },
	// LCD (flags)
	{ S3C24XX_INTERFACE_LCD_REVERSE }
};

static MACHINE_CONFIG_START( hp49gp, hp49gp_state )
	MCFG_CPU_ADD("maincpu", ARM9, 400000000)
	MCFG_CPU_PROGRAM_MAP(hp49gp_map)

	MCFG_PALETTE_LENGTH(32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(160, 85)
	MCFG_SCREEN_VISIBLE_AREA(0, 131 - 1, 0, 80 - 1)
	MCFG_SCREEN_UPDATE_STATIC(s3c2410)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_VIDEO_START(s3c2410)

	MCFG_MACHINE_START(hp49gp)
	MCFG_MACHINE_RESET(hp49gp)

	MCFG_S3C2410_ADD("s3c2410", 12000000, hp49gp_s3c2410_intf)
MACHINE_CONFIG_END

static INPUT_PORTS_START( hp49gp )
	PORT_START("ROW1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)1) PORT_NAME("F1 | A | Y=") PORT_CODE(KEYCODE_F1) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)2) PORT_NAME("F2 | B | WIN") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)3) PORT_NAME("F3 | C | GRAPH") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)4) PORT_NAME("F4 | D | 2D/3D") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)5) PORT_NAME("F5 | E | TBLSET") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)6) PORT_NAME("F6 | F | TABLE") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0xDF, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("ROW7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED(port_changed, (void *)7) PORT_NAME("APPS | G | FILES | BEGIN") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
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

COMP(2009, hp49gp, 0, 0, hp49gp, hp49gp, hp49gp_state, hp49gp, "Hewlett Packard", "HP49G+", GAME_NOT_WORKING | GAME_NO_SOUND)
