// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    ch376.cpp

    "File manage and control chip CH376"
    https://www.mpja.com/download/ch376ds1.pdf
    https://github.com/djuseeq/Ch376msc/blob/master/src/CommDef.h

    This is a module intended to offload USB and USB mass storage
    I/O from a small microcontroller or microprocessor.

    It has 3 host interfaces:
        1) A regular 8-bit bus with 8 data lines, an address line,
           chip select, read enable, and write enable lines
        2) An SPI bus interface (SCS, SCK, MOSI, MISO lines)
        3) An asynchronous serial interface (TxD and RxD lines)

    And 2 guest interfaces: USB 2.0 and a second 4-wire SPI bus for SD/MMC/TF cards.

    The module can give high level directory/open/close/read/write access to
    files on a FAT12, FAT16, or FAT32 filesystem.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/ch376.h"

#include "multibyte.h"

#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CH376, ch376_device, "ch376", "CH376 USB/file manager module")

static constexpr u8 STATUS_USB_INT_SUCCESS = 0x14;
static constexpr u8 STATUS_USB_INT_CONNECT = 0x15;
static constexpr u8 STATUS_USB_INT_DISK_READ = 0x1d;
static constexpr u8 STATUS_USB_INT_DISK_WRITE = 0x1e;
static constexpr u8 STATUS_USB_INT_DISK_ERR = 0x1f;
static constexpr u8 STATUS_ERR_MISS_FILE = 0x42;

// despite the name, this isn't a command
static constexpr u8 CMD_RET_SUCCESS = 0x51;

static constexpr u8 CMD_GET_IC_VER = 0x01;
static constexpr u8 CMD_CHECK_EXIST = 0x06;
static constexpr u8 CMD_GET_FILE_SIZE = 0x0c;
static constexpr u8 CMD_SET_USB_MODE = 0x15;
static constexpr u8 CMD_GET_STATUS = 0x22;
static constexpr u8 CMD_RD_USB_DATA0 = 0x27;
static constexpr u8 CMD_WR_REQ_DATA = 0x2d;
static constexpr u8 CMD_SET_FILE_NAME = 0x2f;
static constexpr u8 CMD_DISK_MOUNT = 0x31;
static constexpr u8 CMD_FILE_OPEN = 0x32;
static constexpr u8 CMD_FILE_ENUM_GO = 0x33;
static constexpr u8 CMD_FILE_CLOSE = 0x36;
static constexpr u8 CMD_BYTE_LOCATE = 0x39;
static constexpr u8 CMD_BYTE_READ = 0x3a;
static constexpr u8 CMD_BYTE_READ_GO = 0x3b;
static constexpr u8 CMD_BYTE_WRITE = 0x3c;
static constexpr u8 CMD_BYTE_WR_GO = 0x3d;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ch376_device - constructor
//-------------------------------------------------

ch376_device::ch376_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CH376, tag, owner, clock),
	m_state(0),
	m_status(0),
	m_int_status(0),
	m_data(0),
	m_last_command(0),
	m_pointer(0),
	m_file_pos(0),
	m_dataPtr(0),
	m_dataLen(0),
	m_readLen(0),
	m_cur_file_size(0)
{
	std::fill_n(&m_file_name[0], sizeof(m_file_name), 0);
	std::fill_n(&m_dataBuffer[0], sizeof(m_dataBuffer), 0);
	m_current_directory.clear();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ch376_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_status));
	save_item(NAME(m_int_status));
	save_item(NAME(m_data));
	save_item(NAME(m_file_name));
	save_item(NAME(m_pointer));
	save_item(NAME(m_dataPtr));
	save_item(NAME(m_dataLen));
	save_item(NAME(m_readLen));

	m_directory.reset();
	m_file.reset();
}

u8 ch376_device::read(offs_t offset)
{
	if (offset & 1)
	{
		return m_status;
	}

	switch (m_state)
	{
		case STATE_GET_STATUS:
			m_state = STATE_IDLE;
			return m_int_status;

		case STATE_WRITE_SIZE_REPLY:
			m_state = STATE_READ_WRITE_DATA;
			m_dataPtr = 0;
			return m_data;
	}

	if ((m_dataLen > 0) && (m_dataPtr < m_dataLen))
	{
		return m_dataBuffer[m_dataPtr++];
	}

	return m_data;
}

void ch376_device::write(offs_t offset, u8 data)
{
	if (offset & 1) // write command
	{
		//printf("CH376: write command %02x\n", data);
		m_last_command = data;
		switch (data)
		{
			case 0x16:  // not a documented valid command, but apparently used by the BBC Micro software
				m_dataBuffer[0] = m_int_status = STATUS_USB_INT_CONNECT;
				m_dataPtr = 0;
				m_dataLen = 1;
				break;

			case CMD_CHECK_EXIST:
				m_state = STATE_CHECK_EXIST;
				break;

			case CMD_GET_IC_VER:
				m_dataBuffer[0] = 0x41;
				m_dataPtr = 0;
				m_dataLen = 1;
				break;

			case CMD_GET_FILE_SIZE:
				m_state = STATE_GET_FILE_SIZE;
				break;

			case CMD_SET_USB_MODE:
				m_state = STATE_USB_MODE_SET;
				break;

			case CMD_GET_STATUS:
				m_state = STATE_GET_STATUS;
				break;

			case CMD_RD_USB_DATA0:
				m_int_status = STATUS_USB_INT_DISK_READ;
				break;

			case CMD_WR_REQ_DATA:
				{
					int lenToWrite = (m_readLen >= 255) ? 255 : m_readLen;
					m_data = (lenToWrite & 0xff);
					m_readLen -= lenToWrite;
					m_state = STATE_WRITE_SIZE_REPLY;
					m_dataLen = lenToWrite;
				}
				break;

			case CMD_SET_FILE_NAME:
				m_state = STATE_SET_FILE_NAME;
				m_pointer = 0;
				break;

			case CMD_DISK_MOUNT:
				m_int_status = STATUS_USB_INT_SUCCESS;
				break;

			case CMD_FILE_OPEN:
				/*
				    CH376 allows paths with a leading / or \ to indicate the root of the SD card.
				    Directories are chained by an unusual method: start with the root dir (leading path separator),
				    then open each subdirectory without a leading path separator, and finally open the file without
				    a leading path separator.

				    The BOOTI card ignores directories entirely, so the implmentation here is not tested with that
				    scenario but it should work.
				*/
				if (m_file_name[strlen(m_file_name) - 1] == '*')
				{
					// it's a directory
					// remove the wildcard
					m_file_name[strlen(m_file_name) - 1] = '\0';
					std::string tmpPath(machine().options().share_directory());
					tmpPath.append(PATH_SEPARATOR);
					if ((m_file_name[0] == '/') || (m_file_name[0] == '\\'))
					{
						tmpPath.append(&m_file_name[1]);
						m_current_directory.clear();
						m_current_directory = tmpPath;
					}
					else
					{
						tmpPath.append(m_file_name);
						m_current_directory.append(PATH_SEPARATOR);
						m_current_directory.append(tmpPath);
					}
					m_directory = osd::directory::open(tmpPath);
					if (generateNextDirEntry())
					{
						m_int_status = STATUS_USB_INT_DISK_READ;
					}
					else
					{
						m_int_status = STATUS_USB_INT_DISK_ERR;
					}
				}
				else
				{
					std::string tmpPath = m_current_directory;
					u64 size;

					// compose this name with the last directory opened, unless it starts with a path separator
					tmpPath.append(PATH_SEPARATOR);
					if ((m_file_name[0] == '/') || (m_file_name[0] == '\\'))
					{
						tmpPath.append(&m_file_name[1]);
					}
					else
					{
						tmpPath.append(m_file_name);
					}

					if (!osd_file::open(tmpPath, OPEN_FLAG_READ|OPEN_FLAG_WRITE, m_file, size))
					{
						m_int_status = STATUS_USB_INT_SUCCESS;
						m_cur_file_size = size & 0xffffffff;
					}
					else
					{
						m_int_status = STATUS_USB_INT_DISK_ERR;
						m_cur_file_size = 0;
					}
				}
				break;

			case CMD_FILE_ENUM_GO:
				if (generateNextDirEntry())
				{
					m_int_status = STATUS_USB_INT_DISK_READ;
				}
				else    // end of directory
				{
					m_int_status = STATUS_ERR_MISS_FILE;
				}
				break;

			case CMD_FILE_CLOSE:
				m_directory.reset();
				m_file.reset();
				m_int_status = STATUS_USB_INT_DISK_READ;
				break;

			case CMD_BYTE_LOCATE:
				m_state = STATE_SEEK;
				break;

			case CMD_BYTE_READ:
				m_state = STATE_READ_SIZE;
				break;

			case CMD_BYTE_READ_GO:
				if (m_readLen > 0)
				{
					doNextFileRead();
					m_int_status = STATUS_USB_INT_DISK_READ;
				}
				else
				{
					m_dataBuffer[0] = 0;
					m_dataLen = 0;
					m_int_status = STATUS_USB_INT_SUCCESS;
				}
				break;

			case CMD_BYTE_WRITE:
				m_state = STATE_READ_SIZE;
				break;

			case CMD_BYTE_WR_GO:
				if (m_dataPtr > 0)
				{
					u32 actual;
					m_state = STATE_IDLE;
					m_file->write(&m_dataBuffer[0], m_file_pos, m_dataLen, actual);
					m_file_pos += m_dataLen;
					if (m_readLen > 0)
					{
						m_int_status = STATUS_USB_INT_DISK_WRITE;
					}
					else
					{
						m_int_status = STATUS_USB_INT_SUCCESS;
					}
				}
				else
				{
					m_int_status = STATUS_USB_INT_SUCCESS;
				}
				break;

			default:
				logerror("CH376: unhandled command %02x\n", data);
				break;
		}
	}
	else
	{
		switch (m_state)
		{
			case STATE_CHECK_EXIST:
				m_data = (data ^ 0xff);
				m_state = STATE_IDLE;
				break;

			case STATE_USB_MODE_SET:
				m_state = STATE_IDLE;
				if ((data == 0x6) || (data == 0x7))
				{
					m_data = CMD_RET_SUCCESS;
				}
				break;

			case STATE_SET_FILE_NAME:
				m_file_name[m_pointer++] = data;
				if (m_pointer == 14)
				{
					m_state = STATE_IDLE;
					// is this shorter than 8.3?
					if (m_file_name[8] == '\0')
					{
						int idx = 8;
						while (m_file_name[idx] == '\0')
						{
							idx--;
						}
						idx++;
						for (int i = 0; i < 5; i++)
						{
							m_file_name[idx + i] = m_file_name[9 + i];
						}
					}
				}
				break;

			case STATE_SEEK:
			case STATE_SEEK1:
			case STATE_SEEK2:
				m_file_pos >>= 8;
				m_file_pos |= (data << 24);
				m_state++;
				break;

			case STATE_SEEK3:
				m_file_pos >>= 8;
				m_file_pos |= (data << 24);
				m_state = STATE_IDLE;
				m_int_status = STATUS_USB_INT_SUCCESS;
				break;

			case STATE_READ_SIZE:
				m_readLen = data;
				m_state++;
				break;

			case STATE_READ_SIZE2:
				m_readLen |= (data << 8);
				m_state = STATE_IDLE;
				m_int_status = STATUS_USB_INT_DISK_READ;

				if (m_last_command == 0x3a)
				{
					doNextFileRead();
				}
				break;

			case STATE_READ_WRITE_DATA:
				m_dataBuffer[m_dataPtr++] = data;
				break;

			case STATE_GET_FILE_SIZE:
				// the host must write 0x68 here; it's unclear what the real chip does if the value doesn't match.
				// reply with the size of the currently open file.
				put_u32le(m_dataBuffer, m_cur_file_size);
				m_dataPtr = 0;
				m_dataLen = 4;
				break;
		}
	}
}

bool ch376_device::generateNextDirEntry()
{
	const osd::directory::entry *ourEntry;

	m_dataPtr = 0;
	m_dataLen = 0;

	// no directory?
	if (m_directory == nullptr)
	{
		return false;
	}

	// is there an entry?
	if ((ourEntry = m_directory->read()) == nullptr)
	{
		return false;
	}

	std::fill_n(&m_dataBuffer[0], sizeof(m_dataBuffer), 0);
	m_dataBuffer[0] = 32;
	if (ourEntry->type == osd::directory::entry::entry_type::DIR)
	{
		strncpy(reinterpret_cast<char *>(&m_dataBuffer[1]), ourEntry->name, 8);
		m_dataBuffer[0xc] = 0x10;   // directory type
	}
	else if (ourEntry->type == osd::directory::entry::entry_type::FILE)
	{
		int dotIdx = 8;
		for (int idx = (strlen(ourEntry->name) - 1); idx >= 0; idx--)
		{
			if (ourEntry->name[idx] == '.')
			{
				dotIdx = idx;
				break;
			}
		}

		// is this an 8.3 filename?  CH376 can only really cope with 8.3, so
		// skip this entry and recurse to get the next one.
		if (dotIdx > 8)
		{
			return generateNextDirEntry();
		}

		std::fill_n(&m_dataBuffer[1], 11, 0x20);

		int baseLen = std::min(8, dotIdx);
		for (int idx = 0; idx < baseLen; idx++)
		{
			m_dataBuffer[idx + 1] = toupper(ourEntry->name[idx]);
			if (ourEntry->name[idx] == '\0')
			{
				baseLen = idx;
				break;
			}
		}

		// copy the extension (up to 3 chars, if we found one)
		if (ourEntry->name[dotIdx] == '.')
		{
			dotIdx++;
			for (int idx = 0; idx < 3; idx++)
			{
				if ((idx + dotIdx) >= strlen(ourEntry->name))
				{
					break;
				}
				m_dataBuffer[idx + 8 + 1] = toupper(ourEntry->name[idx + dotIdx]);
			}
		}

		m_dataBuffer[0xc] = 0;  // no attributes

		if (ourEntry->size >= u64(0x100000000))
			std::fill_n(&m_dataBuffer[0x1d], 4, 0xff);
		else
			put_u32le(&m_dataBuffer[0x1d], ourEntry->size);
	}
	else    // not a file or directory, recurse and hope the next one's better
	{
		return generateNextDirEntry();
	}

	m_dataLen = 32;
	return true;
}

void ch376_device::doNextFileRead()
{
	int lenToRead = (m_readLen >= 255) ? 255 : m_readLen;
	u32 actualRead;

	m_readLen -= lenToRead;
	m_file->read(&m_dataBuffer[1], m_file_pos, lenToRead, actualRead);
	m_dataBuffer[0] = (lenToRead & 0xff);
	m_file_pos += actualRead;
	m_dataLen = 256;
	m_dataPtr = 0;
}
