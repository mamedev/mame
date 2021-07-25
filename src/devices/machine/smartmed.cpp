// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    smartmed.c: SmartMedia Flash ROM emulation

    The SmartMedia is a Flash ROM in a fancy card.  It is used in a variety of
    digital devices (still cameras...) and can be interfaced with a computer.

    References:
    Datasheets for various SmartMedia chips were found on Samsung and Toshiba's
    sites (http://www.toshiba.com/taec and
    http://www.samsung.com/Products/Semiconductor/Flash/FlashCard/SmartMedia)

    TODO:
    * support multi-plane mode?
    * use HD-format images instead of our experimental custom format?

    Raphael Nabet 2004
*/

#include "emu.h"
#include "smartmed.h"


namespace {

/* machine-independent big-endian 32-bit integer */
struct UINT32BE
{
	uint8_t bytes[4];
};

inline uint32_t get_UINT32BE(UINT32BE word)
{
	return (word.bytes[0] << 24) | (word.bytes[1] << 16) | (word.bytes[2] << 8) | word.bytes[3];
}

#ifdef UNUSED_FUNCTION
inline void set_UINT32BE(UINT32BE *word, uint32_t data)
{
	word->bytes[0] = (data >> 24) & 0xff;
	word->bytes[1] = (data >> 16) & 0xff;
	word->bytes[2] = (data >> 8) & 0xff;
	word->bytes[3] = data & 0xff;
}
#endif

/* SmartMedia image header */
struct SM_disk_image_header
{
	uint8_t version;
	UINT32BE page_data_size;
	UINT32BE page_total_size;
	UINT32BE num_pages;
	UINT32BE log2_pages_per_block;
};

struct disk_image_format_2_header
{
	uint8_t data1[3];
	uint8_t padding1[256-3];
	uint8_t data2[16];
	uint8_t data3[16];
	uint8_t padding2[768-32];
};

enum
{
	header_len = sizeof(SM_disk_image_header)
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(NAND, nand_device, "nand", "NAND Flash Memory")

nand_device::nand_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, NAND, tag, owner, clock)
{
}

nand_device::nand_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_page_data_size(0)
	, m_page_total_size(0)
	, m_num_pages(0)
	, m_log2_pages_per_block(0)
	, m_pagereg(nullptr)
	, m_id_len(0)
	, m_col_address_cycles(0)
	, m_row_address_cycles(0)
	, m_sequential_row_read(0)
	, m_write_rnb(*this)
{
	memset(m_id, 0, sizeof(m_id));
}

/*
    Init a SmartMedia image
*/
void nand_device::device_start()
{
	m_feeprom_data = nullptr;
	m_data_uid_ptr = nullptr;
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_page_addr = 0;
	m_byte_addr = 0;
	m_status = 0xC0;
	m_accumulated_status = 0;
	m_mp_opcode = 0;
	m_mode_3065 = 0;
	m_pagereg = std::make_unique<uint8_t[]>(m_page_total_size);

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 0;
#endif
	m_write_rnb.resolve_safe();
}

/*
    Load a SmartMedia image
*/
image_init_result smartmedia_image_device::smartmedia_format_1()
{
	SM_disk_image_header custom_header;

	const int bytes_read = fread(&custom_header, sizeof(custom_header));
	if (bytes_read != sizeof(custom_header))
	{
		return image_init_result::FAIL;
	}

	if (custom_header.version > 1)
	{
		return image_init_result::FAIL;
	}

	m_page_data_size = get_UINT32BE(custom_header.page_data_size);
	m_page_total_size = get_UINT32BE(custom_header.page_total_size);
	m_num_pages = get_UINT32BE(custom_header.num_pages);
	m_log2_pages_per_block = get_UINT32BE(custom_header.log2_pages_per_block);
	m_feeprom_data_alloc = std::make_unique<uint8_t[]>(m_page_total_size*m_num_pages);
	m_feeprom_data = &m_feeprom_data_alloc[0];
	m_data_uid_ptr = std::make_unique<uint8_t[]>(256 + 16);
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_page_addr = 0;
	m_byte_addr = 0;
	m_status = 0x40;
	if (!is_readonly())
		m_status |= 0x80;
	m_accumulated_status = 0;
	m_pagereg = std::make_unique<uint8_t[]>(m_page_total_size);
	memset( m_id, 0, sizeof( m_id));
	m_id_len = 0;
	m_col_address_cycles = 1;
	m_row_address_cycles = (m_num_pages > 0x10000) ? 3 : 2;
	m_sequential_row_read = 1;

	if (custom_header.version == 0)
	{
		m_id_len = 2;
		fread(m_id, m_id_len);
		fread(&m_mp_opcode, 1);
	}
	else if (custom_header.version == 1)
	{
		m_id_len = 3;
		fread(m_id, m_id_len);
		fread(&m_mp_opcode, 1);
		fread(m_data_uid_ptr.get(), 256 + 16);
	}
	fread(m_feeprom_data, m_page_total_size*m_num_pages);

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 1;
#endif

	return image_init_result::PASS;
}

int smartmedia_image_device::detect_geometry( uint8_t id1, uint8_t id2)
{
	int result = false;

	switch (id1)
	{
		case 0xEC :
		{
			switch (id2)
			{
				case 0xA4 : m_page_data_size = 0x0100; m_num_pages = 0x00800; m_page_total_size = 0x0108; m_log2_pages_per_block = 0; result = true; break;
				case 0x6E : m_page_data_size = 0x0100; m_num_pages = 0x01000; m_page_total_size = 0x0108; m_log2_pages_per_block = 0; result = true; break;
				case 0xEA : m_page_data_size = 0x0100; m_num_pages = 0x02000; m_page_total_size = 0x0108; m_log2_pages_per_block = 4; result = true; break;
				case 0xE3 : m_page_data_size = 0x0200; m_num_pages = 0x02000; m_page_total_size = 0x0210; m_log2_pages_per_block = 4; result = true; break;
				case 0xE6 : m_page_data_size = 0x0200; m_num_pages = 0x04000; m_page_total_size = 0x0210; m_log2_pages_per_block = 4; result = true; break;
				case 0x73 : m_page_data_size = 0x0200; m_num_pages = 0x08000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
				case 0x75 : m_page_data_size = 0x0200; m_num_pages = 0x10000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
				case 0x76 : m_page_data_size = 0x0200; m_num_pages = 0x20000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
				case 0x79 : m_page_data_size = 0x0200; m_num_pages = 0x40000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
			}
		}
		break;
		case 0x98 :
		{
			switch (id2)
			{
				case 0x73 : m_page_data_size = 0x0200; m_num_pages = 0x08000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
				case 0x75 : m_page_data_size = 0x0200; m_num_pages = 0x10000; m_page_total_size = 0x0210; m_log2_pages_per_block = 5; result = true; break;
			}
		}
		break;
	}

	return result;
}

image_init_result smartmedia_image_device::smartmedia_format_2()
{
	disk_image_format_2_header custom_header;

	const int bytes_read = fread(&custom_header, sizeof(custom_header));
	if (bytes_read != sizeof(custom_header))
	{
		return image_init_result::FAIL;
	}

	if ((custom_header.data1[0] != 0xEC) && (custom_header.data1[0] != 0x98))
	{
		return image_init_result::FAIL;
	}

	if (!detect_geometry(custom_header.data1[0], custom_header.data1[1]))
	{
		return image_init_result::FAIL;
	}

	m_feeprom_data_alloc = std::make_unique<uint8_t[]>(m_page_total_size*m_num_pages);
	m_feeprom_data = &m_feeprom_data_alloc[0];
	m_data_uid_ptr = std::make_unique<uint8_t[]>(256 + 16);
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_page_addr = 0;
	m_byte_addr = 0;
	m_status = 0x40;
	if (!is_readonly())
		m_status |= 0x80;
	m_accumulated_status = 0;
	m_pagereg = std::make_unique<uint8_t[]>(m_page_total_size);
	m_id_len = 3;
	memcpy(m_id, custom_header.data1, m_id_len);
	m_mp_opcode = 0;
	m_col_address_cycles = 1;
	m_row_address_cycles = (m_num_pages > 0x10000) ? 3 : 2;
	m_sequential_row_read = 1;

	for (int i = 0; i < 8; i++)
	{
		memcpy(m_data_uid_ptr.get() + i * 32, custom_header.data2, 16);
		for (int j = 0; j < 16; j++)
			m_data_uid_ptr[i * 32 + 16 + j] = custom_header.data2[j] ^ 0xFF;
	}
	memcpy(m_data_uid_ptr.get() + 256, custom_header.data3, 16);

	fread(m_feeprom_data, m_page_total_size*m_num_pages);

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 2;
#endif

	return image_init_result::PASS;
}

image_init_result smartmedia_image_device::call_load()
{
	image_init_result result;
	uint64_t position;
	// try format 1
	position = ftell();
	result = smartmedia_format_1();
	if (result != image_init_result::PASS)
	{
		// try format 2
		fseek(position, SEEK_SET);
		result = smartmedia_format_2();
	}
	return result;
}

/*
    Unload a SmartMedia image
*/
void smartmedia_image_device::call_unload()
{
#ifdef SMARTMEDIA_IMAGE_SAVE
	if (!is_readonly())
	{
		if (m_image_format == 1)
		{
			SM_disk_image_header custom_header;
			fseek(0, SEEK_SET);
			const int bytes_read = fread(&custom_header, sizeof(custom_header));
			if (bytes_read == sizeof(custom_header))
			{
				if (custom_header.version == 0)
				{
					fseek(2 + 1, SEEK_CUR);
					fwrite(m_feeprom_data, m_page_total_size * m_num_pages);
				}
				else if (custom_header.version == 1)
				{
					fseek(3 + 1 + 256 + 16, SEEK_CUR);
					fwrite(m_feeprom_data, m_page_total_size * m_num_pages);
				}
			}
		}
		else if (m_image_format == 2)
		{
			fseek(sizeof(disk_image_format_2_header), SEEK_SET);
			fwrite(m_feeprom_data, m_page_total_size * m_num_pages);
		}
	}
#endif

	m_page_data_size = 0;
	m_page_total_size = 0;
	m_num_pages = 0;
	m_log2_pages_per_block = 0;
	m_feeprom_data = nullptr;
	m_data_uid_ptr = nullptr;
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_page_addr = 0;
	m_byte_addr = 0;
	m_status = 0xC0;
	m_accumulated_status = 0;
	m_pagereg = std::make_unique<uint8_t[]>(m_page_total_size);
	memset(m_id, 0, sizeof(m_id));
	m_id_len = 0;
	m_mp_opcode = 0;
	m_mode_3065 = 0;
	m_col_address_cycles = 0;
	m_row_address_cycles = 0;
	m_sequential_row_read = 0;

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 0;
#endif

	return;
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

void nand_device::set_data_ptr(void *ptr)
{
	m_feeprom_data = (uint8_t *)ptr;
}

/*
    write a byte to SmartMedia command port
*/
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
		m_mode_3065 = 0;
		if (!m_write_rnb.isnull())
		{
			m_write_rnb(0);
			m_write_rnb(1);
		}
		break;
	case 0x00: // Read (1st cycle)
		m_mode = SM_M_READ;
		m_pointer_mode = SM_PM_A;
		m_page_addr = 0;
		m_addr_load_ptr = 0;
		break;
	case 0x01:
		if (m_page_data_size != 512)
		{
			logerror("smartmedia: unsupported upper data field select (256-byte pages)\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_READ;
			m_pointer_mode = SM_PM_B;
			m_page_addr = 0;
			m_addr_load_ptr = 0;
		}
		break;
	case 0x50:
		if (m_page_data_size > 512)
		{
			logerror("smartmedia: unsupported spare area select\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode = SM_M_READ;
			m_pointer_mode = SM_PM_C;
			m_page_addr = 0;
			m_addr_load_ptr = 0;
		}
		break;
	case 0x80: // Page Program (1st cycle)
		m_mode = SM_M_PROGRAM;
		m_page_addr = 0;
		m_addr_load_ptr = 0;
		m_program_byte_count = 0;
		memset(m_pagereg.get(), 0xff, m_page_total_size);
		break;
	case 0x10: // Page Program (2nd cycle)
	case 0x15:
		if ((m_mode != SM_M_PROGRAM) && (m_mode != SM_M_RANDOM_DATA_INPUT))
		{
			logerror("smartmedia: illegal page program confirm command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_status = (m_status & 0x80) | m_accumulated_status;
			//logerror( "smartmedia: program, page_addr %08X\n", m_page_addr);
			for (int i = 0; i < m_page_total_size; i++)
				m_feeprom_data[m_page_addr * m_page_total_size + i] &= m_pagereg[i];
			m_status |= 0x40;
			if (data == 0x15)
				m_accumulated_status = m_status & 0x1f;
			else
				m_accumulated_status = 0;
			m_mode = SM_M_INIT;
			if (!m_write_rnb.isnull())
			{
				m_write_rnb(0);
				m_write_rnb(1);
			}
		}
		break;
	//case 0x11:
	//  break;
	case 0x60: // Block Erase (1st cycle)
		m_mode = SM_M_ERASE;
		m_page_addr = 0;
		m_addr_load_ptr = 0;
		break;
	case 0xd0: // Block Erase (2nd cycle)
		if (m_mode != SM_M_ERASE)
		{
			logerror("smartmedia: illegal block erase confirm command\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_status &= 0x80;
			memset(m_feeprom_data + ((m_page_addr & (-1 << m_log2_pages_per_block)) * m_page_total_size), 0xFF, (size_t)(1 << m_log2_pages_per_block) * m_page_total_size);
			//logerror( "smartmedia: erase, page_addr %08X, offset %08X, length %08X\n", m_page_addr, (m_page_addr & (-1 << m_log2_pages_per_block)) * m_page_total_size, (1 << m_log2_pages_per_block) * m_page_total_size);
			m_status |= 0x40;
			m_mode = SM_M_INIT;
			if (m_pointer_mode == SM_PM_B)
				m_pointer_mode = SM_PM_A;
			if (!m_write_rnb.isnull())
			{
				m_write_rnb(0);
				m_write_rnb(1);
			}
		}
		break;
	case 0x70: // Read Status
		m_mode = SM_M_READSTATUS;
		break;
	//case 0x71:
	//  break;
	case 0x90: // Read ID
		m_mode = SM_M_READID;
		m_addr_load_ptr = 0;
		break;
	//case 0x91:
	//  break;
	case 0x30: // Read (2nd cycle)
		if (m_col_address_cycles == 1)
		{
			m_mode = SM_M_30;
		}
		else
		{
			if (m_mode != SM_M_READ)
			{
				logerror("smartmedia: illegal read 2nd cycle command\n");
				m_mode = SM_M_INIT;
			}
			else if (m_addr_load_ptr < (m_col_address_cycles + m_row_address_cycles))
			{
				logerror("smartmedia: read 2nd cycle, not enough address cycles (actual: %d, expected: %d)\n", m_addr_load_ptr, m_col_address_cycles + m_row_address_cycles);
				m_mode = SM_M_INIT;
			}
			else
			{
				if (!m_write_rnb.isnull())
				{
					m_write_rnb(0);
					m_write_rnb(1);
				}
			}
		}
		break;
	case 0x65:
		if (m_mode != SM_M_30)
		{
			logerror("smartmedia: unexpected address port write\n");
			m_mode = SM_M_INIT;
		}
		else
		{
			m_mode_3065 = 1;
		}
		break;
	case 0x05: // Random Data Output (1st cycle)
		if ((m_mode != SM_M_READ) && (m_mode != SM_M_RANDOM_DATA_OUTPUT))
		{
			logerror("smartmedia: illegal random data output command\n");
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
			logerror("smartmedia: illegal random data output confirm command\n");
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
			logerror("smartmedia: illegal random data input command\n");
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
		logerror("smartmedia: unsupported command 0x%02x\n", data);
		m_mode = SM_M_INIT;
		break;
	}
}

/*
    write a byte to SmartMedia address port
*/
void nand_device::address_w(uint8_t data)
{
	if (!is_present())
		return;

	switch (m_mode)
	{
	case SM_M_INIT:
		logerror("smartmedia: unexpected address port write\n");
		break;
	case SM_M_READ:
	case SM_M_PROGRAM:
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
		logerror("smartmedia: unexpected address port write\n");
		break;
	case SM_M_READID:
		if (m_addr_load_ptr == 0)
			m_byte_addr = data;
		m_addr_load_ptr++;
		break;
	}
}

/*
    read a byte from SmartMedia data port
*/
uint8_t nand_device::data_r()
{
	uint8_t reply = 0;
	if (!is_present())
		return 0;

	switch (m_mode)
	{
	case SM_M_INIT:
	case SM_M_30:
		logerror("smartmedia: unexpected data port read\n");
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
			// FIXME: this appears to be incorrect, m_data_uid_ptr is a smaller structure of 256*16
			// this code would always result in reading past the buffer
			uint32_t addr = m_page_addr * m_page_total_size + m_byte_addr;
			if (addr < 256 + 16)
				reply = m_data_uid_ptr[addr];
		}
		m_byte_addr++;
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
		logerror("smartmedia: unexpected data port read\n");
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

/*
    write a byte to SmartMedia data port
*/
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
		logerror("smartmedia: unexpected data port write\n");
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
		logerror("smartmedia: unexpected data port write\n");
		break;
	}
}


/*
    Initialize one SmartMedia chip: may be called at driver init or image load
    time (or machine init time if you don't use MESS image core)
*/
void nand_device::device_reset()
{
	m_mode = SM_M_INIT;
	m_pointer_mode = SM_PM_A;
	m_status = (m_status & 0x80) | 0x40;
	m_accumulated_status = 0;
}


DEFINE_DEVICE_TYPE(SMARTMEDIA, smartmedia_image_device, "smartmedia", "SmartMedia Flash card")

smartmedia_image_device::smartmedia_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SMARTMEDIA, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}
