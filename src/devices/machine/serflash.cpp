// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/* Serial Flash Device */

/* todo: cleanup, refactor etc. */
/* ghosteo.c is similar? */

#include "emu.h"
#include "machine/serflash.h"

#include <algorithm>

ALLOW_SAVE_TYPE(serflash_device::flash_state_t);

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SERFLASH, serflash_device, "serflash", "Serial Flash")

//-------------------------------------------------
//  serflash_device - constructor
//-------------------------------------------------

serflash_device::serflash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SERFLASH, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_length(0)
	, m_region(nullptr)
	, m_row_num(0)
	, m_flash_page_size(2048+64)
	, m_flash_state()
	, m_flash_enab(0)
	, m_flash_cmd_seq(0), m_flash_cmd_prev(0), m_flash_addr_seq(0), m_flash_read_seq(0)
	, m_flash_row(0), m_flash_col(0), m_flash_page_addr(0), m_flash_page_index(0), m_last_flash_cmd(0), m_flash_addr(0)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void serflash_device::device_start()
{
	m_length = machine().root_device().memregion(tag())->bytes();
	m_region = machine().root_device().memregion(tag())->base();

	m_row_num = m_length / m_flash_page_size;

	m_flashwritemap.resize(m_row_num);
	std::fill(m_flashwritemap.begin(), m_flashwritemap.end(), 0);

	m_flash_page_data.resize(m_flash_page_size);

	save_item(NAME(m_flash_state));
	save_item(NAME(m_flash_enab));
	save_item(NAME(m_flash_cmd_seq));
	save_item(NAME(m_flash_cmd_prev));
	save_item(NAME(m_flash_addr_seq));
	save_item(NAME(m_flash_read_seq));
	save_item(NAME(m_flash_row));
	save_item(NAME(m_flash_col));
	save_item(NAME(m_flash_page_addr));
	save_item(NAME(m_flash_page_index));
	save_item(NAME(m_flashwritemap));
	save_item(NAME(m_last_flash_cmd));
	save_item(NAME(m_flash_addr));
	save_item(NAME(m_flash_page_data));
}

void serflash_device::device_reset()
{
	m_flash_enab = 0;
	flash_hard_reset();

	m_last_flash_cmd = 0x00;
	m_flash_addr_seq = 0;
	m_flash_addr = 0;

	m_flash_page_addr = 0;
}

//-------------------------------------------------
//  serflash_default - called to initialize SERFLASH to
//  its default state
//-------------------------------------------------

void serflash_device::nvram_default()
{
}


//-------------------------------------------------
//  nvram_read - called to read SERFLASH from the
//  .nv file
//-------------------------------------------------

bool serflash_device::nvram_read(util::read_stream &file)
{
	if (m_length % m_flash_page_size) return false; // region size must be multiple of flash page size
	int size = m_length / m_flash_page_size;

	{
		uint32_t page;
		size_t actual;
		if (file.read(&page, 4, actual) || actual != 4)
			return false;
		while (page < size)
		{
			m_flashwritemap[page] = 1;
			if (file.read(m_region + page * m_flash_page_size, m_flash_page_size, actual) || actual != m_flash_page_size)
				return false;
			if (file.read(&page, 4, actual) || actual != 4)
				return false;
		}
	}

	return true;
}


//-------------------------------------------------
//  nvram_write - called to write SERFLASH to the
//  .nv file
//-------------------------------------------------

bool serflash_device::nvram_write(util::write_stream &file)
{
	if (m_length % m_flash_page_size) return false; // region size must be multiple of flash page size
	int size = m_length / m_flash_page_size;

	uint32_t page = 0;
	size_t actual;
	while (page < size)
	{
		if (m_flashwritemap[page])
		{
			if (file.write(&page, 4, actual) || actual != 4)
				return false;
			if (file.write(m_region + page * m_flash_page_size, m_flash_page_size, actual) || actual != m_flash_page_size)
				return false;
		}
		page++;
	}
	if (file.write(&page, 4, actual) || actual != 4)
		return false;

	return true;
}

void serflash_device::flash_hard_reset()
{
//  logerror("%08x FLASH: RESET\n", cpuexec_describe_context(machine));

	m_flash_state = flash_state_t::READ;

	m_flash_cmd_prev = -1;
	m_flash_cmd_seq = 0;

	m_flash_addr_seq = 0;
	m_flash_read_seq = 0;

	m_flash_row = 0;
	m_flash_col = 0;

	std::fill(m_flash_page_data.begin(), m_flash_page_data.end(), 0);
	m_flash_page_addr = 0;
	m_flash_page_index = 0;
}

void serflash_device::flash_enab_w(uint8_t data)
{
	//logerror("%08x FLASH: enab = %02X\n", m_maincpu->pc(), data);
	m_flash_enab = data;
}

void serflash_device::flash_change_state(flash_state_t state)
{
	m_flash_state = state;

	m_flash_cmd_prev = -1;
	m_flash_cmd_seq = 0;

	m_flash_read_seq = 0;
	m_flash_addr_seq = 0;

	//logerror("flash_change_state - FLASH: state = %s\n", m_flash_state_name[state]);
}

void serflash_device::flash_cmd_w(uint8_t data)
{
	if (!m_flash_enab)
		return;

	//logerror("%08x FLASH: cmd = %02X (prev = %02X)\n", m_maincpu->pc(), data, m_flash_cmd_prev);

	if (m_flash_cmd_prev == -1)
	{
		m_flash_cmd_prev = data;

		switch (data)
		{
			case 0x00:  // READ
				m_flash_addr_seq = 0;
				break;

			case 0x60:  // BLOCK ERASE
				m_flash_addr_seq = 2; // row address only
				break;

			case 0x70:  // READ STATUS
				flash_change_state( flash_state_t::READ_STATUS );
				break;

			case 0x80:  // PAGE / CACHE PROGRAM
				m_flash_addr_seq = 0;
				// this actually seems to be set with the next 2 writes?
				m_flash_page_addr = 0;
				break;

			case 0x90:  // READ ID
				flash_change_state( flash_state_t::READ_ID );
				break;

			case 0xff:  // RESET
				flash_change_state( flash_state_t::IDLE );
				break;

			default:
			{
				//logerror("%s FLASH: unknown cmd1 = %02X\n", machine().describe_context(), data);
			}
		}
	}
	else
	{
		switch (m_flash_cmd_prev)
		{
			case 0x00:  // READ
				if (data == 0x30)
				{
					if (m_flash_row < m_row_num)
					{
						std::copy_n(&m_region[m_flash_row * m_flash_page_size], m_flash_page_size, m_flash_page_data.begin());
						m_flash_page_addr = m_flash_col;
						m_flash_page_index = m_flash_row;
					}
					flash_change_state( flash_state_t::READ );

					//logerror("%08x FLASH: caching page = %04X\n", m_maincpu->pc(), m_flash_row);
				}
				break;

			case 0x60: // BLOCK ERASE
				if (data==0xd0)
				{
					flash_change_state( flash_state_t::BLOCK_ERASE );
					if (m_flash_row < m_row_num)
					{
						m_flashwritemap[m_flash_row] |= 1;
						std::fill_n(&m_region[m_flash_row * m_flash_page_size], m_flash_page_size, 0xff);
					}
					//logerror("erased block %04x (%08x - %08x)\n", m_flash_col, m_flash_col * m_flash_page_size,  ((m_flash_col+1) * m_flash_page_size)-1);
				}
				else
				{
					//logerror("unexpected 2nd command after BLOCK ERASE\n");
				}
				break;
			case 0x80:
				if (data==0x10)
				{
					flash_change_state( flash_state_t::PAGE_PROGRAM );
					if (m_flash_row < m_row_num)
					{
						m_flashwritemap[m_flash_row] |= (memcmp(m_region + m_flash_row * m_flash_page_size, &m_flash_page_data[0], m_flash_page_size) != 0);
						std::copy_n(m_flash_page_data.begin(), m_flash_page_size, &m_region[m_flash_row * m_flash_page_size]);
					}
					//logerror("re-written block %04x (%08x - %08x)\n", m_flash_row, m_flash_row * m_flash_page_size,  ((m_flash_row+1) * m_flash_page_size)-1);
				}
				else
				{
					//logerror("unexpected 2nd command after SPAGE PROGRAM\n");
				}
				break;


			default:
			{
				//logerror("%08x FLASH: unknown cmd2 = %02X (cmd1 = %02X)\n", m_maincpu->pc(), data, m_flash_cmd_prev);
			}
		}
	}
}

void serflash_device::flash_data_w(uint8_t data)
{
	if (!m_flash_enab)
		return;

	//logerror("flash data write %04x\n", m_flash_page_addr);
	if (m_flash_page_addr < m_flash_page_size)
	{
		m_flash_page_data[m_flash_page_addr] = data;
	}
	m_flash_page_addr++;
}

void serflash_device::flash_addr_w(uint8_t data)
{
	if (!m_flash_enab)
		return;

	//logerror("%08x FLASH: addr = %02X (seq = %02X)\n", m_maincpu->pc(), data, m_flash_addr_seq);

	switch( m_flash_addr_seq++ )
	{
		case 0:
			m_flash_col = (m_flash_col & 0xff00) | data;
			break;
		case 1:
			m_flash_col = (m_flash_col & 0x00ff) | (data << 8);
			break;
		case 2:
			m_flash_row = (m_flash_row & 0xffff00) | data;
			if (m_row_num <= 256)
			{
				m_flash_addr_seq = 0;
			}
			break;
		case 3:
			m_flash_row = (m_flash_row & 0xff00ff) | (data << 8);
			if (m_row_num <= 65536)
			{
				m_flash_addr_seq = 0;
			}
			break;
		case 4:
			m_flash_row = (m_flash_row & 0x00ffff) | (data << 16);
			m_flash_addr_seq = 0;
			break;
	}
}

uint8_t serflash_device::flash_io_r()
{
	uint8_t data = 0x00;
//  uint32_t old;

	if (!m_flash_enab)
		return 0xff;

	switch (m_flash_state)
	{
		case flash_state_t::READ_ID:
			//old = m_flash_read_seq;

			switch( m_flash_read_seq++ )
			{
				case 0:
					data = 0xEC;    // Manufacturer
					break;
				case 1:
					data = 0xF1;    // Device
					break;
				case 2:
					data = 0x00;    // XX
					break;
				case 3:
					data = 0x15;    // Flags
					m_flash_read_seq = 0;
					break;
			}

			//logerror("%08x FLASH: read %02X from id(%02X)\n", m_maincpu->pc(), data, old);
			break;

		case flash_state_t::READ:
			if (m_flash_page_addr > m_flash_page_size-1)
				m_flash_page_addr = m_flash_page_size-1;

			//old = m_flash_page_addr;

			data = m_flash_page_data[m_flash_page_addr++];

			//logerror("%08x FLASH: read data %02X from addr %03X (page %04X)\n", m_maincpu->pc(), data, old, m_flash_page_index);
			break;

		case flash_state_t::READ_STATUS:
			// bit 7 = writeable, bit 6 = ready, bit 5 = ready/true ready, bit 1 = fail(N-1), bit 0 = fail
			data = 0xe0;
			//logerror("%08x FLASH: read status %02X\n", m_maincpu->pc(), data);
			break;

		default:
		{
		//  logerror("%08x FLASH: unknown read in state %s\n",0x00/*m_maincpu->pc()*/, m_flash_state_name[m_flash_state]);
		}
	}

	return data;
}

uint8_t serflash_device::flash_ready_r()
{
	return 1;
}



uint8_t serflash_device::n3d_flash_r(offs_t offset)
{
	if (m_last_flash_cmd==0x70) return 0xe0;

	if (m_last_flash_cmd==0x00)
	{
		uint8_t retdat = m_flash_page_data[m_flash_page_addr];

		//logerror("n3d_flash_r %02x %04x\n", offset, m_flash_page_addr);

		m_flash_page_addr++;
		return retdat;
	}


	logerror("n3d_flash_r %02x\n", offset);
	return 0x00;

}


void serflash_device::n3d_flash_cmd_w(offs_t offset, uint8_t data)
{
	logerror("n3d_flash_cmd_w %02x %02x\n", offset, data);
	m_last_flash_cmd = data;

	if (data==0x00)
	{
		if (m_flash_addr < m_row_num)
		{
			std::copy_n(&m_region[m_flash_addr * m_flash_page_size], m_flash_page_size, m_flash_page_data.begin());
		}
	}
}

void serflash_device::n3d_flash_addr_w(offs_t offset, uint8_t data)
{
//  logerror("n3d_flash_addr_w %02x %02x\n", offset, data);

	m_flash_addr_seq++;

	if (m_flash_addr_seq==3)
	{
		m_flash_addr = (m_flash_addr & 0xffff00) | data;
		if (m_row_num <= 256)
		{
			m_flash_addr_seq = 0;
			m_flash_page_addr = 0;
			logerror("set flash block to %08x\n", m_flash_addr);
		}
	}
	if (m_flash_addr_seq==4)
	{
		m_flash_addr = (m_flash_addr & 0xff00ff) | data << 8;
		if (m_row_num <= 65536)
		{
			m_flash_addr_seq = 0;
			m_flash_page_addr = 0;
			logerror("set flash block to %08x\n", m_flash_addr);
		}
	}
	if (m_flash_addr_seq==5)
	{
		m_flash_addr = (m_flash_addr & 0x00ffff) | data << 16;
		m_flash_addr_seq = 0;
		m_flash_page_addr = 0;
		logerror("set flash block to %08x\n", m_flash_addr);
	}
}
