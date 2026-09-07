// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SD2IEC emulation

**********************************************************************/

#include "emu.h"
#include "sd2iec.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define ATMEGA1284P_TAG "maincpu"
#define PCF8583_TAG     "rtc"
#define EEPROM_TAG      "eeprom"
#define EEPROM_SIZE     0x1000



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SD2IEC, sd2iec_device, "sd2iec", "SD2IEC")


//-------------------------------------------------
//  ROM( sd2iec )
//-------------------------------------------------

ROM_START( sd2iec )
	ROM_REGION( 0x1f000, ATMEGA1284P_TAG, 0 )
	ROM_LOAD( "sd2iec-1.0.0atentdead0-38-g7d0be2e-larsp-m1284p.bin", 0x00000, 0x1f000, CRC(cfdd03b8) SHA1(32b15de3948f4995db002797359a323a9ef0b5d2) )

	ROM_REGION( EEPROM_SIZE, EEPROM_TAG, ROMREGION_ERASEFF )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *sd2iec_device::device_rom_region() const
{
	return ROM_NAME( sd2iec );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sd2iec_device::device_add_mconfig(machine_config &config)
{
	ATMEGA1284(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sd2iec_device::main_map);
	m_maincpu->set_addrmap(AS_DATA, &sd2iec_device::data_map);
	m_maincpu->set_eeprom_tag(EEPROM_TAG);
	m_maincpu->gpio_in<atmega1284_device::GPIOA>().set(FUNC(sd2iec_device::pa_r));
	m_maincpu->gpio_out<atmega1284_device::GPIOA>().set(FUNC(sd2iec_device::pa_w));
	m_maincpu->gpio_in<atmega1284_device::GPIOB>().set(FUNC(sd2iec_device::pb_r));
	m_maincpu->gpio_out<atmega1284_device::GPIOB>().set(FUNC(sd2iec_device::pb_w));
	m_maincpu->gpio_in<atmega1284_device::GPIOC>().set(FUNC(sd2iec_device::pc_r));
	m_maincpu->gpio_out<atmega1284_device::GPIOC>().set(FUNC(sd2iec_device::pc_w));
	m_maincpu->gpio_in<atmega1284_device::GPIOD>().set(FUNC(sd2iec_device::pd_r));

	PCF8583(config, m_rtc, XTAL(32'768));

	SPI_SDCARD(config, m_sdcard);
	m_sdcard->spi_miso_callback().set(FUNC(sd2iec_device::sdcard_miso_w));
	m_sdcard->card_present_callback().set(m_maincpu, FUNC(atmega1284_device::pd_w<2>));
}

void sd2iec_device::sdcard_miso_w(int state)
{
	m_sdcard_miso = state;
}

void sd2iec_device::cbm_iec_atn(int state)
{
	m_maincpu->pc_w<0>(state);
}

void sd2iec_device::cbm_iec_clk(int state)
{
	m_maincpu->pc_w<2>(state);
}

void sd2iec_device::cbm_iec_data(int state)
{
	m_maincpu->pc_w<1>(state);
}

void sd2iec_device::main_map(address_map &map)
{
	map(0x00000, 0x1efff).rom();
}

void sd2iec_device::data_map(address_map &map)
{
	map(0x0100, 0x40ff).ram();
}

uint8_t sd2iec_device::pa_r()
{
	/*

		bit		description

		PA0
		PA1
		PA2		DEVICE 8/9
		PA3		DEVICE 10/11
		PA4
		PA5
		PA6
		PA7

	*/

	return ((m_slot->get_address() & 0x3) ^ 0x03) << 2;
}

void sd2iec_device::pa_w(uint8_t data)
{
	/*

		bit		description

		PA0		LED_GREEN
		PA1		LED_RED
		PA2
		PA3
		PA4
		PA5
		PA6
		PA7

	*/

	m_leds[0] = BIT(data, 0);
	m_leds[1] = BIT(data, 1);
}

uint8_t sd2iec_device::pb_r()
{
	/*

		bit		description

		PB0
		PB1
		PB2
		PB3
		PB4
		PB5
		PB6		SD_DATA_IN (MISO)
		PB7

	*/

	return m_sdcard_miso << 6;
}

void sd2iec_device::pb_w(uint8_t data)
{
		/*

		bit		description

		PB0
		PB1
		PB2
		PB3
		PB4		SD_CARD_SELECT (SS)
		PB5		SD_DATA_OUT (MOSI)
		PB6
		PB7		SD_CLOCK (SCK)

	*/

	m_sdcard->spi_ss_w(!BIT(data, 4));
	m_sdcard->spi_mosi_w(BIT(data, 5));
	m_sdcard->spi_clock_w(BIT(data, 7));
}

uint8_t sd2iec_device::pc_r()
{
	/*

		bit		description

		PC0		IEC_ATN
		PC1		IEC_DATA
		PC2		IEC_CLK
		PC3
		PC4		1
		PC5		RTC SDA
		PC6		1
		PC7

	*/

	u8 data = 0x50;

	data |= m_bus->atn_r();
	data |= m_bus->data_r() << 1;
	data |= m_bus->clk_r() << 2;

	data |= m_rtc->sda_r() << 5;

	return data;
}

void sd2iec_device::pc_w(uint8_t data)
{
	/*

		bit		description

		PC0		IEC_ATN
		PC1		IEC_DATA
		PC2		IEC_CLK
		PC3
		PC4
		PC5		RTC SDA
		PC6		RTC SCL
		PC7		JP8.2

	*/

	m_bus->atn_w(this, BIT(data, 0));
	m_bus->data_w(this, BIT(data, 1));
	m_bus->clk_w(this, BIT(data, 2));

	m_rtc->sda_w(BIT(data, 5));
	m_rtc->scl_w(BIT(data, 6));
}

uint8_t sd2iec_device::pd_r()
{
	/*

		bit		description

		PD0
		PD1
		PD2		SD_CARD_DETECT
		PD3
		PD4
		PD5
		PD6		SD_WRITE_PROTECT
		PD7

	*/

	u8 data = 0;

	data |= (!m_sdcard->get_card_present()) << 2;

	return data;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sd2iec_device - constructor
//-------------------------------------------------

sd2iec_device::sd2iec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SD2IEC, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_maincpu(*this, ATMEGA1284P_TAG),
	m_rtc(*this, PCF8583_TAG),
	m_sdcard(*this, "sdcard"),
	m_leds(*this, "led%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sd2iec_device::device_start()
{
	m_eeprom = memregion(EEPROM_TAG)->base();
}


//-------------------------------------------------
//  device_nvram_interface overrides
//-------------------------------------------------

void sd2iec_device::nvram_default()
{
	std::fill_n(m_eeprom, EEPROM_SIZE, 0xff);
}

bool sd2iec_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_eeprom, EEPROM_SIZE);
	return !err && actual == EEPROM_SIZE;
}

bool sd2iec_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_eeprom, EEPROM_SIZE);
	return !err;
}
