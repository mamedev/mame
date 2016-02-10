// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
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

class mini2440_state : public driver_device
{
public:
	mini2440_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2440(*this, "s3c2440"),
		m_nand(*this, "nand"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_penx(*this, "PENX"),
		m_peny(*this, "PENY") { }

	required_device<cpu_device> m_maincpu;
	required_device<s3c2440_device> m_s3c2440;
	required_device<nand_device> m_nand;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	required_ioport m_penx;
	required_ioport m_peny;

	UINT32 m_port[9];
	DECLARE_DRIVER_INIT(mini2440);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_INPUT_CHANGED_MEMBER(mini2440_input_changed);
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	DECLARE_READ32_MEMBER(s3c2440_gpio_port_r);
	DECLARE_WRITE32_MEMBER(s3c2440_gpio_port_w);
	DECLARE_READ32_MEMBER(s3c2440_core_pin_r);
	DECLARE_WRITE8_MEMBER(s3c2440_nand_command_w );
	DECLARE_WRITE8_MEMBER(s3c2440_nand_address_w );
	DECLARE_READ8_MEMBER(s3c2440_nand_data_r );
	DECLARE_WRITE8_MEMBER(s3c2440_nand_data_w );
	DECLARE_WRITE16_MEMBER(s3c2440_i2s_data_w );
	DECLARE_READ32_MEMBER(s3c2440_adc_data_r );

};

inline void mini2440_state::verboselog(int n_level, const char *s_fmt, ...)
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

// GPIO

READ32_MEMBER(mini2440_state::s3c2440_gpio_port_r)
{
	UINT32 data = m_port[offset];
	switch (offset)
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

WRITE32_MEMBER(mini2440_state::s3c2440_gpio_port_w)
{
	m_port[offset] = data;
	switch (offset)
	{
		case S3C2440_GPIO_PORT_B :
		{
			verboselog(5,  "LED %d %d %d %d\n", BIT( data, 5), BIT( data, 6), BIT( data, 7), BIT( data, 8));
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

READ32_MEMBER(mini2440_state::s3c2440_core_pin_r)
{
	int data = 0;
	switch (offset)
	{
		case S3C2440_CORE_PIN_NCON : data = 1; break;
		case S3C2440_CORE_PIN_OM0  : data = 0; break;
		case S3C2440_CORE_PIN_OM1  : data = 0; break;
	}
	return data;
}

// NAND

WRITE8_MEMBER(mini2440_state::s3c2440_nand_command_w )
{
	m_nand->command_w(data);
}

WRITE8_MEMBER(mini2440_state::s3c2440_nand_address_w )
{
	m_nand->address_w(data);
}

READ8_MEMBER(mini2440_state::s3c2440_nand_data_r )
{
	return m_nand->data_r();
}

WRITE8_MEMBER(mini2440_state::s3c2440_nand_data_w )
{
	m_nand->data_w(data);
}

// I2S

WRITE16_MEMBER(mini2440_state::s3c2440_i2s_data_w )
{
	if ( offset )
		m_dac1->write_signed16(data + 0x8000);
	else
		m_dac2->write_signed16(data + 0x8000);
}

// ADC

READ32_MEMBER(mini2440_state::s3c2440_adc_data_r )
{
	UINT32 data = 0;
	switch (offset)
	{
		case 2 + 0 : data = m_penx->read(); break;
		case 2 + 1 : data = 915 - m_peny->read() + 90; break;
	}
	verboselog(5,  "s3c2440_adc_data_r %08X\n", data);
	return data;
}

// TOUCH

INPUT_CHANGED_MEMBER(mini2440_state::mini2440_input_changed)
{
	m_s3c2440->s3c2440_touch_screen( (newval & 0x01) ? 1 : 0);
}

// ...

void mini2440_state::machine_start()
{
	m_nand->set_data_ptr(memregion("nand")->base());
}

void mini2440_state::machine_reset()
{
	m_maincpu->reset();
	memset( m_port, 0, sizeof( m_port));
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

static MACHINE_CONFIG_START( mini2440, mini2440_state )
	MCFG_CPU_ADD("maincpu", ARM920T, 400000000)
	MCFG_CPU_PROGRAM_MAP(mini2440_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 319)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_UPDATE_DEVICE("s3c2440", s3c2440_device, screen_update)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac2", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_DEVICE_ADD("s3c2440", S3C2440, 12000000)
	MCFG_S3C2440_PALETTE("palette")
	MCFG_S3C2440_CORE_PIN_R_CB(READ32(mini2440_state, s3c2440_core_pin_r))
	MCFG_S3C2440_GPIO_PORT_R_CB(READ32(mini2440_state, s3c2440_gpio_port_r))
	MCFG_S3C2440_GPIO_PORT_W_CB(WRITE32(mini2440_state, s3c2440_gpio_port_w))
	MCFG_S3C2440_ADC_DATA_R_CB(READ32(mini2440_state, s3c2440_adc_data_r))
	MCFG_S3C2440_I2S_DATA_W_CB(WRITE16(mini2440_state, s3c2440_i2s_data_w))
	MCFG_S3C2440_NAND_COMMAND_W_CB(WRITE8(mini2440_state, s3c2440_nand_command_w))
	MCFG_S3C2440_NAND_ADDRESS_W_CB(WRITE8(mini2440_state, s3c2440_nand_address_w))
	MCFG_S3C2440_NAND_DATA_R_CB(READ8(mini2440_state, s3c2440_nand_data_r))
	MCFG_S3C2440_NAND_DATA_W_CB(WRITE8(mini2440_state, s3c2440_nand_data_w))

	MCFG_DEVICE_ADD("nand", NAND, 0)
	MCFG_NAND_TYPE(NAND_CHIP_K9F1G08U0B)
	MCFG_NAND_RNB_CALLBACK(DEVWRITELINE("s3c2440", s3c2440_device, frnb_w))
MACHINE_CONFIG_END

static INPUT_PORTS_START( mini2440 )
	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CHANGED_MEMBER(DEVICE_SELF, mini2440_state, mini2440_input_changed, NULL) PORT_PLAYER(1)
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
