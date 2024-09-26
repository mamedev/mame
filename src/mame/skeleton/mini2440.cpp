// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*******************************************************************************

    FriendlyARM Mini2440

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/nandflash.h"
#include "machine/s3c2440.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"

#include <cstdarg>


namespace {

#define VERBOSE_LEVEL ( 0 )

class mini2440_state : public driver_device
{
public:
	mini2440_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_s3c2440(*this, "s3c2440"),
		m_nand(*this, "nand"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_penx(*this, "PENX"),
		m_peny(*this, "PENY") { }

	void mini2440(machine_config &config);

	void init_mini2440();

	DECLARE_INPUT_CHANGED_MEMBER(mini2440_input_changed);

private:
	required_device<cpu_device> m_maincpu;
	required_device<s3c2440_device> m_s3c2440;
	required_device<samsung_k9f1g08u0b_device> m_nand;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_ioport m_penx;
	required_ioport m_peny;

	uint32_t m_port[9] = { };
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
	uint32_t s3c2440_gpio_port_r(offs_t offset);
	void s3c2440_gpio_port_w(offs_t offset, uint32_t data);
	uint32_t s3c2440_core_pin_r(offs_t offset);
	void s3c2440_nand_command_w(uint8_t data);
	void s3c2440_nand_address_w(uint8_t data);
	uint8_t s3c2440_nand_data_r();
	void s3c2440_nand_data_w(uint8_t data);
	void s3c2440_i2s_data_w(offs_t offset, uint16_t data);
	uint32_t s3c2440_adc_data_r(offs_t offset);

	void mini2440_map(address_map &map) ATTR_COLD;
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

uint32_t mini2440_state::s3c2440_gpio_port_r(offs_t offset)
{
	uint32_t data = m_port[offset];
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

void mini2440_state::s3c2440_gpio_port_w(offs_t offset, uint32_t data)
{
	// tout2/gb2 -> uda1341ts l3mode
	// tout3/gb3 -> uda1341ts l3data
	// tclk0/gb4 -> uda1341ts l3clock

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

uint32_t mini2440_state::s3c2440_core_pin_r(offs_t offset)
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

void mini2440_state::s3c2440_nand_command_w(uint8_t data)
{
	m_nand->command_w(data);
}

void mini2440_state::s3c2440_nand_address_w(uint8_t data)
{
	m_nand->address_w(data);
}

uint8_t mini2440_state::s3c2440_nand_data_r()
{
	return m_nand->data_r();
}

void mini2440_state::s3c2440_nand_data_w(uint8_t data)
{
	m_nand->data_w(data);
}

// I2S

void mini2440_state::s3c2440_i2s_data_w(offs_t offset, uint16_t data)
{
	if ( offset )
		m_ldac->write(data);
	else
		m_rdac->write(data);
}

// ADC

uint32_t mini2440_state::s3c2440_adc_data_r(offs_t offset)
{
	uint32_t data = 0;
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
}

void mini2440_state::machine_reset()
{
	m_maincpu->reset();
	memset( m_port, 0, sizeof( m_port));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void mini2440_state::mini2440_map(address_map &map)
{
//  map(0x00000000, 0x001fffff).rom();
	map(0x30000000, 0x37ffffff).ram();
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void mini2440_state::init_mini2440()
{
	// do nothing
}

void mini2440_state::mini2440(machine_config &config)
{
	ARM920T(config, m_maincpu, 400000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mini2440_state::mini2440_map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1024, 768);
	screen.set_visarea(0, 239, 0, 319);
	screen.set_screen_update("s3c2440", FUNC(s3c2440_device::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	UDA1341TS(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // uda1341ts.u12
	UDA1341TS(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // uda1341ts.u12

	S3C2440(config, m_s3c2440, 12000000);
	m_s3c2440->set_palette_tag("palette");
	m_s3c2440->set_screen_tag("screen");
	m_s3c2440->core_pin_r_callback().set(FUNC(mini2440_state::s3c2440_core_pin_r));
	m_s3c2440->gpio_port_r_callback().set(FUNC(mini2440_state::s3c2440_gpio_port_r));
	m_s3c2440->gpio_port_w_callback().set(FUNC(mini2440_state::s3c2440_gpio_port_w));
	m_s3c2440->adc_data_r_callback().set(FUNC(mini2440_state::s3c2440_adc_data_r));
	m_s3c2440->i2s_data_w_callback().set(FUNC(mini2440_state::s3c2440_i2s_data_w));
	m_s3c2440->nand_command_w_callback().set(FUNC(mini2440_state::s3c2440_nand_command_w));
	m_s3c2440->nand_address_w_callback().set(FUNC(mini2440_state::s3c2440_nand_address_w));
	m_s3c2440->nand_data_r_callback().set(FUNC(mini2440_state::s3c2440_nand_data_r));
	m_s3c2440->nand_data_w_callback().set(FUNC(mini2440_state::s3c2440_nand_data_w));

	SAMSUNG_K9F1G08U0B(config, m_nand, 0);
	m_nand->rnb_wr_callback().set(m_s3c2440, FUNC(s3c2440_device::frnb_w));
}

static INPUT_PORTS_START( mini2440 )
	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CHANGED_MEMBER(DEVICE_SELF, mini2440_state, mini2440_input_changed, 0) PORT_PLAYER(1)
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
	ROMX_LOAD( "linux.bin", 0, 0x8400000, CRC(7c98b249) SHA1(7c2e76edcbbcbfc3f3b0e53fb42d3e5c96e9a9fb), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "wince", "Windows Embedded CE 6.00 (2011/03/14)" )
	ROMX_LOAD( "wince.bin", 0, 0x8400000, CRC(6acd56b8) SHA1(d039968820348fb1169827fa12b38b94e80a076f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "android", "Android 1.5 (2009/05/13)" )
	ROMX_LOAD( "android.bin", 0, 0x8400000, CRC(4721837d) SHA1(88fcf553b106d9fc624c9615d9c1da9c705ccb46), ROM_BIOS(2) )
ROM_END

} // anonymous namespace


COMP(2009, mini2440, 0, 0, mini2440, mini2440, mini2440_state, init_mini2440, "FriendlyARM", "Mini2440", 0)
