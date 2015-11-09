// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    Palm Z22

    (C) 2011 Tim Schuerewegen

*******************************************************************************/

/*

Hardware ...

Samsung OCEAN-L-20      (SoC)
Samsung K9F5608U0D-JIB0 (NAND)
Samsung K4S2816321-UL75 (RAM)

The following reads are harmless ...

'maincpu' (3060017C): (GPIO) 56000080 -> 00010020
'maincpu' (30600960): unmapped program memory read from A0000000 & FFFFFFFF
'maincpu' (30600960): unmapped program memory read from A0000020 & FFFFFFFF
'maincpu' (30600960): unmapped program memory read from A0000040 & FFFFFFFF
...
'maincpu' (306411D4): (GPIO) 56000000 -> 005E03FF
'maincpu' (306411E0): (GPIO) 56000004 -> 0041F000
'maincpu' (30641258): (GPIO) 56000010 -> 00000015
'maincpu' (30641264): (GPIO) 56000010 -> 00000005
'maincpu' (306412C4): (GPIO) 56000040 -> 00155400
'maincpu' (306412D0): (GPIO) 56000040 -> 00155000
'maincpu' (306412DC): (GPIO) 56000044 -> 00000442
'maincpu' (306412F0): (GPIO) 56000000 -> 005E03FF
'maincpu' (306412FC): (GPIO) 56000000 -> 001E03FF
'maincpu' (30641308): (GPIO) 56000004 -> 0041F000
'maincpu' (30641320): (GPIO) 56000010 -> 00000025
'maincpu' (30641330): (GPIO) 56000010 -> 00000024

The following reads are NOT harmless ...

'maincpu' (3060022C): (GPIO) 56000044 -> 00000400 ... bits 2..1 is tested

if bits 2..1 = 0 then
  'maincpu' (30600240): (GPIO) 56000040 <- 00154400
  'maincpu' (3060024C): (GPIO) 56000044 <- 00000000
  'maincpu' (30600258): (GPIO) 56000048 <- 000007A0
else
  'maincpu' (30600268): (GPIO) 56000040 <- 00155400
  'maincpu' (30600274): (GPIO) 56000044 <- 00000440
  'maincpu' (30600280): (GPIO) 56000048 <- 000007E6
end

'maincpu' (3060004C): (GPIO) 56000054 -> 000000FF
'maincpu' (30600060): (GPIO) 56000054 -> 000000FF
'maincpu' (30600074): (GPIO) 56000054 -> 000000FF
'maincpu' (30600088): (GPIO) 56000054 -> 000000FF
'maincpu' (3060009C): (GPIO) 56000054 -> 000000FF
'maincpu' (306000B0): (GPIO) 56000054 -> 000000FF
'maincpu' (306000C4): (GPIO) 56000054 -> 000000FF
'maincpu' (306000D8): (GPIO) 56000054 -> 000000FF ... check if keys are pressed? (8 bits)

'maincpu' (3064116C): unmapped program memory read from 4D000068 & FFFFFFFF ... should read 0 or else stuck in a loop?

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2410.h"
#include "machine/smartmed.h"
#include "rendlay.h"

#define PALM_Z22_BATTERY_LEVEL  75

#define VERBOSE_LEVEL ( 0 )

class palmz22_state : public driver_device
{
public:
	palmz22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_s3c2410(*this, "s3c2410")
	{ }

	required_device<cpu_device> m_maincpu;

	required_device<s3c2410_device> m_s3c2410;
	nand_device *m_nand;

	UINT32 m_port[8];
	DECLARE_DRIVER_INIT(palmz22);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_INPUT_CHANGED_MEMBER(palmz22_input_changed);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	DECLARE_WRITE8_MEMBER( s3c2410_nand_command_w );
	DECLARE_WRITE8_MEMBER( s3c2410_nand_address_w );
	DECLARE_READ8_MEMBER( s3c2410_nand_data_r );
	DECLARE_WRITE8_MEMBER( s3c2410_nand_data_w );
	DECLARE_READ32_MEMBER(s3c2410_gpio_port_r);
	DECLARE_WRITE32_MEMBER(s3c2410_gpio_port_w);
	DECLARE_READ32_MEMBER(s3c2410_core_pin_r);
	DECLARE_READ32_MEMBER(s3c2410_adc_data_r );

};


inline void palmz22_state::verboselog(int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine().describe_context(), buf);
	}
}

/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// NAND

WRITE8_MEMBER( palmz22_state::s3c2410_nand_command_w )
{
	verboselog(9, "s3c2410_nand_command_w %02X\n", data);
	m_nand->command_w(data);
}

WRITE8_MEMBER( palmz22_state::s3c2410_nand_address_w )
{
	verboselog(9, "s3c2410_nand_address_w %02X\n", data);
	m_nand->address_w(data);
}

READ8_MEMBER( palmz22_state::s3c2410_nand_data_r )
{
	UINT8 data = m_nand->data_r();
	verboselog(9, "s3c2410_nand_data_r %02X\n", data);
	return data;
}

WRITE8_MEMBER( palmz22_state::s3c2410_nand_data_w )
{
	verboselog(9, "s3c2410_nand_data_w %02X\n", data);
	m_nand->data_w(data);
}

/*
READ8_MEMBER( palmz22_state::s3c2410_nand_busy_r )
{
    UINT8 data = m_nand->is_busy();
    verboselog(9, "s3c2410_nand_busy_r %02X\n", data);
    return data;
}
*/

// GPIO

READ32_MEMBER(palmz22_state::s3c2410_gpio_port_r)
{
	UINT32 data = m_port[offset];
	switch (offset)
	{
		case S3C2410_GPIO_PORT_E :
		{
			data = (data & ~(3 << 1)) | (0 << 1); // bits 2..1 = LCD related? (0 = clear 4D000010 bit 3, 1 = clear 560000000 bit 22)
			//data = 0x1F4;
		}
		break;
		case S3C2410_GPIO_PORT_F :
		{
			data = (data & ~0xFF) | (ioport( "PORT-F")->read() & 0xFF);
		}
		break;
		case S3C2410_GPIO_PORT_G :
		{
			data = (data & ~(1 << 1)) | (0 << 1); // bit 1 | 'maincpu' (200B56C0): (GPIO) 56000064 -> 00000000
			data = (data & ~(1 << 2)) | (0 << 2); // bit 2 | 'maincpu' (200B3EC4): (GPIO) 56000064 -> 00000000
		}
		break;
	}
	return data;
}

WRITE32_MEMBER(palmz22_state::s3c2410_gpio_port_w)
{
	m_port[offset] = data;
}

// CORE

/*

OM[1:0] = 00b : Enable NAND flash controller auto boot mode

NAND flash memory page size should be 512Bytes.

NCON : NAND flash memory address step selection
0 : 3 Step addressing
1 : 4 Step addressing

*/

READ32_MEMBER(palmz22_state::s3c2410_core_pin_r)
{
	int data = 0;
	switch (offset)
	{
		case S3C2410_CORE_PIN_NCON : data = 0; break;
		case S3C2410_CORE_PIN_OM0  : data = 0; break;
		case S3C2410_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// ADC

READ32_MEMBER(palmz22_state::s3c2410_adc_data_r )
{
	UINT32 data = 0;
	switch (offset)
	{
		case 0 + 0 : data = 0x2EE + (PALM_Z22_BATTERY_LEVEL * 0xFF / 100); break;
		case 0 + 1 : data = 0; break;
		case 2 + 0 : data = ioport( "PENX")->read(); break;
		case 2 + 1 : data = 0x3FF - ioport( "PENY")->read(); break;
	}
	verboselog(5,  "s3c2410_adc_data_r %08X\n", data);
	return data;
}

// INPUT

INPUT_CHANGED_MEMBER(palmz22_state::palmz22_input_changed)
{
	if (param == 0)
	{
		m_s3c2410->s3c2410_touch_screen( (newval & 0x01) ? 1 : 0);
	}
	else
	{
		m_s3c2410->s3c2410_request_eint( (FPTR)param - 1);
	}
}

// MACHINE

void palmz22_state::machine_start()
{
	m_nand = machine().device<nand_device>("nand");
	m_nand->set_data_ptr( memregion("nand")->base());
}

void palmz22_state::machine_reset()
{
	m_maincpu->reset();
	memset( m_port, 0, sizeof( m_port));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( palmz22_map, AS_PROGRAM, 32, palmz22_state )
	AM_RANGE(0x30000000, 0x31ffffff) AM_RAM
ADDRESS_MAP_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

DRIVER_INIT_MEMBER(palmz22_state,palmz22)
{
}

static MACHINE_CONFIG_START( palmz22, palmz22_state )
	MCFG_CPU_ADD("maincpu", ARM920T, 266000000)
	MCFG_CPU_PROGRAM_MAP(palmz22_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(160, 160)
	MCFG_SCREEN_VISIBLE_AREA(0, 160 - 1, 0, 160 - 1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_UPDATE_DEVICE("s3c2410", s3c2410_device, screen_update)

	MCFG_DEVICE_ADD("s3c2410", S3C2410, 12000000)
	MCFG_S3C2410_PALETTE("palette")
	MCFG_S3C2410_CORE_PIN_R_CB(READ32(palmz22_state, s3c2410_core_pin_r))
	MCFG_S3C2410_GPIO_PORT_R_CB(READ32(palmz22_state, s3c2410_gpio_port_r))
	MCFG_S3C2410_GPIO_PORT_W_CB(WRITE32(palmz22_state, s3c2410_gpio_port_w))
	MCFG_S3C2410_ADC_DATA_R_CB(READ32(palmz22_state, s3c2410_adc_data_r))
	MCFG_S3C2410_NAND_COMMAND_W_CB(WRITE8(palmz22_state, s3c2410_nand_command_w))
	MCFG_S3C2410_NAND_ADDRESS_W_CB(WRITE8(palmz22_state, s3c2410_nand_address_w))
	MCFG_S3C2410_NAND_DATA_R_CB(READ8(palmz22_state, s3c2410_nand_data_r))
	MCFG_S3C2410_NAND_DATA_W_CB(WRITE8(palmz22_state, s3c2410_nand_data_w))

	MCFG_DEVICE_ADD("nand", NAND, 0)
	MCFG_NAND_TYPE(NAND_CHIP_K9F5608U0D_J)
	MCFG_NAND_RNB_CALLBACK(DEVWRITELINE("s3c2410", s3c2410_device, frnb_w))
MACHINE_CONFIG_END

static INPUT_PORTS_START( palmz22 )
	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)0) PORT_PLAYER(2)
	PORT_START( "PENX" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_NAME("Pen X") PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1, 0, 0) PORT_KEYDELTA(30) PORT_PLAYER(2)
	PORT_START( "PENY" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_NAME("Pen Y") PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_KEYDELTA(30) PORT_PLAYER(2)
	PORT_START( "PORT-F" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1) PORT_NAME("Power")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1) PORT_NAME("Contacts")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1) PORT_NAME("Calendar")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1) PORT_NAME("Center")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, palmz22_state, palmz22_input_changed, (void *)1)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( palmz22 )
	ROM_REGION( 0x2100000, "nand", 0 )
	ROM_LOAD( "palmz22.bin", 0, 0x2100000, CRC(6d0320b3) SHA1(99297975fdad44faf69cc6eaf0fa2560d5579a4d) )
ROM_END

COMP(2005, palmz22, 0, 0, palmz22, palmz22, palmz22_state, palmz22, "Palm", "Palm Z22", MACHINE_NO_SOUND)
