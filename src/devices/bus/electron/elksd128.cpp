// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ElkSD128 Electron SD Interface

    SD Card Interface, 128K Memory Expansion & Joystick Adapter

    http://ramtop-retro.uk/elksd128.html

**********************************************************************/


#include "emu.h"
#include "elksd128.h"

#include "machine/spi_sdcard.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_elksd128_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_elksd128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_electron_expansion_interface implementation
	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_flash;
	required_device<spi_sdcard_device> m_sdcard;
	required_ioport m_joy;

	uint8_t m_romsel;
	uint8_t m_adc_channel;
	uint8_t m_swr_lock;

	TIMER_CALLBACK_MEMBER(spi_clock);

	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_clock_sysclk;
	int m_spi_clock_cycles;
	int m_in_bit;
	uint8_t m_in_latch;
	uint8_t m_out_latch;

	std::unique_ptr<uint8_t[]> m_ram;
};


static INPUT_PORTS_START( elksd128 )
	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_elksd128_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( elksd128 );
}


//-------------------------------------------------
//  ROM( elksd128 )
//-------------------------------------------------

ROM_START( elksd128 )
	ROM_REGION(0x80000, "flash", 0)
	ROM_LOAD("esd12815.rom", 0x0000, 0x80000, CRC(3ecf23ce) SHA1(d552149fec6a1deec2b75c740092bd311d67046f))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_elksd128_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set([this](int state) { m_in_bit = state; });
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *electron_elksd128_device::device_rom_region() const
{
	return ROM_NAME( elksd128 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_elksd128_device - constructor
//-------------------------------------------------

electron_elksd128_device::electron_elksd128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ELKSD128, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_flash(*this, "flash")
	, m_sdcard(*this, "sdcard")
	, m_joy(*this, "JOY")
	, m_romsel(0)
	, m_adc_channel(0)
	, m_swr_lock(0)
	, m_spi_clock_state(false)
	, m_spi_clock_sysclk(false)
	, m_spi_clock_cycles(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_elksd128_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x20000);

	m_spi_clock = timer_alloc(FUNC(electron_elksd128_device::spi_clock), this);

	save_item(NAME(m_romsel));
	save_item(NAME(m_adc_channel));
	save_item(NAME(m_swr_lock));
	save_pointer(NAME(m_ram), 0x20000);
	save_item(NAME(m_spi_clock_state));
	save_item(NAME(m_spi_clock_sysclk));
	save_item(NAME(m_spi_clock_cycles));
	save_item(NAME(m_in_bit));
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void electron_elksd128_device::device_reset()
{
	m_spi_clock->adjust(attotime::never);
	m_spi_clock_cycles = 0;
	m_spi_clock_state = false;
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_elksd128_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			data = m_ram[(m_romsel) << 14 | (offset & 0x3fff)];
			break;

		case 12:
			data = m_flash->base()[(m_romsel - 10) << 14 | (offset & 0x3fff)];
			break;

		case 14: case 15:
			data = m_flash->base()[(m_romsel - 14) << 14 | (offset & 0x3fff)];
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			switch (offset)
			{
			case 0xfc70: // Plus 1 analogue/digital converter
				switch (m_adc_channel)
				{
				case 0x04:
					if (!BIT(m_joy->read(), 3))
						data = 0x00;
					else if (!BIT(m_joy->read(), 2))
						data = 0xff;
					else
						data = 0x80;
					break;
				case 0x05:
					if (!BIT(m_joy->read(), 1))
						data = 0x00;
					else if (!BIT(m_joy->read(), 0))
						data = 0xff;
					else
						data = 0x80;
					break;
				case 0x06:
				case 0x07:
					data = 0x80;
					break;
				}
				break;

			case 0xfc72: // Plus 1 joystick and ADC status
				data = 0xaf | (m_joy->read() & 0x10);
				break;

			case 0xfc80: // SPI controller data port
				data = m_in_latch;
				break;

			case 0xfc81: // SPI controller status register
				data = m_spi_clock_cycles > 0 ? 0x01 : 0x00;
				break;

			case 0xfc83: // Device ID
				data = 0x80; // ElkSD128 rev 1.0
				break;

			case 0xfcc0: // First Byte interface
				data = m_joy->read() | 0xe0;
				break;

			case 0xfcd0: // Slogger interface
				data = m_joy->read() | 0xe0;
				break;
			}
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_elksd128_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0x8: case 0x9: case 0xa: case 0xb:
		switch (m_romsel)
		{
		case 0: case 1: case 2: case 3: case 4: case 5:
			m_ram[(m_romsel) << 14 | (offset & 0x3fff)] = data;
			break;
		case 6:
			if (!BIT(m_swr_lock, 6) || (offset & 0x3fff) > 0x3600)
				m_ram[(m_romsel) << 14 | (offset & 0x3fff)] = data;
			break;
		case 7:
			if (BIT(m_swr_lock, 7) || (offset & 0x3fff) > 0x3600)
				m_ram[(m_romsel) << 14 | (offset & 0x3fff)] = data;
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			switch (offset)
			{
			case 0xfc70: // Plus 1 analogue/digital converter
				m_adc_channel = data;
				break;

			case 0xfc80: // SPI controller data port
				m_out_latch = data;
				m_spi_clock_cycles = 8;

				if (m_spi_clock_sysclk) // TODO: confirm fast/slow clock dividers
					m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 2), 0, attotime::from_hz(16_MHz_XTAL / 2));
				else
					m_spi_clock->adjust(attotime::from_hz(16_MHz_XTAL / 32), 0, attotime::from_hz(16_MHz_XTAL / 32));
				break;

			case 0xfc81: // SPI controller clock register
				m_spi_clock_sysclk = bool(BIT(data, 0));
				break;

			case  0xfc82: // Sideways RAM lock
				m_swr_lock = data;
				break;
			}
			break;

		case 0xfe:
			if ((offset == 0xfe05) && !(data & 0xf0))
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}


TIMER_CALLBACK_MEMBER(electron_elksd128_device::spi_clock)
{
	if (m_spi_clock_cycles > 0)
	{
		m_sdcard->spi_ss_w(1);

		if (m_spi_clock_state)
		{
			m_in_latch <<= 1;
			m_in_latch &= ~0x01;
			m_in_latch |= m_in_bit;

			m_sdcard->spi_clock_w(1);

			m_spi_clock_cycles--;
		}
		else
		{
			m_sdcard->spi_mosi_w(BIT(m_out_latch, 7));
			m_sdcard->spi_clock_w(0);

			m_out_latch <<= 1;
		}

		m_spi_clock_state = !m_spi_clock_state;
	}
	else
	{
		m_spi_clock_state = false;
		m_spi_clock->adjust(attotime::never);
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ELECTRON_ELKSD128, device_electron_expansion_interface, electron_elksd128_device, "electron_elksd128", "ElkSD128 Electron SD Interface")
