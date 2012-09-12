/*******************************************************************************

    FriendlyARM Mini2440

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2440.h"
#include "machine/smartmed.h"
#include "sound/dac.h"
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
		logerror( "%s: %s", machine.describe_context(), buf);
	}
}

class mini2440_state : public driver_device
{
public:
	mini2440_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	device_t *m_s3c2440;
	nand_device *m_nand;
	dac_device *m_dac[2];

	UINT32 m_port[9];
	DECLARE_DRIVER_INIT(mini2440);
};

/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// GPIO

static UINT32 s3c2440_gpio_port_r( device_t *device, int port, UINT32 mask)
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	UINT32 data = state->m_port[port];
	switch (port)
	{
		case S3C2440_GPIO_PORT_G :
		{
			data = (data & ~(1 << 13)) | (1 << 13); // [nand] 1 = 2048 byte page  (if ncon = 1)
			data = (data & ~(1 << 14)) | (1 << 14); // [nand] 1 = 5 address cycle (if ncon = 1)
			data = (data & ~(1 << 15)) | (0 << 15); // [nand] 0 = 8-bit bus width (if ncon = 1)
			data = data | 0x8E9; // for "load Image of Linux..."
		}
		break;
	}
	return data;
}

static void s3c2440_gpio_port_w( device_t *device, int port, UINT32 mask, UINT32 data)
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	state->m_port[port] = data;
	switch (port)
	{
		case S3C2440_GPIO_PORT_B :
		{
			verboselog( device->machine(), 5,  "LED %d %d %d %d\n", BIT( data, 5), BIT( data, 6), BIT( data, 7), BIT( data, 8));
		}
		break;
	}
}

// CORE

/*

OM[1:0] = 00: Enable NAND flash memory boot

NCON: NAND flash memory selection (Normal / Advance)
0: Normal NAND flash (256Words/512Bytes page size, 3/4 address cycle)
1: Advance NAND flash (1KWords/2KBytes page size, 4/5 address cycle)

*/

static int s3c2440_core_pin_r( device_t *device, int pin)
{
	int data = 0;
	switch (pin)
	{
		case S3C2440_CORE_PIN_NCON : data = 1; break;
		case S3C2440_CORE_PIN_OM0  : data = 0; break;
		case S3C2440_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// NAND

static WRITE8_DEVICE_HANDLER( s3c2440_nand_command_w )
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	state->m_nand->command_w(data);
}

static WRITE8_DEVICE_HANDLER( s3c2440_nand_address_w )
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	state->m_nand->address_w(data);
}

static READ8_DEVICE_HANDLER( s3c2440_nand_data_r )
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	return state->m_nand->data_r();
}

static WRITE8_DEVICE_HANDLER( s3c2440_nand_data_w )
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	state->m_nand->data_w(data);
}

// I2S

static WRITE16_DEVICE_HANDLER( s3c2440_i2s_data_w )
{
	mini2440_state *state = device->machine().driver_data<mini2440_state>();
	state->m_dac[offset]->write_signed16(data + 0x8000);
}

// ADC

static READ32_DEVICE_HANDLER( s3c2440_adc_data_r )
{
	UINT32 data = 0;
	switch (offset)
	{
		case 2 + 0 : data = device->machine().root_device().ioport( "PENX")->read(); break;
		case 2 + 1 : data = 915 - device->machine().root_device().ioport( "PENY")->read() + 90; break;
	}
	verboselog( device->machine(), 5,  "s3c2440_adc_data_r %08X\n", data);
	return data;
}

// TOUCH

static INPUT_CHANGED( mini2440_input_changed )
{
	mini2440_state *state = field.machine().driver_data<mini2440_state>();
	s3c2440_touch_screen( state->m_s3c2440, (newval & 0x01) ? 1 : 0);
}

// ...

static MACHINE_START( mini2440 )
{
	mini2440_state *state = machine.driver_data<mini2440_state>();
	state->m_s3c2440 = machine.device("s3c2440");
	state->m_nand = machine.device<nand_device>("nand");
	state->m_dac[0] = machine.device<dac_device>("dac1");
	state->m_dac[1] = machine.device<dac_device>("dac2");
	state->m_nand->set_data_ptr(state->memregion("nand")->base());
}

static MACHINE_RESET( mini2440 )
{
	mini2440_state *state = machine.driver_data<mini2440_state>();
	machine.device("maincpu")->reset();
	memset( state->m_port, 0, sizeof( state->m_port));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( mini2440_map, AS_PROGRAM, 32, mini2440_state )
//  AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x30000000, 0x37ffffff) AM_RAM
ADDRESS_MAP_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

DRIVER_INIT_MEMBER(mini2440_state,mini2440)
{
	// do nothing
}

static S3C2440_INTERFACE( mini2440_s3c2440_intf )
{
	// CORE (pin read / pin write)
	{ s3c2440_core_pin_r, NULL },
	// GPIO (port read / port write)
	{ s3c2440_gpio_port_r, s3c2440_gpio_port_w },
	// I2C (scl write / sda read / sda write)
	{ NULL, NULL, NULL },
	// ADC (data read)
	{ s3c2440_adc_data_r },
	// I2S (data write)
	{ s3c2440_i2s_data_w },
	// NAND (command write / address write / data read / data write)
	{ s3c2440_nand_command_w, s3c2440_nand_address_w, s3c2440_nand_data_r, s3c2440_nand_data_w }
};

static NAND_INTERFACE( mini2440_nand_intf )
{
	NAND_CHIP_K9F1G08U0B,
	DEVCB_DEVICE_LINE( "s3c2440", s3c2440_pin_frnb_w)
};

static MACHINE_CONFIG_START( mini2440, mini2440_state )
	MCFG_CPU_ADD("maincpu", ARM920T, 400000000)
	MCFG_CPU_PROGRAM_MAP(mini2440_map)

	MCFG_PALETTE_LENGTH(32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 319)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_UPDATE_DEVICE("s3c2440", s3c2440_device, screen_update)

	MCFG_MACHINE_START(mini2440)
	MCFG_MACHINE_RESET(mini2440)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_S3C2440_ADD("s3c2440", 12000000, mini2440_s3c2440_intf)

	MCFG_NAND_ADD("nand", mini2440_nand_intf)
MACHINE_CONFIG_END

static INPUT_PORTS_START( mini2440 )
	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CHANGED(mini2440_input_changed, NULL) PORT_PLAYER(1)
	PORT_START( "PENX" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_NAME("Pen X") PORT_MINMAX(80, 950) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_KEYDELTA(30) PORT_PLAYER(1)
	PORT_START( "PENY" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_NAME("Pen Y") PORT_MINMAX(90, 915) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_KEYDELTA(30) PORT_PLAYER(1)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( mini2440 )
	ROM_REGION( 0x8400000, "nand", 0 )
	ROM_SYSTEM_BIOS( 0, "linux", "Linux 2.6.29.4-FriendlyARM + Qtopia 2.2.0 (2009/07/08)" )
	ROMX_LOAD( "linux.bin", 0, 0x8400000, CRC(7c98b249) SHA1(7c2e76edcbbcbfc3f3b0e53fb42d3e5c96e9a9fb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "wince", "Windows Embedded CE 6.00 (2011/03/14)" )
	ROMX_LOAD( "wince.bin", 0, 0x8400000, CRC(6acd56b8) SHA1(d039968820348fb1169827fa12b38b94e80a076f), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "android", "Android 1.5 (2009/05/13)" )
	ROMX_LOAD( "android.bin", 0, 0x8400000, CRC(4721837d) SHA1(88fcf553b106d9fc624c9615d9c1da9c705ccb46), ROM_BIOS(3) )
ROM_END

COMP(2009, mini2440, 0, 0, mini2440, mini2440, mini2440_state, mini2440, "FriendlyARM", "Mini2440", 0)
