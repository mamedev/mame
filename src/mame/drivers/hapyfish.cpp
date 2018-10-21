// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, R. Belmont, David Haywood
/*******************************************************************************

    Happy Fish 2 302-in-1

    Skeleton driver adapted from mini2440.cpp

    TODO: bootloader fails b/c it can't find the second NAND.  Hookup in
    s3c24xx may not be correct.

    To see bootloader messages, uncomment the UART_PRINTF define in s3c24xx.hxx.

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/s3c2440.h"
#include "machine/smartmed.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE_LEVEL ( 0 )

class hapyfish_state : public driver_device
{
public:
	hapyfish_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2440(*this, "s3c2440"),
		m_nand(*this, "nand"),
		m_nand2(*this, "nand2"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac")
		{ }

	void hapyfish(machine_config &config);

	void init_mini2440();

private:
	required_device<cpu_device> m_maincpu;
	required_device<s3c2440_device> m_s3c2440;
	required_device<nand_device> m_nand, m_nand2;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;

	uint32_t m_port[9];
	virtual void machine_start() override;
	virtual void machine_reset() override;
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

	void hapyfish_map(address_map &map);
};

inline void hapyfish_state::verboselog(int n_level, const char *s_fmt, ...)
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

READ32_MEMBER(hapyfish_state::s3c2440_gpio_port_r)
{
	uint32_t data = m_port[offset];
	verboselog(3, "Read GPIO @ %x\n", offset);
	switch (offset)
	{
		case S3C2440_GPIO_PORT_E:
			data = 0x80008000;
			break;

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

WRITE32_MEMBER(hapyfish_state::s3c2440_gpio_port_w)
{
	// tout2/gb2 -> uda1341ts l3mode
	// tout3/gb3 -> uda1341ts l3data
	// tclk0/gb4 -> uda1341ts l3clock

	//printf("%08x to GPIO @ %x\n", data, offset);

	m_port[offset] = data;
}

// CORE

/*

OM[1:0] = 00: Enable NAND flash memory boot

NCON: NAND flash memory selection (Normal / Advance)
0: Normal NAND flash (256Words/512Bytes page size, 3/4 address cycle)
1: Advance NAND flash (1KWords/2KBytes page size, 4/5 address cycle)

*/

READ32_MEMBER(hapyfish_state::s3c2440_core_pin_r)
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

WRITE8_MEMBER(hapyfish_state::s3c2440_nand_command_w )
{
	printf("%08x to NAND command\n", data);
	m_nand->command_w(data);
}

WRITE8_MEMBER(hapyfish_state::s3c2440_nand_address_w )
{
	printf("%08x to NAND address\n", data);
	m_nand->address_w(data);
}

READ8_MEMBER(hapyfish_state::s3c2440_nand_data_r )
{
	return m_nand->data_r();
}

WRITE8_MEMBER(hapyfish_state::s3c2440_nand_data_w )
{
	m_nand->data_w(data);
}

// I2S

WRITE16_MEMBER(hapyfish_state::s3c2440_i2s_data_w )
{
	if ( offset )
		m_ldac->write(data);
	else
		m_rdac->write(data);
}

// ADC

READ32_MEMBER(hapyfish_state::s3c2440_adc_data_r )
{
	uint32_t data = 0;
	return data;
}

// ...

void hapyfish_state::machine_start()
{
	m_nand->set_data_ptr(memregion("nand")->base());
	m_nand2->set_data_ptr(memregion("nand2")->base());
}

void hapyfish_state::machine_reset()
{
	m_maincpu->reset();
	memset( m_port, 0, sizeof( m_port));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void hapyfish_state::hapyfish_map(address_map &map)
{
	map(0x30000000, 0x37ffffff).ram();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void hapyfish_state::init_mini2440()
{
	// do nothing
}

MACHINE_CONFIG_START(hapyfish_state::hapyfish)
	MCFG_DEVICE_ADD("maincpu", ARM920T, 100000000)
	MCFG_DEVICE_PROGRAM_MAP(hapyfish_map)

	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1024, 768)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_SCREEN_UPDATE_DEVICE("s3c2440", s3c2440_device, screen_update)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	MCFG_DEVICE_ADD("ldac", UDA1341TS, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0) // uda1341ts.u12
	MCFG_DEVICE_ADD("rdac", UDA1341TS, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0) // uda1341ts.u12
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "ldac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "ldac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE(0, "rdac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "rdac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_DEVICE_ADD("s3c2440", S3C2440, 12000000)
	MCFG_S3C2440_PALETTE("palette")
	MCFG_S3C2440_SCREEN("screen")
	MCFG_S3C2440_CORE_PIN_R_CB(READ32(*this, hapyfish_state, s3c2440_core_pin_r))
	MCFG_S3C2440_GPIO_PORT_R_CB(READ32(*this, hapyfish_state, s3c2440_gpio_port_r))
	MCFG_S3C2440_GPIO_PORT_W_CB(WRITE32(*this, hapyfish_state, s3c2440_gpio_port_w))
	MCFG_S3C2440_ADC_DATA_R_CB(READ32(*this, hapyfish_state, s3c2440_adc_data_r))
	MCFG_S3C2440_I2S_DATA_W_CB(WRITE16(*this, hapyfish_state, s3c2440_i2s_data_w))
	MCFG_S3C2440_NAND_COMMAND_W_CB(WRITE8(*this, hapyfish_state, s3c2440_nand_command_w))
	MCFG_S3C2440_NAND_ADDRESS_W_CB(WRITE8(*this, hapyfish_state, s3c2440_nand_address_w))
	MCFG_S3C2440_NAND_DATA_R_CB(READ8(*this, hapyfish_state, s3c2440_nand_data_r))
	MCFG_S3C2440_NAND_DATA_W_CB(WRITE8(*this, hapyfish_state, s3c2440_nand_data_w))

	MCFG_DEVICE_ADD("nand", NAND, 0)
	MCFG_NAND_TYPE(K9F1G08U0B)
	MCFG_NAND_RNB_CALLBACK(WRITELINE("s3c2440", s3c2440_device, frnb_w))

	MCFG_DEVICE_ADD("nand2", NAND, 0)
	MCFG_NAND_TYPE(K9F1G08U0B)
MACHINE_CONFIG_END

static INPUT_PORTS_START( hapyfish )
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( hapyfsh2 )
	ROM_REGION( 0x84000000, "nand", 0 )
	ROM_LOAD( "flash.u6",        0x00000000, 0x84000000, CRC(3aa364a2) SHA1(fe09f549a937ecaf8f7a859522a6635e272fe714) )

	ROM_REGION( 0x84000000, "nand2", 0 )
	ROM_LOAD( "flash.u28",        0x00000000, 0x84000000, CRC(f00a25cd) SHA1(9c33f8e26b84cea957d9c37fb83a686b948c6834) )
ROM_END

GAME( 201?, hapyfsh2, 0, hapyfish, hapyfish, hapyfish_state, empty_init, ROT0, "bootleg", "Happy Fish (V2 PCB, 302-in-1)", MACHINE_IS_SKELETON )
