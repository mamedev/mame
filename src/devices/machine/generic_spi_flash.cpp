// license:BSD-3-Clause
// copyright-holders:David Haywood

// HLE-like implementation for SPI Flash ROMs using byte interface rather than SPI signals

#include "emu.h"
#include "generic_spi_flash.h"

#define LOG_SPI (1U << 1)

#define VERBOSE     (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GENERIC_SPI_FLASH, generic_spi_flash_device, "generic_spi_flash", "Generic Byte HLE SPI Flash handling")

generic_spi_flash_device::generic_spi_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GENERIC_SPI_FLASH, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_multibyte_status_read(1)
	, m_multibyte_status_write(1)
{
	m_idbytes[0] = 0xfe;
	m_idbytes[1] = 0xfe;
	m_idbytes[2] = 0x00;
}

void generic_spi_flash_device::device_start()
{
	save_item(NAME(m_spiaddr));
	save_item(NAME(m_spi_state));
	save_item(NAME(m_spilatch));
}

void generic_spi_flash_device::device_reset()
{
	m_spiaddr = 0;
	m_spi_state = 0;
	m_spilatch = 0;
}

void generic_spi_flash_device::get_command(u8 data)
{
	if (data == COMMAND_01_WRSR)
	{
		LOGMASKED(LOG_SPI, "Set SPI to WRSR, 1 or 2 params required\n");
		m_spi_state = COMMAND_01_WRSR;
	}
	else if (data == COMMAND_9F_RDID)
	{
		LOGMASKED(LOG_SPI, "Set SPI to RDID (Read Identification)\n");
		m_spi_state = COMMAND_9F_RDID;
	}
	else if (data == COMMAND_03_READ)
	{
		LOGMASKED(LOG_SPI, "Set SPI to READ (normal - 3 params needed)\n");
		m_spi_state = COMMAND_03_READ;
	}
	else if (data == COMMAND_05_RDSR)
	{
		LOGMASKED(LOG_SPI, "Set SPI to RDSR (Read Status Register)\n");
		m_spi_state = COMMAND_05_RDSR;
	}
	else if (data == COMMAND_0B_FAST_READ)
	{
		LOGMASKED(LOG_SPI, "Set SPI to FAST READ (fast - 4 params needed)\n");
		m_spi_state = COMMAND_0B_FAST_READ;
	}
	else if (data == COMMAND_06_WREN)
	{
		LOGMASKED(LOG_SPI, "Set SPI to WREN (Write Enable)\n");
		m_spi_state = READY_FOR_COMMAND;
	}
	else if (data == COMMAND_04_WRDI)
	{
		LOGMASKED(LOG_SPI, "Set SPI to WRDI (Write Disable)\n");
		m_spi_state = READY_FOR_COMMAND;
	}
	else if (data == COMMAND_02_PP)
	{
		LOGMASKED(LOG_SPI, "Set SPI to PP (Page Program)\n");
		m_spi_state = COMMAND_02_PP;
	}
	else if (data == COMMAND_20_SE)
	{
		LOGMASKED(LOG_SPI, "Set SPI to SE (Sector Erase)\n");
		m_spi_state = COMMAND_20_SE;
	}
	else if (data == COMMAND_90_REMS)
	{
		LOGMASKED(LOG_SPI, "Set SPI to REMS (Read Electronic Manufacturer & Device ID)\n");
		m_spi_state = COMMAND_90_REMS;
	}
	else if (data == COMMAND_AB_RDP)
	{
		LOGMASKED(LOG_SPI, "Set SPI to RDP (Release from deep power down)\n");
		m_spi_state = READY_FOR_COMMAND;
	}
	else if (data == COMMAND_B9_DP)
	{
		LOGMASKED(LOG_SPI, "Set SPI to DP (deep power down)\n");
		m_spi_state = READY_FOR_COMMAND;
	}
	else if (data == COMMAND_EB_4READ)
	{
		LOGMASKED(LOG_SPI, "Set SPI to 4READ (Quad I/O read with configurable dummy bytes)\n");
		m_spi_state = COMMAND_EB_4READ;
	}
	else
	{
		fatalerror("SPI set to unknown/unhandled command %02x\n", data);
	}

	m_spi_state_step = 0;
}

void generic_spi_flash_device::process_read_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16); m_spi_state_step++;
		break;
	case 0x01:
		m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8); m_spi_state_step++;
		break;
	case 0x02:
		m_spiaddr = (m_spiaddr & 0xffff00) | (data); m_spi_state_step++;
		break;
	default:
		m_spilatch = m_spiptr[(m_spiaddr++) & (m_length - 1)];
		break;
	}
}

void generic_spi_flash_device::process_hsread_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16); m_spi_state_step++;
		break;
	case 0x01:
		m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8); m_spi_state_step++;
		break;
	case 0x02:
		m_spiaddr = (m_spiaddr & 0xffff00) | (data); m_spi_state_step++;
		break;
	case 0x03:
		/* dummy */  m_spi_state_step++;
		break;
	default:
		m_spilatch = m_spiptr[(m_spiaddr++) & (m_length - 1)];
		break;
	}
}

// has configurable dummy bytes?
void generic_spi_flash_device::process_read4_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16); m_spi_state_step++;
		break;
	case 0x01:
		m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8); m_spi_state_step++;
		break;
	case 0x02:
		m_spiaddr = (m_spiaddr & 0xffff00) | (data); m_spi_state_step++;
		break;
	case 0x03: case 0x04: case 0x05:
		/* dummy */  m_spi_state_step++;
		break;
	default:
		m_spilatch = m_spiptr[(m_spiaddr++) & (m_length - 1)];
		break;
	}
}

void generic_spi_flash_device::process_write_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16); m_spi_state_step++;
		break;
	case 0x01:
		m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8); m_spi_state_step++;
		break;
	case 0x02:
		m_spiaddr = (m_spiaddr & 0xffff00) | (data); m_spi_state_step++;
		break;
	default:
		LOGMASKED(LOG_SPI, "Write SPI data %02x\n", data);
		m_spiptr[(m_spiaddr++) & (m_length - 1)] = data;
		break;
	}
}

void generic_spi_flash_device::process_sector_erase_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spiaddr = (m_spiaddr & 0x00ffff) | (data << 16); m_spi_state_step++;
		break;
	case 0x01:
		m_spiaddr = (m_spiaddr & 0xff00ff) | (data << 8); m_spi_state_step++;
		break;
	case 0x02:
		m_spiaddr = (m_spiaddr & 0xffff00) | (data); m_spi_state_step++;
		LOGMASKED(LOG_SPI, "SPI set to Erase Sector with address %08x\n", m_spiaddr);
		break;
	default:
		LOGMASKED(LOG_SPI, "%s unexpected byte %02x when writing sector erase address\n", data);
		break;
	}
}

void generic_spi_flash_device::process_status_write_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		LOGMASKED(LOG_SPI, "status write step 1\n");
		if (m_multibyte_status_write != 0)
			m_spi_state_step++;
		else
			m_spi_state = READY_FOR_COMMAND;
		break;

	case 0x01:
		LOGMASKED(LOG_SPI, "status write step 2\n");
		m_spi_state = READY_FOR_COMMAND;
		break;
	}
}

void generic_spi_flash_device::process_status_read_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		LOGMASKED(LOG_SPI, "status read step 1\n");
		m_spilatch = 0x00;
		if (m_multibyte_status_read != 0)
			m_spi_state_step++;
		else
			m_spi_state = READY_FOR_COMMAND;
		break;

	case 0x01:
		LOGMASKED(LOG_SPI, "status read step 2\n");
		m_spilatch = 0x00;
		m_spi_state = READY_FOR_COMMAND;
		break;
	}
}


void generic_spi_flash_device::process_status_rems_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		LOGMASKED(LOG_SPI, "REMS step 1\n");
		m_spi_state_step++;
		break;

	case 0x01:
		LOGMASKED(LOG_SPI, "REMS step 2\n");
		m_spi_state_step++;
		break;

	case 0x02:
		LOGMASKED(LOG_SPI, "REMS step 3\n");
		m_spi_state_step++;
		break;

	case 0x03:
		LOGMASKED(LOG_SPI, "REMS step 4\n");
		m_spi_state_step++;
		break;

	case 0x04:
		LOGMASKED(LOG_SPI, "REMS step 5\n");
		m_spi_state = READY_FOR_COMMAND;
		break;
	}
}

void generic_spi_flash_device::process_status_rdid_command(u8 data)
{
	switch (m_spi_state_step)
	{
	case 0x00:
		m_spilatch = m_idbytes[0];
		m_spi_state_step++;
		break;

	case 0x01:
		m_spilatch = m_idbytes[1];
		m_spi_state_step++;
		break;

	case 0x02:
		m_spilatch = m_idbytes[2];
		m_spi_state = READY_FOR_COMMAND;
		break;
	}
}

void generic_spi_flash_device::write(u8 data)
{
	// not all commands have extra params/reads
	switch (m_spi_state)
	{
	case READY_FOR_COMMAND:
		get_command(data);
		break;

	case COMMAND_01_WRSR:
		process_status_write_command(data);
		break;

	case COMMAND_02_PP:
		process_write_command(data);
		break;

	case COMMAND_03_READ:
		process_read_command(data);
		break;

	case COMMAND_05_RDSR:
		process_status_read_command(data);
		break;

	case COMMAND_0B_FAST_READ:
		process_hsread_command(data);
		break;

	case COMMAND_20_SE:
		process_sector_erase_command(data);
		break;

	case COMMAND_90_REMS:
		process_status_rems_command(data);
		break;

	case COMMAND_9F_RDID:
		process_status_rdid_command(data);
		break;

	case COMMAND_EB_4READ:
		process_read4_command(data);
		break;
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

