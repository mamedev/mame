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
    * better separation of SmartMedia image and NAND flash device

    Raphael Nabet 2004
*/

#include "emu.h"
#include "smartmed.h"

#include "softlist_dev.h"

#include <tuple>


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

/*
    Load a SmartMedia image
*/
std::error_condition smartmedia_image_device::smartmedia_format_1()
{
	std::error_condition err;
	size_t bytes_read;

	SM_disk_image_header custom_header;

	std::tie(err, bytes_read) = read(image_core_file(), &custom_header, sizeof(custom_header));
	if (err || (bytes_read != sizeof(custom_header)))
		return err ? err : std::errc::io_error;

	if (custom_header.version > 1)
		return image_error::INVALIDIMAGE;

	m_page_data_size = get_UINT32BE(custom_header.page_data_size);
	m_page_total_size = get_UINT32BE(custom_header.page_total_size);
	m_num_pages = get_UINT32BE(custom_header.num_pages);
	m_log2_pages_per_block = get_UINT32BE(custom_header.log2_pages_per_block);
	m_feeprom_data = std::make_unique<uint8_t[]>(m_page_total_size*m_num_pages);
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

	std::tie(err, m_feeprom_data, bytes_read) = read(image_core_file(), m_page_total_size * m_num_pages);
	if (err || (bytes_read != (m_page_total_size * m_num_pages)))
		return err ? err : std::errc::io_error;

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 1;
#endif

	return std::error_condition();
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

std::error_condition smartmedia_image_device::smartmedia_format_2()
{
	std::error_condition err;
	size_t bytes_read;

	disk_image_format_2_header custom_header;

	std::tie(err, bytes_read) = read(image_core_file(), &custom_header, sizeof(custom_header));
	if (err || (bytes_read != sizeof(custom_header)))
		return err ? err : std::errc::io_error;

	if ((custom_header.data1[0] != 0xEC) && (custom_header.data1[0] != 0x98))
		return image_error::INVALIDIMAGE;

	if (!detect_geometry(custom_header.data1[0], custom_header.data1[1]))
		return image_error::INVALIDIMAGE;

	m_feeprom_data = std::make_unique<uint8_t[]>(m_page_total_size*m_num_pages);
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

	std::tie(err, m_feeprom_data, bytes_read) = read(image_core_file(), m_page_total_size * m_num_pages);
	if (err || (bytes_read != (m_page_total_size * m_num_pages)))
		return err ? err : std::errc::io_error;

#ifdef SMARTMEDIA_IMAGE_SAVE
	m_image_format = 2;
#endif

	return std::error_condition();
}

std::pair<std::error_condition, std::string> smartmedia_image_device::call_load()
{
	std::error_condition result;
	// try format 1
	uint64_t const position = ftell();
	result = smartmedia_format_1();
	if (result)
	{
		// try format 2
		fseek(position, SEEK_SET);
		result = smartmedia_format_2();
	}
	return std::make_pair(result, std::string());
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

DEFINE_DEVICE_TYPE(SMARTMEDIA, smartmedia_image_device, "smartmedia", "SmartMedia Flash card")

smartmedia_image_device::smartmedia_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nand_device(mconfig, SMARTMEDIA, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
{
	// SmartMedia images have been read only so keep it that way until someone puts more thought into this device
	nvram_enable_backup(false);
}

const software_list_loader &smartmedia_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}
