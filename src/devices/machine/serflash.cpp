// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/* Serial Flash Device */

/* todo: cleanup, refactor etc. */
/* ghosteo.c is similar? */

#include "emu.h"
#include "machine/serflash.h"



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type SERFLASH = &device_creator<serflash_device>;

//-------------------------------------------------
//  serflash_device - constructor
//-------------------------------------------------

serflash_device::serflash_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SERFLASH, "Serial Flash", tag, owner, clock, "serflash", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_length(0), m_region(nullptr), m_flash_state(), m_flash_enab(0), m_flash_cmd_seq(0), m_flash_cmd_prev(0), m_flash_addr_seq(0), m_flash_read_seq(0), m_flash_row(0),
	m_flash_col(0), m_flash_page_addr(0), m_flash_page_index(0), m_last_flash_cmd(0), m_flash_addr(0)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void serflash_device::device_start()
{
	m_length = machine().root_device().memregion( tag() )->bytes();
	m_region = machine().root_device().memregion( tag() )->base();

	m_flashwritemap.resize(m_length / FLASH_PAGE_SIZE);
	memset(&m_flashwritemap[0], 0, m_length / FLASH_PAGE_SIZE);
}

void serflash_device::device_reset()
{
	m_flash_enab = 0;
	flash_hard_reset(machine());

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

void serflash_device::nvram_read(emu_file &file)
{
	if (m_length % FLASH_PAGE_SIZE) return; // region size must be multiple of flash page size
	int size = m_length / FLASH_PAGE_SIZE;


	if (file)
	{
		UINT32 page;
		file.read(&page, 4);
		while (page < size)
		{
			m_flashwritemap[page] = 1;
			file.read(m_region + page * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
			file.read(&page, 4);
		}
	}

}


//-------------------------------------------------
//  nvram_write - called to write SERFLASH to the
//  .nv file
//-------------------------------------------------

void serflash_device::nvram_write(emu_file &file)
{
	if (m_length % FLASH_PAGE_SIZE) return; // region size must be multiple of flash page size
	int size = m_length / FLASH_PAGE_SIZE;

	UINT32 page = 0;
	while (page < size)
	{
		if (m_flashwritemap[page])
		{
			file.write(&page, 4);
			file.write(m_region + page * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
		}
		page++;
	}
	file.write(&page, 4);
}

void serflash_device::flash_hard_reset(running_machine &machine)
{
//  logerror("%08x FLASH: RESET\n", cpuexec_describe_context(machine));

	m_flash_state = STATE_READ;

	m_flash_cmd_prev = -1;
	m_flash_cmd_seq = 0;

	m_flash_addr_seq = 0;
	m_flash_read_seq = 0;

	m_flash_row = 0;
	m_flash_col = 0;

	memset(m_flash_page_data, 0, FLASH_PAGE_SIZE);
	m_flash_page_addr = 0;
	m_flash_page_index = 0;
}

WRITE8_MEMBER( serflash_device::flash_enab_w )
{
	//logerror("%08x FLASH: enab = %02X\n", m_maincpu->pc(), data);
	m_flash_enab = data;
}

void serflash_device::flash_change_state(running_machine &machine, flash_state_t state)
{
	m_flash_state = state;

	m_flash_cmd_prev = -1;
	m_flash_cmd_seq = 0;

	m_flash_read_seq = 0;
	m_flash_addr_seq = 0;

	//logerror("flash_change_state - FLASH: state = %s\n", m_flash_state_name[state]);
}

WRITE8_MEMBER( serflash_device::flash_cmd_w )
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
				m_flash_addr_seq = 0;
				break;

			case 0x70:  // READ STATUS
				flash_change_state( space.machine(), STATE_READ_STATUS );
				break;

			case 0x80:  // PAGE / CACHE PROGRAM
				m_flash_addr_seq = 0;
				// this actually seems to be set with the next 2 writes?
				m_flash_page_addr = 0;
				break;

			case 0x90:  // READ ID
				flash_change_state( space.machine(), STATE_READ_ID );
				break;

			case 0xff:  // RESET
				flash_change_state( space.machine(), STATE_IDLE );
				break;

			default:
			{
				//logerror("%08x FLASH: unknown cmd1 = %02X\n", cpu_get_pc(space.device()), data);
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
					memcpy(m_flash_page_data, m_region + m_flash_row * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
					m_flash_page_addr = m_flash_col;
					m_flash_page_index = m_flash_row;

					flash_change_state( space.machine(), STATE_READ );

					//logerror("%08x FLASH: caching page = %04X\n", m_maincpu->pc(), m_flash_row);
				}
				break;

			case 0x60: // BLOCK ERASE
				if (data==0xd0)
				{
					flash_change_state( space.machine(), STATE_BLOCK_ERASE );
					m_flashwritemap[m_flash_col] |= 1;
					memset(m_region + m_flash_col * FLASH_PAGE_SIZE, 0xff, FLASH_PAGE_SIZE);
					//logerror("erased block %04x (%08x - %08x)\n", m_flash_col, m_flash_col * FLASH_PAGE_SIZE,  ((m_flash_col+1) * FLASH_PAGE_SIZE)-1);
				}
				else
				{
					//logerror("unexpected 2nd command after BLOCK ERASE\n");
				}
				break;
			case 0x80:
				if (data==0x10)
				{
					flash_change_state( space.machine(), STATE_PAGE_PROGRAM );
					m_flashwritemap[m_flash_row] |= (memcmp(m_region + m_flash_row * FLASH_PAGE_SIZE, m_flash_page_data, FLASH_PAGE_SIZE) != 0);
					memcpy(m_region + m_flash_row * FLASH_PAGE_SIZE, m_flash_page_data, FLASH_PAGE_SIZE);
					//logerror("re-written block %04x (%08x - %08x)\n", m_flash_row, m_flash_row * FLASH_PAGE_SIZE,  ((m_flash_row+1) * FLASH_PAGE_SIZE)-1);

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

WRITE8_MEMBER( serflash_device::flash_data_w )
{
	if (!m_flash_enab)
		return;

	//logerror("flash data write %04x\n", m_flash_page_addr);
	m_flash_page_data[m_flash_page_addr] = data;
	m_flash_page_addr++;
}

WRITE8_MEMBER( serflash_device::flash_addr_w )
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
			m_flash_row = (m_flash_row & 0xff00) | data;
			break;
		case 3:
			m_flash_row = (m_flash_row & 0x00ff) | (data << 8);
			m_flash_addr_seq = 0;
			break;
	}
}

READ8_MEMBER( serflash_device::flash_io_r )
{
	UINT8 data = 0x00;
//  UINT32 old;

	if (!m_flash_enab)
		return 0xff;

	switch (m_flash_state)
	{
		case STATE_READ_ID:
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

		case STATE_READ:
			if (m_flash_page_addr > FLASH_PAGE_SIZE-1)
				m_flash_page_addr = FLASH_PAGE_SIZE-1;

			//old = m_flash_page_addr;

			data = m_flash_page_data[m_flash_page_addr++];

			//logerror("%08x FLASH: read data %02X from addr %03X (page %04X)\n", m_maincpu->pc(), data, old, m_flash_page_index);
			break;

		case STATE_READ_STATUS:
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

READ8_MEMBER( serflash_device::flash_ready_r )
{
	return 1;
}



READ8_MEMBER(serflash_device::n3d_flash_r)
{
	if (m_last_flash_cmd==0x70) return 0xe0;

	if (m_last_flash_cmd==0x00)
	{
		UINT8 retdat = m_flash_page_data[m_flash_page_addr];

		//logerror("n3d_flash_r %02x %04x\n", offset, m_flash_page_addr);

		m_flash_page_addr++;
		return retdat;
	}


	logerror("n3d_flash_r %02x\n", offset);
	return 0x00;

}


WRITE8_MEMBER(serflash_device::n3d_flash_cmd_w)
{
	logerror("n3d_flash_cmd_w %02x %02x\n", offset, data);
	m_last_flash_cmd = data;

	if (data==0x00)
	{
		memcpy(m_flash_page_data, m_region + m_flash_addr * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);

	}

}

WRITE8_MEMBER(serflash_device::n3d_flash_addr_w)
{
//  logerror("n3d_flash_addr_w %02x %02x\n", offset, data);

	m_flash_addr_seq++;

	if (m_flash_addr_seq==3)
		m_flash_addr = (m_flash_addr & 0xffff00) | data;

	if (m_flash_addr_seq==4)
		m_flash_addr = (m_flash_addr & 0xff00ff) | data << 8;

	if (m_flash_addr_seq==5)
		m_flash_addr = (m_flash_addr & 0x00ffff) | data << 16;

	if (m_flash_addr_seq==5)
	{
		m_flash_addr_seq = 0;
		m_flash_page_addr = 0;
		logerror("set flash block to %08x\n", m_flash_addr);
	}
}
