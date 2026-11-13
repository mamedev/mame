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
#include "machine/nandflash.h"
#include "machine/s3c24xx.h"
#include "screen.h"

#include <cstdarg>

#define LOG_ADC		(1U << 1)
#define LOG_NAND	(1U << 2)

#define VERBOSE ( 0 )
#include "logmacro.h"


namespace {

class palmz22_state : public driver_device
{
public:
	palmz22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s3c2410(*this, "s3c2410")
		, m_nand(*this, "nand")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

	void palmz22(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void nand_command_w(uint8_t data);
	void nand_address_w(uint8_t data);
	uint8_t nand_data_r();
	void nand_data_w(uint8_t data);
	uint32_t gpio_port_r(offs_t offset);
	void gpio_port_w(offs_t offset, uint32_t data);
	uint32_t adc_data_r(offs_t offset);

	void map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<s3c2410_device> m_s3c2410;
	required_device<samsung_k9f5608u0dj_device> m_nand;

	uint32_t m_port[8];

	static constexpr int BATTERY_LEVEL = 75;
};


/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// NAND

void palmz22_state::nand_command_w(uint8_t data)
{
	LOGMASKED(LOG_NAND, "nand_command_w %02x\n", data);
	m_nand->command_w(data);
}

void palmz22_state::nand_address_w(uint8_t data)
{
	LOGMASKED(LOG_NAND, "nand_address_w %02x\n", data);
	m_nand->address_w(data);
}

uint8_t palmz22_state::nand_data_r()
{
	uint8_t data = m_nand->data_r();
	LOGMASKED(LOG_NAND, "nand_data_r %02x\n", data);
	return data;
}

void palmz22_state::nand_data_w(uint8_t data)
{
	LOGMASKED(LOG_NAND, "nand_data_w %02x\n", data);
	m_nand->data_w(data);
}

/*
uint8_t palmz22_state::nand_busy_r()
{
    uint8_t data = m_nand->is_busy();
    LOGMASKED(LOG_NAND, "nand_busy_r %02X\n", data);
    return data;
}
*/

// GPIO

uint32_t palmz22_state::gpio_port_r(offs_t offset)
{
	uint32_t data = m_port[offset];
	switch (offset)
	{
	case s3c24xx_device::GPIO_PORT_E:
		data = (data & ~(3 << 1)) | (0 << 1); // bits 2..1 = LCD related? (0 = clear 4D000010 bit 3, 1 = clear 560000000 bit 22)
		//data = 0x1F4;
		break;
	case s3c24xx_device::GPIO_PORT_F:
		data = (data & ~0xff) | (ioport("PORT-F")->read() & 0xff);
		break;
	case s3c24xx_device::GPIO_PORT_G:
		data = (data & ~(1 << 1)) | (0 << 1); // bit 1 | 'maincpu' (200B56C0): (GPIO) 56000064 -> 00000000
		data = (data & ~(1 << 2)) | (0 << 2); // bit 2 | 'maincpu' (200B3EC4): (GPIO) 56000064 -> 00000000
		break;
	}
	return data;
}

void palmz22_state::gpio_port_w(offs_t offset, uint32_t data)
{
	m_port[offset] = data;
}

// ADC

uint32_t palmz22_state::adc_data_r(offs_t offset)
{
	uint32_t data = 0;
	switch (offset)
	{
	case 0 + 0 : data = 0x2ee + (BATTERY_LEVEL * 0xff / 100); break;
	case 0 + 1 : data = 0; break;
	case 2 + 0 : data = ioport("PENX")->read(); break;
	case 2 + 1 : data = 0x3ff - ioport("PENY")->read(); break;
	}
	LOGMASKED(LOG_ADC, "adc_data_r %08x\n", data);
	return data;
}

// INPUT

INPUT_CHANGED_MEMBER(palmz22_state::input_changed)
{
	if (param == 0)
	{
		m_s3c2410->touch_screen(BIT(newval, 0));
	}
	else
	{
		m_s3c2410->gpio_set_extint(0, newval ? 1 : 0);
	}
}

// MACHINE

void palmz22_state::machine_start()
{
	save_item(NAME(m_port));
}

void palmz22_state::machine_reset()
{
	m_maincpu->reset();
	memset(m_port, 0, sizeof(m_port));
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void palmz22_state::map(address_map &map)
{
	map(0x30000000, 0x31ffffff).ram();
	map(0x40000000, 0x5fffffff).m(m_s3c2410, FUNC(s3c2410_device::map));
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void palmz22_state::palmz22(machine_config &config)
{
	ARM920T(config, m_maincpu, 266000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &palmz22_state::map);

	PALETTE(config, "palette").set_entries(32768);

	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(160, 160);
	screen.set_visarea(0, 160 - 1, 0, 160 - 1);
	screen.set_screen_update("s3c2410", FUNC(s3c2410_device::screen_update));

	S3C2410(config, m_s3c2410, 12000000);
	m_s3c2410->set_palette_tag("palette");
	m_s3c2410->set_screen_tag("screen");
	m_s3c2410->om0_r_callback().set_constant(0); // OM[1:0] = 00: Enable NAND flash memory boot
	m_s3c2410->om1_r_callback().set_constant(0);
	m_s3c2410->ncon_r_callback().set_constant(0); // NCON = 0: Normal NAND flash (256Words/512Bytes page size, 3/4 address cycle)
	m_s3c2410->gpio_port_r_callback().set(FUNC(palmz22_state::gpio_port_r));
	m_s3c2410->gpio_port_w_callback().set(FUNC(palmz22_state::gpio_port_w));
	m_s3c2410->adc_data_r_callback().set(FUNC(palmz22_state::adc_data_r));
	m_s3c2410->nand_command_w_callback().set(FUNC(palmz22_state::nand_command_w));
	m_s3c2410->nand_address_w_callback().set(FUNC(palmz22_state::nand_address_w));
	m_s3c2410->nand_data_r_callback().set(FUNC(palmz22_state::nand_data_r));
	m_s3c2410->nand_data_w_callback().set(FUNC(palmz22_state::nand_data_w));
	m_s3c2410->nand_update_ecc_r_callback().set_constant(1);

	SAMSUNG_K9F5608U0DJ(config, m_nand);
	m_nand->rnb_wr_callback().set(m_s3c2410, FUNC(s3c2410_device::frnb_w));
}

static INPUT_PORTS_START( palmz22 )
	PORT_START( "PENB" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pen Button") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 0) PORT_PLAYER(2)
	PORT_START( "PENX" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_X ) PORT_NAME("Pen X") PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1, 0, 0) PORT_KEYDELTA(30) PORT_PLAYER(2)
	PORT_START( "PENY" )
	PORT_BIT( 0x3ff, 0x200, IPT_LIGHTGUN_Y ) PORT_NAME("Pen Y") PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_KEYDELTA(30) PORT_PLAYER(2)
	PORT_START( "PORT-F" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5        ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1) PORT_NAME("Power")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1) PORT_NAME("Contacts")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1) PORT_NAME("Calendar")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1) PORT_NAME("Center")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(palmz22_state::input_changed), 1)
INPUT_PORTS_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

ROM_START( palmz22 )
	ROM_REGION( 0x2100000, "nand", 0 )
	ROM_LOAD( "palmz22.bin", 0, 0x2100000, CRC(6d0320b3) SHA1(99297975fdad44faf69cc6eaf0fa2560d5579a4d) )
ROM_END

} // anonymous namespace


COMP( 2005, palmz22, 0, 0, palmz22, palmz22, palmz22_state, empty_init, "Palm", "Palm Z22", MACHINE_NO_SOUND)
