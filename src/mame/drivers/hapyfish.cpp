// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen, R. Belmont, David Haywood
/*******************************************************************************

    Happy Fish 2 302-in-1

    Skeleton driver adapted from mini2440.cpp

    TODO: Linux kernel + initrd loads now but not much happens afterwards.

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

#include <algorithm>

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
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("%s: %s", machine().describe_context(), buf);
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
	if ((m_port[8] & 0x1800) != 0)
	{
		m_nand->command_w(data);
	}
	else
	{
		m_nand2->command_w(data);
	}
}

WRITE8_MEMBER(hapyfish_state::s3c2440_nand_address_w )
{
	if ((m_port[8] & 0x1800) != 0)
	{
		m_nand->address_w(data);
	}
	else
	{
		m_nand2->address_w(data);
	}
}

READ8_MEMBER(hapyfish_state::s3c2440_nand_data_r )
{
	if ((m_port[8] & 0x1800) != 0)
	{
		return m_nand->data_r();
	}

	return m_nand2->data_r();
}

WRITE8_MEMBER(hapyfish_state::s3c2440_nand_data_w )
{
	if ((m_port[8] & 0x1800) != 0)
	{
		m_nand->data_w(data);
		return;
	}

	m_nand2->data_w(data);
}

// I2S

WRITE16_MEMBER(hapyfish_state::s3c2440_i2s_data_w )
{
	if (offset)
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
	m_port[8] = 0x1800; // select NAND #1 (S3C2440 bootloader will happen before machine_reset())
}

void hapyfish_state::machine_reset()
{
	m_maincpu->reset();
	std::fill(std::begin(m_port), std::end(m_port), 0);
	m_port[8] = 0x1800; // select NAND #1
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

void hapyfish_state::hapyfish(machine_config &config)
{
	ARM920T(config, m_maincpu, 100000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hapyfish_state::hapyfish_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1024, 768);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update("s3c2440", FUNC(s3c2440_device::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	UDA1341TS(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // uda1341ts.u12
	UDA1341TS(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // uda1341ts.u12
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "ldac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "rdac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);

	S3C2440(config, m_s3c2440, 12000000);
	m_s3c2440->set_palette_tag("palette");
	m_s3c2440->set_screen_tag("screen");
	m_s3c2440->core_pin_r_callback().set(FUNC(hapyfish_state::s3c2440_core_pin_r));
	m_s3c2440->gpio_port_r_callback().set(FUNC(hapyfish_state::s3c2440_gpio_port_r));
	m_s3c2440->gpio_port_w_callback().set(FUNC(hapyfish_state::s3c2440_gpio_port_w));
	m_s3c2440->adc_data_r_callback().set(FUNC(hapyfish_state::s3c2440_adc_data_r));
	m_s3c2440->i2s_data_w_callback().set(FUNC(hapyfish_state::s3c2440_i2s_data_w));
	m_s3c2440->nand_command_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_command_w));
	m_s3c2440->nand_address_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_address_w));
	m_s3c2440->nand_data_r_callback().set(FUNC(hapyfish_state::s3c2440_nand_data_r));
	m_s3c2440->nand_data_w_callback().set(FUNC(hapyfish_state::s3c2440_nand_data_w));

	NAND(config, m_nand, 0);
	m_nand->set_nand_type(nand_device::chip::K9LAG08U0M);
	m_nand->rnb_wr_callback().set(m_s3c2440, FUNC(s3c2440_device::frnb_w));

	NAND(config, m_nand2, 0);
	m_nand2->set_nand_type(nand_device::chip::K9LAG08U0M);
	m_nand2->rnb_wr_callback().set(m_s3c2440, FUNC(s3c2440_device::frnb_w));
}

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
