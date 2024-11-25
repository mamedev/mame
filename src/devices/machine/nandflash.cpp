// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    NAND flash emulation

    References:
    Datasheets for various SmartMedia chips were found on Samsung and Toshiba's
    sites (http://www.toshiba.com/taec and
    http://www.samsung.com/Products/Semiconductor/Flash/FlashCard/SmartMedia)

    Raphael Nabet 2004
*/

#include "emu.h"
#include "nandflash.h"

#include "formats/imageutl.h"

ALLOW_SAVE_TYPE(nand_device::sm_mode_t)
ALLOW_SAVE_TYPE(nand_device::pointer_sm_mode_t)

DEFINE_DEVICE_TYPE(NAND, nand_device, "nand", "NAND Flash Memory")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F5608U0D,  samsung_k9f5608u0d_device,  "samsung_k9f5608u0d",  "Samsung K9F5608U0D")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F5608U0DJ, samsung_k9f5608u0dj_device, "samsung_k9f5608u0dj", "Samsung K9F5608U0D-J")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F5608U0B,  samsung_k9f5608u0b_device,  "samsung_k9f5608u0b",  "Samsung K9F5608U0B")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F2808U0B,  samsung_k9f2808u0b_device,  "samsung_k9f2808u0b",  "Samsung K9F2808U0B")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F1G08U0B,  samsung_k9f1g08u0b_device,  "samsung_k9f1g08u0b",  "Samsung K9F1G08U0B")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F1G08U0M,  samsung_k9f1g08u0m_device,  "samsung_k9f1g08u0m",  "Samsung K9F1G08U0M")
DEFINE_DEVICE_TYPE(SAMSUNG_K9LAG08U0M,  samsung_k9lag08u0m_device,  "samsung_k9lag08u0m",  "Samsung K9LAG08U0M")
DEFINE_DEVICE_TYPE(SAMSUNG_K9F2G08U0M,  samsung_k9f2g08u0m_device,  "samsung_k9f2g08u0m",  "Samsung K9F2G08U0M")
DEFINE_DEVICE_TYPE(TOSHIBA_TC58256AFT,  toshiba_tc58256aft_device,  "toshiba_tc58256aft",  "Toshiba TC58256AFT")

nand_device::nand_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, NAND, tag, owner, clock)
{
}

nand_device::nand_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_page_data_size(0),
	m_page_total_size(0),
	m_num_pages(0),
	m_log2_pages_per_block(0),
	m_pagereg(nullptr),
	m_id_len(0),
	m_col_address_cycles(0),
	m_row_address_cycles(0),
	m_sequential_row_read(0),
	m_write_rnb(*this)
{
	memset(m_id, 0, sizeof(m_id));
}

samsung_k9f5608u0d_device::samsung_k9f5608u0d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F5608U0D, tag, owner, clock)
{
	m_id_len = 2;
	m_id[0] = 0xec;
	m_id[1] = 0x75;
	m_page_data_size = 512;
	m_page_total_size = 512 + 16;
	m_log2_pages_per_block = compute_log2(32);
	m_num_pages = 32 * 2048;
	m_col_address_cycles = 1;
	m_row_address_cycles = 2;
	m_sequential_row_read = 1;
}

samsung_k9f5608u0dj_device::samsung_k9f5608u0dj_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F5608U0DJ, tag, owner, clock)
{
	m_id_len = 2;
	m_id[0] = 0xec;
	m_id[1] = 0x75;
	m_page_data_size = 512;
	m_page_total_size = 512 + 16;
	m_log2_pages_per_block = compute_log2(32);
	m_num_pages = 32 * 2048;
	m_col_address_cycles = 1;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

samsung_k9f5608u0b_device::samsung_k9f5608u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F5608U0B, tag, owner, clock)
{
	m_id_len = 2;
	m_id[0] = 0xec;
	m_id[1] = 0x75;
	m_page_data_size = 512;
	m_page_total_size = 512 + 16;
	m_log2_pages_per_block = compute_log2(32);
	m_num_pages = 32 * 2048;
	m_col_address_cycles = 1;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

samsung_k9f2808u0b_device::samsung_k9f2808u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F2808U0B, tag, owner, clock)
{
	m_id_len = 2;
	m_id[0] = 0xec;
	m_id[1] = 0x73;
	m_page_data_size = 512;
	m_page_total_size = 512 + 16;
	m_log2_pages_per_block = compute_log2(32);
	m_num_pages = 32 * 1024;
	m_col_address_cycles = 1;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

samsung_k9f1g08u0b_device::samsung_k9f1g08u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F1G08U0B, tag, owner, clock)
{
	m_id_len = 5;
	m_id[0] = 0xec;
	m_id[1] = 0xf1;
	m_id[2] = 0x00;
	m_id[3] = 0x95;
	m_id[4] = 0x40;
	m_page_data_size = 2048;
	m_page_total_size = 2048 + 64;
	m_log2_pages_per_block = compute_log2(64);
	m_num_pages = 64 * 1024;
	m_col_address_cycles = 2;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

samsung_k9f1g08u0m_device::samsung_k9f1g08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F1G08U0M, tag, owner, clock)
{
	m_id_len = 4;
	m_id[0] = 0xec;
	m_id[1] = 0xf1;
	m_id[2] = 0x00;
	m_id[3] = 0x15;
	m_page_data_size = 2048;
	m_page_total_size = 2048 + 64;
	m_log2_pages_per_block = compute_log2(64);
	m_num_pages = 64 * 1024;
	m_col_address_cycles = 2;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

samsung_k9lag08u0m_device::samsung_k9lag08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9LAG08U0M, tag, owner, clock)
{
	m_id_len = 5;
	m_id[0] = 0xec;
	m_id[1] = 0xd5;
	m_id[2] = 0x55;
	m_id[3] = 0x25;
	m_id[4] = 0x68;
	m_page_data_size = 2048;
	m_page_total_size = 2048 + 64;
	m_log2_pages_per_block = compute_log2(128);
	m_num_pages = 128 * 8192;
	m_col_address_cycles = 2;
	m_row_address_cycles = 3;
	m_sequential_row_read = 0;
}

samsung_k9f2g08u0m_device::samsung_k9f2g08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SAMSUNG_K9F2G08U0M, tag, owner, clock)
{
	m_id_len = 4;
	m_id[0] = 0xec;
	m_id[1] = 0xda;
	m_id[2] = 0x00;
	m_id[3] = 0x15;
	m_page_data_size = 2048;
	m_page_total_size = 2048 + 64;
	m_log2_pages_per_block = compute_log2(64);
	m_num_pages = 128 * 1024;
	m_col_address_cycles = 2;
	m_row_address_cycles = 3;
	m_sequential_row_read = 0;
}

toshiba_tc58256aft_device::toshiba_tc58256aft_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, TOSHIBA_TC58256AFT, tag, owner, clock)
{
	m_id_len = 2;
	m_id[0] = 0x98;
	m_id[1] = 0x75;
	m_page_data_size = 512;
	m_page_total_size = 512 + 16;
	m_log2_pages_per_block = compute_log2(32);
	m_num_pages = 32 * 2048;
	m_col_address_cycles = 1;
	m_row_address_cycles = 2;
	m_sequential_row_read = 0;
}

void nand_device::device_start()
{
	m_data_uid_ptr = nullptr; // smartmed cruft
	m_feeprom_data = std::make_unique<uint8_t[]>(m_page_total_size * m_num_pages);
	m_pagereg = std::make_unique<uint8_t[]>(m_page_total_size);

	save_item(NAME(m_mode));
	save_item(NAME(m_pointer_mode));
	save_item(NAME(m_page_addr));
	save_item(NAME(m_byte_addr));
	save_item(NAME(m_status));
	save_item(NAME(m_accumulated_status));
	save_item(NAME(m_mode_3065));
}

void nand_device::device_reset()
{
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_page_addr = 0;
	m_byte_addr = 0;
	m_accumulated_status = 0;
	m_mode_3065 = false;
	m_status = 0xc0;

	std::fill_n(m_pagereg.get(), m_page_total_size, 0);
}

void nand_device::nvram_default()
{
	if (m_region.found())
	{
		// Copy from region if it exists
		uint32_t bytes = m_region->bytes();

		if (bytes > m_page_total_size * m_num_pages)
			bytes = m_page_total_size * m_num_pages;

		for (offs_t offs = 0; offs < bytes; offs++)
			m_feeprom_data[offs] = m_region->as_u8(offs);

		return;
	}

	memset(&m_feeprom_data[0], 0xff, m_page_total_size * m_num_pages);
}

bool nand_device::nvram_read(util::read_stream &file)
{
	uint32_t const size = m_page_total_size * m_num_pages;
	auto const [err, actual] = read(file, &m_feeprom_data[0], size);
	return !err && (actual == size);
}

bool nand_device::nvram_write(util::write_stream &file)
{
	uint32_t const size = m_page_total_size * m_num_pages;
	auto const [err, actual] = write(file, &m_feeprom_data[0], size);
	return !err;
}

int nand_device::is_present()
{
	return m_num_pages != 0;
}

int nand_device::is_protected()
{
	return (m_status & 0x80) == 0;
}

int nand_device::is_busy()
{
	return (m_status & 0x40) == 0;
}

void nand_device::command_w(uint8_t data)
{
	if (!is_present())
		return;

	switch (data)
	{
	case 0xff: // Reset
		m_mode = SM_M_INIT;
		m_pointer_mode = SM_PM_A;
		m_status = (m_status & 0x80) | 0x40;
		m_accumulated_status = 0;
		m_mode_3065 = false;

		m_write_rnb(0);
		m_write_rnb(1);
		break;
	case 0x00: // Read (1st cycle)
		m_mode = SM_M_READ;
		m_pointer_mode = SM_PM_A;
		m_addr_load_ptr = 0;
		break;
	case 0x01:
		if (m_page_data_size != 512)
		{
			logerror("nandflash: unsupported upper data field select (256-byte pages)\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_READ;
			m_pointer_mode = SM_PM_B;
			m_addr_load_ptr = 0;
		}
		break;
	case 0x50:
		if (m_page_data_size > 512)
		{
			logerror("nandflash: unsupported spare area select\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_READ;
			m_pointer_mode = SM_PM_C;
			m_addr_load_ptr = 0;
		}
		break;
	case 0x80: // Page Program (1st cycle)
		m_mode = SM_M_PROGRAM;
		m_addr_load_ptr = 0;
		m_program_byte_count = 0;
		memset(m_pagereg.get(), 0xff, m_page_total_size);
		break;
	case 0x10: // Page Program (2nd cycle)
	case 0x15:
		if ((m_mode != SM_M_PROGRAM) && (m_mode != SM_M_RANDOM_DATA_INPUT))
		{
			logerror("nandflash: illegal page program confirm command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_status = (m_status & 0x80) | m_accumulated_status;
			// logerror( "nandflash: program, page_addr %08X\n", m_page_addr);
			for (int i = 0; i < m_page_total_size; i++)
				m_feeprom_data[m_page_addr * m_page_total_size + i] &= m_pagereg[i];
			m_status |= 0x40;
			if (data == 0x15)
				m_accumulated_status = m_status & 0x1f;
			else
				m_accumulated_status = 0;
			m_mode = SM_M_INIT;

			m_write_rnb(0);
			m_write_rnb(1);
		}
		break;
	// case 0x11:
	//   break;
	case 0x60: // Block Erase (1st cycle)
		m_mode = SM_M_ERASE;
		m_page_addr = 0;
		m_addr_load_ptr = 0;
		break;
	case 0xd0: // Block Erase (2nd cycle)
		if (m_mode != SM_M_ERASE)
		{
			logerror("nandflash: illegal block erase confirm command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_status &= 0x80;
			memset(m_feeprom_data.get() + ((m_page_addr & (-1 << m_log2_pages_per_block)) * m_page_total_size), 0xFF, (size_t)(1 << m_log2_pages_per_block) * m_page_total_size);
			// logerror( "nandflash: erase, page_addr %08X, offset %08X, length %08X\n", m_page_addr, (m_page_addr & (-1 << m_log2_pages_per_block)) * m_page_total_size, (1 << m_log2_pages_per_block) * m_page_total_size);
			m_status |= 0x40;
			m_mode = SM_M_INIT;
			if (m_pointer_mode == SM_PM_B)
				m_pointer_mode = SM_PM_A;

			m_write_rnb(0);
			m_write_rnb(1);
		}
		break;
	case 0x70: // Read Status
		m_mode = SM_M_READSTATUS;
		break;
	// case 0x71:
	//   break;
	case 0x90: // Read ID
		m_mode = SM_M_READID;
		m_addr_load_ptr = 0;
		break;
	// case 0x91:
	//   break;
	case 0x30: // Read (2nd cycle)
		if (m_col_address_cycles == 1)
		{
			m_mode = SM_M_30;
		}
		else
		{
			if (m_mode != SM_M_READ)
			{
				logerror("nandflash: illegal read 2nd cycle command\n");
				m_mode = SM_M_INIT;
			}
			else if (m_addr_load_ptr < (m_col_address_cycles + m_row_address_cycles))
			{
				logerror("nandflash: read 2nd cycle, not enough address cycles (actual: %d, expected: %d)\n", m_addr_load_ptr, m_col_address_cycles + m_row_address_cycles);
				m_mode = SM_M_INIT;
			}
			else
			{
				m_write_rnb(0);
				m_write_rnb(1);
			}
		}
		break;
	case 0x65:
		if (m_mode != SM_M_30)
		{
			logerror("nandflash: unexpected address port write\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode_3065 = true;
		}
		break;
	case 0x05: // Random Data Output (1st cycle)
		if ((m_mode != SM_M_READ) && (m_mode != SM_M_RANDOM_DATA_OUTPUT))
		{
			logerror("nandflash: illegal random data output command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_RANDOM_DATA_OUTPUT;
			m_addr_load_ptr = 0;
		}
		break;
	case 0xE0: // Random Data Output (2nd cycle)
		if (m_mode != SM_M_RANDOM_DATA_OUTPUT)
		{
			logerror("nandflash: illegal random data output confirm command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			// do nothing
		}
		break;
	case 0x85: // Random Data Input
		if ((m_mode != SM_M_PROGRAM) && (m_mode != SM_M_RANDOM_DATA_INPUT))
		{
			logerror("nandflash: illegal random data input command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_RANDOM_DATA_INPUT;
			m_addr_load_ptr = 0;
			m_program_byte_count = 0;
		}
		break;
	default:
		logerror("nandflash: unsupported command 0x%02x\n", data);
		m_mode = SM_M_INIT;
		break;
	}
}

void nand_device::address_w(uint8_t data)
{
	if (!is_present())
		return;

	switch (m_mode)
	{
	case SM_M_INIT:
		logerror("nandflash: unexpected address port write\n");
		break;
	case SM_M_READ:
	case SM_M_PROGRAM:
		if (m_addr_load_ptr == 0)
		{
			m_page_addr = 0;
		}
		if ((m_addr_load_ptr == 0) && (m_col_address_cycles == 1))
		{
			switch (m_pointer_mode)
			{
			case SM_PM_A:
				m_byte_addr = data;
				break;
			case SM_PM_B:
				m_byte_addr = data + 256;
				m_pointer_mode = SM_PM_A;
				break;
			case SM_PM_C:
				if (!m_mode_3065)
					m_byte_addr = (data & 0x0f) + m_page_data_size;
				else
					m_byte_addr = (data & 0x0f) + 256;
				break;
			}
		}
		else
		{
			if (m_addr_load_ptr < m_col_address_cycles)
			{
				m_byte_addr &= ~(0xFF << (m_addr_load_ptr * 8));
				m_byte_addr |= (data << (m_addr_load_ptr * 8));
			}
			else if (m_addr_load_ptr < m_col_address_cycles + m_row_address_cycles)
			{
				m_page_addr &= ~(0xFF << ((m_addr_load_ptr - m_col_address_cycles) * 8));
				m_page_addr |= (data << ((m_addr_load_ptr - m_col_address_cycles) * 8));
			}
		}
		m_addr_load_ptr++;
		break;
	case SM_M_ERASE:
		if (m_addr_load_ptr < m_row_address_cycles)
		{
			m_page_addr &= ~(0xFF << (m_addr_load_ptr * 8));
			m_page_addr |= (data << (m_addr_load_ptr * 8));
		}
		m_addr_load_ptr++;
		break;
	case SM_M_RANDOM_DATA_INPUT:
	case SM_M_RANDOM_DATA_OUTPUT:
		if (m_addr_load_ptr < m_col_address_cycles)
		{
			m_byte_addr &= ~(0xFF << (m_addr_load_ptr * 8));
			m_byte_addr |= (data << (m_addr_load_ptr * 8));
		}
		m_addr_load_ptr++;
		break;
	case SM_M_READSTATUS:
	case SM_M_30:
		logerror("nandflash: unexpected address port write\n");
		break;
	case SM_M_READID:
		if (m_addr_load_ptr == 0)
			m_byte_addr = data;
		m_addr_load_ptr++;
		break;
	}
}

uint8_t nand_device::data_r()
{
	uint8_t reply = 0;
	if (!is_present())
		return 0;

	switch (m_mode)
	{
	case SM_M_INIT:
	case SM_M_30:
		logerror("nandflash: unexpected data port read\n");
		break;
	case SM_M_READ:
	case SM_M_RANDOM_DATA_OUTPUT:
		if (!m_mode_3065)
		{
			if (m_byte_addr < m_page_total_size)
			{
				if (m_page_addr < m_num_pages)
					reply = m_feeprom_data[m_page_addr * m_page_total_size + m_byte_addr];
				else
					reply = 0xff;
			}
			else
			{
				reply = 0xFF;
			}
		}
		else
		{
			if (m_data_uid_ptr != nullptr)
			{
				// FIXME: this appears to be incorrect, m_data_uid_ptr is a smaller structure of 256*16
				// this code would always result in reading past the buffer
				uint32_t addr = m_page_addr * m_page_total_size + m_byte_addr;
				if (addr < 256 + 16)
					reply = m_data_uid_ptr[addr];
			}
			else
			{
				reply = 0xff;
			}
		}
		m_byte_addr++;

		// "Sequential Row Read is available only on K9F5608U0D_Y,P,V,F or K9F5608D0D_Y,P"
		if ((m_byte_addr == m_page_total_size) && (m_sequential_row_read != 0))
		{
			m_byte_addr = (m_pointer_mode != SM_PM_C) ? 0 : m_page_data_size;
			m_page_addr++;
			if (m_page_addr == m_num_pages)
				m_page_addr = 0;
		}
		break;
	case SM_M_PROGRAM:
	case SM_M_RANDOM_DATA_INPUT:
	case SM_M_ERASE:
		logerror("nandflash: unexpected data port read\n");
		break;
	case SM_M_READSTATUS:
		reply = m_status & 0xc1;
		break;
	case SM_M_READID:
		if (m_byte_addr < m_id_len)
			reply = m_id[m_byte_addr];
		else
			reply = 0;
		m_byte_addr++;
		break;
	}

	return reply;
}

void nand_device::data_w(uint8_t data)
{
	if (!is_present())
		return;

	switch (m_mode)
	{
	case SM_M_INIT:
	case SM_M_READ:
	case SM_M_30:
	case SM_M_RANDOM_DATA_OUTPUT:
		logerror("nandflash: unexpected data port write\n");
		break;
	case SM_M_PROGRAM:
	case SM_M_RANDOM_DATA_INPUT:
		if (m_program_byte_count++ < m_page_total_size)
		{
			m_pagereg[m_byte_addr] = data;
		}
		m_byte_addr++;
		if (m_byte_addr == m_page_total_size)
			m_byte_addr = (m_pointer_mode != SM_PM_C) ? 0 : m_page_data_size;
		break;
	case SM_M_ERASE:
	case SM_M_READSTATUS:
	case SM_M_READID:
		logerror("nandflash: unexpected data port write\n");
		break;
	}
}
