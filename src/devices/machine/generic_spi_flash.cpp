// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "generic_spi_flash.h"

#define LOG_SPI (1U << 1)

#define VERBOSE     (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GENERIC_SPI_FLASH, generic_spi_flash_device, "generic_spi_flash", "Generic SPI Flash handling")

generic_spi_flash_device::generic_spi_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GENERIC_SPI_FLASH, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

void generic_spi_flash_device::device_start()
{
	save_item(NAME(m_spiaddr));
	save_item(NAME(m_spi_state));
	save_item(NAME(m_spilatch));
	save_item(NAME(m_spidir));
}

void generic_spi_flash_device::device_reset()
{
	m_spiaddr = 0;
	m_spi_state = 0;
	m_spilatch = 0;
	m_spidir = false;
}

void generic_spi_flash_device::write(uint8_t data)
{
	if (!m_spidir) // Send to SPI
	{
		switch (m_spi_state)
		{
		case READY_FOR_COMMAND:
			if (data == 0x03)
			{
				m_spi_state = READY_FOR_ADDRESS2;
			}
			else if (data == 0x05)
			{
				m_spi_state = READY_FOR_STATUS_READ;
			}
			else if (data == 0x0b)
			{
				m_spi_state = READY_FOR_HSADDRESS2;
			}
			else if (data == 0x06)
			{
				// write enable
				m_spi_state = READY_FOR_COMMAND;
			}
			else if (data == 0x04)
			{
				// write disable
				m_spi_state = READY_FOR_COMMAND;
			}
			else if (data == 0x02)
			{
				// page program
				m_spi_state = READY_FOR_WRITEADDRESS2;
			}
			else if (data == 0x20)
			{
				// erase 4k sector
				m_spi_state = READY_FOR_COMMAND;
			}
			else
			{
				fatalerror("SPI set to unknown mode %02x\n", data);
			}
			break;

		case READY_FOR_WRITEADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_WRITEADDRESS1;
			break;

		case READY_FOR_WRITEADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_WRITEADDRESS0;
			break;

		case READY_FOR_WRITEADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_WRITE;
			LOGMASKED(LOG_SPI, "SPI set to page WRITE mode with address %08x\n", m_spiaddr);
			break;

		case READY_FOR_WRITE:
			LOGMASKED(LOG_SPI, "Write SPI data %02x\n", data);

			m_spiptr[(m_spiaddr++) & (m_length-1)] = data;

			break;


		case READY_FOR_ADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_ADDRESS1;
			break;

		case READY_FOR_ADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_ADDRESS0;
			break;

		case READY_FOR_ADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_READ;
			m_spidir = 1;
			LOGMASKED(LOG_SPI, "SPI set to READ mode with address %08x\n", m_spiaddr);
			break;

		case READY_FOR_HSADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_HSADDRESS1;
			break;

		case READY_FOR_HSADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_HSADDRESS0;
			break;

		case READY_FOR_HSADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			m_spi_state = READY_FOR_HSDUMMY;
			break;

		case READY_FOR_HSDUMMY:
			m_spi_state = READY_FOR_READ;
			m_spidir = 1;
			LOGMASKED(LOG_SPI, "SPI set to High Speed READ mode with address %08x\n", m_spiaddr);
			break;

		case READY_FOR_SECTORERASEADDRESS2:
			m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16);
			m_spi_state = READY_FOR_SECTORERASEADDRESS1;
			break;

		case READY_FOR_SECTORERASEADDRESS1:
			m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8);
			m_spi_state = READY_FOR_SECTORERASEADDRESS0;
			break;

		case READY_FOR_SECTORERASEADDRESS0:
			m_spiaddr = (m_spiaddr & 0xffff00) | (data);
			LOGMASKED(LOG_SPI, "SPI set to Erase Sector with address %08x\n", m_spiaddr);
			break;

		}
	}
	else
	{
		if (m_spi_state == READY_FOR_READ)
		{
			m_spilatch = m_spiptr[(m_spiaddr++) & (m_length-1)];
		}
		else if (m_spi_state == READY_FOR_STATUS_READ)
		{
			m_spilatch = 0x00;
		}
		else
		{
			m_spilatch = 0x00;
		}
	}
}



void generic_spi_flash_device::nvram_default()
{
}

bool generic_spi_flash_device::nvram_read(util::read_stream &file)
{
	if (m_spiptr == nullptr)
	{
		return false;
	}

	auto const [err, actual] = util::read(file, m_spiptr, m_length);
	return !err && (actual == m_length);
}

bool generic_spi_flash_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_spiptr, m_length);
	return !err;
}

