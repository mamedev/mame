// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2sd.cpp

    Implementation of the AppleIISD card by Florian Reitz
    https://github.com/freitz85/AppleIISd

    AppleIISD has a Xilinx FPGA which implements a minimally hardware
    assisted SPI interface, and the SD card is thus driven in SPI
    mode rather than SD.

    The SPI controller is fixed to SPI Mode 3 only.
    (shift on falling CLK edges, shift-then-latch).

    Firmware is contained in an Atmel 28C64B parallel EEPROM, which
    has a Flash-style command set.

*********************************************************************/

#include "emu.h"
#include "a2sd.h"

#include "machine/at28c64b.h"
#include "machine/spi_sdcard.h"

#define LOG_SPI     (1U << 1)

//#define VERBOSE (LOG_GENERAL)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

static constexpr u8 C0N1_ECE        = 0x04; // external clock: toggles between 500 kHz internal clock and 1/2 of the 7M A2 bus clock
static constexpr u8 C0N1_FRX        = 0x10; // fast recieve: when enabled, both reads and writes of the SPI data register start a shift cycle
[[maybe_unused]] static constexpr u8 C0N1_BSY     = 0x20;
static constexpr u8 C0N1_TC         = 0x80; // SPI transfer complete

static constexpr u8 C0N3_CD         = 0x40; // card detect
static constexpr u8 C0N3_BIT_SS     = 7;

ROM_START( a2sd )
	ROM_REGION(0x2000, "flash", ROMREGION_ERASE00)
	ROM_LOAD( "appleiisd.bin", 0x000000, 0x000800, CRC(e82eea8a) SHA1(7e0acef01e622eeed6f8e87893d07c701bbef016) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_a2sd_device:
		public device_t,
		public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_a2sd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_a2sd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;
	virtual u8 read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, u8 data) override;

	// SPI 4-wire interface
	void spi_miso_w(int state) { m_in_bit = state; }

	TIMER_CALLBACK_MEMBER(shift_tick);

private:
	required_device<at28c64b_device> m_flash;
	required_device<spi_sdcard_device> m_sdcard;

	u8 m_datain, m_in_latch, m_out_latch;
	u8 m_c0n1, m_c0n3;
	int m_in_bit;

	int m_shift_count;
	emu_timer *m_shift_timer;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void a2bus_a2sd_device::device_add_mconfig(machine_config &config)
{
	AT28C64B(config, m_flash, 0);

	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set(FUNC(a2bus_a2sd_device::spi_miso_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_a2sd_device::device_rom_region() const
{
	return ROM_NAME( a2sd );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_a2sd_device::a2bus_a2sd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_flash(*this, "flash"),
	m_sdcard(*this, "sdcard"),
	m_datain(0), m_in_latch(0), m_out_latch(0), m_c0n1(0), m_c0n3(0x80), m_in_bit(0), m_shift_count(0)
{
}

a2bus_a2sd_device::a2bus_a2sd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_a2sd_device(mconfig, A2BUS_A2SD, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_a2sd_device::device_start()
{
	m_shift_timer = timer_alloc(FUNC(a2bus_a2sd_device::shift_tick), this);

	save_item(NAME(m_datain));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_c0n1));
	save_item(NAME(m_c0n3));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_shift_count));
}

void a2bus_a2sd_device::device_reset()
{
	m_shift_timer->adjust(attotime::never);
	m_shift_count = 0;
	m_sdcard->spi_clock_w(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(a2bus_a2sd_device::shift_tick)
{
	LOGMASKED(LOG_SPI, ">>>>>>> SHIFT %d (%c)\n", m_shift_count, (m_shift_count & 1) ? 'L' : 'S');
	if (!(m_shift_count & 1))
	{
		if (m_shift_count < 16)
		{
			m_out_latch <<= 1;
		}
		m_in_latch <<= 1;
		m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
		m_sdcard->spi_clock_w(CLEAR_LINE);
	}
	else
	{
		m_in_latch &= ~0x01;
		m_in_latch |= m_in_bit;
		m_sdcard->spi_clock_w(ASSERT_LINE);

		if (m_shift_count == 1)
		{
			m_datain = m_in_latch;
			LOGMASKED(LOG_SPI, "SPI: got %02x (in latch %02x)\n", m_datain, m_in_latch);
		}
	}

	m_shift_count--;
	if (m_shift_count == 0)
	{
		m_shift_timer->adjust(attotime::never);
		m_c0n1 |= C0N1_TC; // set TC
	}
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

u8 a2bus_a2sd_device::read_c0nx(u8 offset)
{
	switch (offset)
	{
		case 0:
			m_c0n1 &= ~C0N1_TC; // clear TC
			// if FRX is set, both reads and writes trigger a shift cycle
			if (m_c0n1 & C0N1_FRX)
			{
				m_c0n1 &= ~C0N1_TC; // clear TC
				m_shift_count = 16;
				m_out_latch = 0xff;

				if (m_c0n1 & C0N1_ECE)
				{
					m_shift_timer->adjust(attotime::from_hz(14.318181_MHz_XTAL / 4), 0, attotime::from_hz(14.318181_MHz_XTAL / 4));
				}
				else
				{
					m_shift_timer->adjust(attotime::from_hz(500_kHz_XTAL), 0, attotime::from_hz(500_kHz_XTAL));
				}
			}
			return m_datain;

		case 1:
			return m_c0n1;

		case 2:
			return 0;

		case 3:
			m_c0n3 &= ~C0N3_CD;
			m_c0n3 |= m_sdcard->get_card_present() ? 0 : C0N3_CD;   // bit is set if no card is present
			return m_c0n3;
	}
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_a2sd_device::write_c0nx(u8 offset, u8 data)
{
	switch (offset)
	{
		case 0:
			if (m_shift_count)
			{
				return;
			}
			LOGMASKED(LOG_SPI, "SPI sending %02x\n", data);
			m_out_latch = data;
			m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
			m_c0n1 &= ~C0N1_TC; // clear TC
			m_shift_count = 16;
			// if ECE is set, clock is 3.58 MHz from the A2 bus, otherwise internally generated 500 kHz
			if (m_c0n1 & C0N1_ECE)
			{
				m_shift_timer->adjust(attotime::from_hz(14.318181_MHz_XTAL / 4), 0, attotime::from_hz(14.318181_MHz_XTAL/4));
			}
			else
			{
				m_shift_timer->adjust(attotime::from_hz(500_kHz_XTAL), 0, attotime::from_hz(500_kHz_XTAL));
			}
			break;

		case 1:
			m_c0n1 &= 0xea;
			m_c0n1 |= (data & 0x15);
			break;

		case 2:
			break;

		case 3:
			m_c0n3 &= 0x9f;
			m_c0n3 |= (data & 0x91);

			m_sdcard->spi_ss_w(BIT(data, C0N3_BIT_SS));
			LOG("/SS is %x\n", BIT(data, C0N3_BIT_SS));
			break;

		default:
			logerror("a2sd: write %02x to c0n%x (%s)\n", data, offset, machine().describe_context().c_str());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

u8 a2bus_a2sd_device::read_cnxx(u8 offset)
{
	// slot image at 0
	return m_flash->read(offset);
}

void a2bus_a2sd_device::write_cnxx(u8 offset, u8 data)
{
	m_flash->write(offset, data);
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

u8 a2bus_a2sd_device::read_c800(u16 offset)
{
	return m_flash->read(offset + 0x100);
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_a2sd_device::write_c800(u16 offset, u8 data)
{
	m_flash->write(offset + 0x100, data);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_A2SD, device_a2bus_card_interface, a2bus_a2sd_device, "a2sd", "Apple II SD Card")
