// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 RAM with SPI/SDI/SQI/QPI interface

                _____
        _CE  1 |*    |  8  VDD
  SO/SIO[1]  2 |     |  7  SIO[3]
     SIO[2]  3 |     |  6  SCLK
        VSS  4 |_____|  5  SI/SIO[0]

                _____
        _CE  1 |*    | 14  VDD
  SO/SIO[1]  2 |     | 13  SIO[3]
     SIO[2]  3 |     | 12  SCLK
        VSS  4 |     | 11  SI/SIO[0]
         NC  5 |     | 10  VBAT
         NC  6 |     |  9  NC
         NC  7 |_____|  8  NC

 Made by various manufacturers.

 Example SRAM:
 * ISS IS62WVS1288F (128 KiB)
 * ISS IS65WVS1288F (128 KiB, automotive)
 * Microchip 23AA02M (256 KiB)
 * Microchip 23AA04M (512 KiB)
 * Microchip 23LCV02M (256 KiB, battery backup)
 * Microchip 23LCV04M (512 KiB, battery backup)

 Example PSRAM:
 * AP Memory APS1604L-SQ (2 MiB)
 * AP Memory APS3204L-SQ (4 MiB)
 * AP Memory APS6404L-SQ (8 MiB)
 * AP Memory APS12804L-SQ (16 MiB)
 * ISS IS66WVS1M8 (1 MiB)
 * ISS IS66WVS2M8 (2 MiB)
 * ISS IS66WVS8M8 (8 MiB)
 * ISS IS67WVS1M8 (1 MiB, automotive)
 * ISS IS67WVS2M8 (2 MiB, automotive)
 * ISS IS67WVS8M8 (8 MiB, automotive)
 * Vilsion Tech VTI7064 (8 MiB)

 Reading/writing single bytes in SPI mode is compatible across device
 families, but additional functionality is not.

 Currently only SPI reads and writes are implemented in a way that's
 compatible with all device families for small transfers.

 TODO:
 * Implement additional functionality for each device family
 */
#include "emu.h"
#include "spi_psram.h"


DEFINE_DEVICE_TYPE(SPI_PSRAM, spi_psram_device, "spi_psram", "Generic SPI RAM")


ALLOW_SAVE_TYPE(spi_psram_device::phase)

enum class spi_psram_device::phase : u8
{
	IDLE,
	COMMAND,
	ADDRESS,
	WAIT,
	READ,
	WRITE
};


enum spi_psram_device::command : u8
{
	COMMAND_READ            = 0x03,
	COMMAND_FAST_READ       = 0x0b, // 8 wait cycles in SPI mode, 4 wait cycles in QPI mode
	COMMAND_FAST_READ_QUAD  = 0xeb, // 6 wait cycles, always 4-bit address and data

	COMMAND_WRITE           = 0x02,
	COMMAND_WRITE_QUAD      = 0x38, // always 4-bit address and data

	COMMAND_QPI_ENTER       = 0x35,
	COMMAND_QPI_EXIT        = 0xf5,

	COMMAND_SDI_ENTER       = 0x3b,
	COMMAND_SQI_ENTER       = 0x38,
	COMMAND_SDI_SQI_RESET   = 0xff,

	COMMAND_RESET_ENABLE    = 0x66, // must be immediately followed by 0x99
	COMMAND_RESET           = 0x99, // must be immediately preceded by 0x66

	COMMAND_WRAP_BOUNDARY   = 0xc0, // toggle between 1024-byte wrap and 32-byte wrap

	COMMAND_MR_READ         = 0x05, // read mode/status register (size depends on family)
	COMMAND_MR_WRITE        = 0x01, // write mode/status register (size depends on family)

	COMMAND_READ_ID         = 0x9f, // returns 64-bit info

	COMMAND_DEEP_PD_ENTER   = 0xb9  // enter deep power-down mode
};


spi_psram_device::spi_psram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SPI_PSRAM, tag, owner, clock),
	m_sio_cb(*this),
	m_ram(),
	m_size(0)
{
}

spi_psram_device::~spi_psram_device()
{
}


void spi_psram_device::ce_w(int state)
{
	if (state)
	{
		m_sio_cb(0, 0xf, 0x0);
		m_phase = phase::IDLE;
	}
	else if (m_ce)
	{
		m_buffer = 0;
		m_data_width = m_cmd_width;
		m_bits = 8;
		m_phase = phase::COMMAND;
	}
	m_ce = state ? 1 : 0;
}

void spi_psram_device::sclk_w(int state)
{
	switch (m_phase)
	{
	case phase::COMMAND:
	case phase::ADDRESS:
	case phase::WRITE:
		if (state && !m_sclk)
		{
			m_buffer = (m_buffer << m_cmd_width) | (m_sio & util::make_bitmask<u8>(m_cmd_width));
			m_bits -= m_data_width;
			if (!m_bits)
			{
				if (phase::COMMAND == m_phase)
				{
					m_cmd = u8(m_buffer);
					start_command();
				}
				else if (phase::ADDRESS == m_phase)
				{
					m_addr = m_buffer & (m_size - 1);
					address_complete();
				}
				else
				{
					m_ram[m_addr] = u8(m_buffer);
					m_buffer = 0;
					next_address();
				}
			}
		}
		break;

	case phase::READ:
		if (!state && m_sclk)
		{
			if (1 == m_data_width)
			{
				m_sio_cb(0, BIT(m_buffer, 7) ? 0xf : 0xd, 0x2);
			}
			else
			{
				u8 const mask = make_bitmask<u8>(m_data_width);
				u8 const sio = ((m_buffer >> (8 - m_data_width)) & mask) | (0x0f ^ mask);
				m_sio_cb(0, sio, mask);
			}
			m_buffer = (m_buffer << m_data_width) & 0xff;
			m_bits -= m_data_width;
			if (!m_bits)
			{
				next_address();
				m_buffer = m_ram[m_addr];
				m_bits = 8;
			}
		}
		break;

	default:
		break;
	}
	m_sclk = state ? 1 : 0;
}

void spi_psram_device::sio_w(offs_t offset, u8 data, u8 mem_mask)
{
	m_sio = data & 0xf;
}


void spi_psram_device::device_validity_check(validity_checker &valid) const
{
	if (!m_size || (m_size & (m_size - 1)) || (m_size > 0x0100'0000))
		osd_printf_error("Unsupported size %u (must be a power of 2 not larger than 16M)\n", m_size);
}

void spi_psram_device::device_resolve_objects()
{
	m_wrap_mask = util::make_bitmask<u32>(10);
	m_addr = 0;
	m_buffer = 0;
	m_cmd_width = 1;
	m_data_width = 1;
	m_bits = 0;

	m_ce = 1;
	m_sclk = 0;
	m_sio = 0xf;
	m_phase = phase::IDLE;
	m_cmd = 0;
}

void spi_psram_device::device_start()
{
	if (!m_size || (m_size & (m_size - 1)) || (m_size > 0x0100'0000))
		osd_printf_error("%s: Unsupported size %u (must be a power of 2 not larger than 16M)\n", tag(), m_size);

	m_ram = make_unique_clear<u8 []>(m_size);

	save_pointer(NAME(m_ram), m_size);
	save_item(NAME(m_wrap_mask));
	save_item(NAME(m_addr));
	save_item(NAME(m_buffer));
	save_item(NAME(m_cmd_width));
	save_item(NAME(m_data_width));
	save_item(NAME(m_bits));
	save_item(NAME(m_ce));
	save_item(NAME(m_sclk));
	save_item(NAME(m_sio));
	save_item(NAME(m_phase));
	save_item(NAME(m_cmd));
}


void spi_psram_device::start_command()
{
	switch (m_cmd)
	{
	case COMMAND_READ:
		// FIXME: AP Memory devices don't support this command in QPI mode
		m_buffer = 0;
		m_data_width = m_cmd_width;
		m_bits = 24;
		m_phase = phase::ADDRESS;
		break;

	case COMMAND_WRITE:
		m_buffer = 0;
		m_data_width = m_cmd_width;
		m_bits = 24;
		m_phase = phase::ADDRESS;
		break;

	default:
		logerror("unimplemented command 0x%02x\n", m_cmd);
		m_phase = phase::IDLE;
	}
}

void spi_psram_device::address_complete()
{
	switch (m_cmd)
	{
	case COMMAND_READ:
		// FIXME: wait cycles depend on mode and device family
		m_buffer = m_ram[m_addr];
		m_data_width = m_cmd_width;
		m_bits = 8;
		m_phase = phase::READ;
		break;

	case COMMAND_WRITE:
		m_buffer = 0;
		m_data_width = m_cmd_width;
		m_bits = 8;
		m_phase = phase::WRITE;
		break;

	default:
		throw false; // if we get here, there's a bug in the code
	}
}

inline void spi_psram_device::next_address()
{
	if (m_wrap_mask)
	{
		m_addr = (m_addr & ~m_wrap_mask) | ((m_addr + 1) & m_wrap_mask);
		m_bits = 8;
	}
	else
	{
		m_phase = phase::IDLE;
	}
}
