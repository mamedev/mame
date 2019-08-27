// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

    Western Digital WD1000 Winchester Disk Controller

***************************************************************************/

#include "emu.h"
#include "machine/wd1000.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WD1000, wd1000_device, "wd1000", "Western Digital WD1000 Winchester Disk Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wd1000_device - constructor
//-------------------------------------------------

wd1000_device::wd1000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WD1000, tag, owner, clock)
	, m_intrq_cb(*this)
	, m_drq_cb(*this)
	, m_sector_base(0)
	, m_buffer_index(0)
	, m_buffer_end(0)
	, m_intrq(0)
	, m_drq(0)
	, m_stepping_rate(0x00)
	, m_command(0x00)
	, m_error(0x00)
	, m_precomp(0x00)
	, m_sector_count(0x00)
	, m_sector_number(0x00)
	, m_cylinder(0x0000)
	, m_sdh(0x00)
	, m_status(0x00)
{
}

void wd1000_device::set_sector_base(uint32_t base)
{
	m_sector_base = base;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1000_device::device_start()
{
	// Resolve callbacks
	m_intrq_cb.resolve();
	m_drq_cb.resolve();

	// Allocate timers
	m_seek_timer = timer_alloc(TIMER_SEEK);
	m_drq_timer = timer_alloc(TIMER_DRQ);

	// Empty buffer.
	m_buffer_index = 0;

	for (int i = 0; i < 4; i++)
	{
		char name[2];
		name[0] = '0' + i;
		name[1] = 0;
		m_drives[i].drive = subdevice<harddisk_image_device>(name);
		m_drives[i].cylinder = 0;
	}

	// Initialize the status as ready if the initial drive exists,
	// and assume it has been restored so seek is complete.
	if (m_drives[drive()].drive->exists())
		m_status |= S_RDY | S_SC;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd1000_device::device_reset()
{
	m_buffer_index = 0;
	m_sdh = 0x20;

	for (int i = 0; i < 4; i++)
		m_drives[i].cylinder = 0;

	if (m_drives[drive()].drive->exists())
		m_status |= S_RDY | S_SC;
}

//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void wd1000_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	case TIMER_SEEK:

		m_drives[drive()].cylinder = param;
		m_status |= S_SC;

		switch (m_command >> 4)
		{
		case CMD_RESTORE:
			cmd_restore();
			break;

		case CMD_SEEK:
			cmd_seek();
			break;

		case CMD_READ_SECTOR:
			cmd_read_sector();
			break;

		case CMD_WRITE_SECTOR:
			cmd_write_sector();
			break;

		case CMD_WRITE_FORMAT:
			cmd_format_sector();
			break;
		}

		break;

	case TIMER_DRQ:
		set_drq();
		break;
	}
}

void wd1000_device::set_error(int error)
{
	if (error)
	{
		m_error |= error;
		m_status |= S_ERR;
	}
	else
	{
		m_error = 0;
		m_status &= ~S_ERR;
	}
}

void wd1000_device::set_intrq(int state)
{
	if (m_intrq == 0 && state == 1)
	{
		m_intrq = 1;
		m_intrq_cb(1);
	}
	else if (m_intrq == 1 && state == 0)
	{
		m_intrq = 0;
		m_intrq_cb(0);
	}
}

void wd1000_device::set_drq()
{
	if ((m_status & S_DRQ) == 0)
	{
		m_status |= S_DRQ;
		if (!m_drq_cb.isnull())
			m_drq_cb(true);
	}
}

void wd1000_device::drop_drq()
{
	if (m_status & S_DRQ)
	{
		m_status &= ~S_DRQ;
		if (!m_drq_cb.isnull())
			m_drq_cb(false);
	}
}

attotime wd1000_device::get_stepping_rate()
{
	if (m_stepping_rate)
		return attotime::from_usec(500 * m_stepping_rate);
	else
		return attotime::from_usec(10);
}

void wd1000_device::start_command()
{
	m_status |= S_BSY;
	set_error(0);

	switch (m_command >> 4)
	{
	case CMD_RESTORE:
		m_stepping_rate = m_command & 0x0f;
		break;
	case CMD_READ_SECTOR:
		break;
	case CMD_WRITE_SECTOR:
		break;
	case CMD_WRITE_FORMAT:
		break;
	case CMD_SEEK:
		m_stepping_rate = m_command & 0x0f;
		break;
	}
}

void wd1000_device::end_command()
{
	m_status &= ~S_BSY;
	set_intrq(1);
}

int wd1000_device::get_lbasector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);
	int lbasector;

	if (m_cylinder > info->cylinders)
	{
		logerror("%s: Unexpected cylinder %d for range 0 to %d\n", machine().describe_context(), m_cylinder, info->cylinders - 1);
	}

	if (head() >= info->heads)
	{
		logerror("%s: Unexpected head %d for range 0 to %d\n", machine().describe_context(), head(), info->heads - 1);
	}

	int16_t sector = m_sector_number - m_sector_base;

	if (sector < 0 || sector >= info->sectors)
	{
		logerror("%s: Unexpected sector number %d for range %d to %d\n", machine().describe_context(), m_sector_number, m_sector_base, info->sectors + m_sector_base);
	}

	if (sector_bytes() != info->sectorbytes)
	{
		logerror("%s: Unexpected sector bytes %d, expected %d\n", machine().describe_context(), sector_bytes(), info->sectorbytes);
	}

	lbasector = m_cylinder;
	lbasector *= info->heads;
	lbasector += head();
	lbasector *= info->sectors;
	lbasector += sector;

	return lbasector;
}

uint8_t wd1000_device::data_r()
{
	uint8_t data = 0x00;

	if (machine().side_effects_disabled())
		return data;

	drop_drq();

	if (m_buffer_index >= m_buffer_end)
	{
		// Transfer has already completed.
		logerror("%s: Unexpected buffer read at %d beyond tail %d\n", machine().describe_context(), m_buffer_index, m_buffer_end);
		return data;
	}

	data = m_buffer[m_buffer_index++];

	if (m_buffer_index == m_buffer_end)
	{
		// Tranfer completed.
		if ((m_command >> 4) == CMD_READ_SECTOR)
		{
			uint8_t dma = BIT(m_command, 3);

			if (dma)
			{
				set_intrq(1);
			}
		}
		else
		{
			logerror("%s: Unexpected buffer read transfer for command %02x\n", machine().describe_context(), m_command);
		}
	}
	else
	{
		// Continue the transfer. A delay is implemented here to avoid
		// recursion in the DRQ handler.
		m_drq_timer->adjust(attotime::from_usec(1));
	}

	return data;
}

READ8_MEMBER( wd1000_device::read )
{
	if ((m_status & S_BSY) && offset != 7)
	{
		logerror("%s Unexpected register %d read while busy\n", machine().describe_context(), offset & 0x07);
		return m_status;
	}

	switch (offset & 0x07)
	{
	case 0:
		// Data register.
		return data_r();

	case 1:
		// Error register:
		//  bit 7 bad block detect
		//  bit 6 CRC error, data field
		//  bit 5 CRC error, ID field
		//  bit 4 ID not found
		//  bit 3 unused
		//  bit 2 Aborted Command
		//  bit 1 TR000 (track zero) error
		//  bit 0 DAM not found
		// The error register is only valid if the error bit in the
		// status register is set
		return m_error;

	case 2:
		// Sector count. This is only used in the format command where
		// it is decremented to zero.
		return m_sector_count;

	case 3:
		// Sector number.
		return m_sector_number;

	case 4:
		// Cylinder low byte C0-C7
		// This read clears DRQ. This is intended for the handling
		// aborted sector reads that leave DRQ asserted.
		if (!machine().side_effects_disabled())
		{
			set_intrq(0);
			drop_drq();
		}
		return m_cylinder & 0xff;

	case 5:
		// Cylinder high byte C8-C9
		return m_cylinder >> 8;

	case 6:
		// Size / head / drive
		return m_sdh;

	case 7:
		// Status.
		// bit 7 Busy
		// bit 6 Ready
		// bit 5 Write fault
		// bit 4 Seek complete
		// bit 3 Data request (DRQ)
		// bit 2 -
		// bit 1 -
		// bit 0 Error
		// return 'ready' + 'seek complete'
		// If the busy bit is set then no other bits are valid!!!
		//
		// Reading the status register clears the interrupt.
		//
		// TODO should reading the status register while busy reset
		// the interrupt?
		if (!machine().side_effects_disabled())
			set_intrq(0);

		return m_status;
	}

	return 0x00;
}

void wd1000_device::data_w(uint8_t data)
{
	drop_drq();

	if (m_buffer_index >= m_buffer_end)
	{
		// Transfer has already completed.
		logerror("%s: Unexpected buffer write at %d beyond tail %d\n", machine().describe_context(), m_buffer_index, m_buffer_end);
		return;
	}

	m_buffer[m_buffer_index++] = data;

	if (m_buffer_index == m_buffer_end)
	{
		// Tranfer completed.
		if ((m_command >> 4) == CMD_WRITE_SECTOR ||
		    (m_command >> 4) == CMD_WRITE_FORMAT)
		{
			m_status |= S_BSY;
			set_error(0);
			// Implied seek
			m_status &= ~S_SC;
			int amount = abs(m_drives[drive()].cylinder - m_cylinder);
			int target = m_cylinder;
			m_seek_timer->adjust(get_stepping_rate() * amount, target);
		}
		else
		{
			logerror("%s: Unexpected buffer write transfer for command %02x\n", machine().describe_context(), m_command);
		}
	}
	else
	{
		// Continue the transfer
		m_drq_timer->adjust(attotime::from_usec(1));
	}
}

WRITE8_MEMBER( wd1000_device::write )
{
	switch (offset & 0x07)
	{
	case 0:
		// Data register
		data_w(data);
		break;

	case 1:
		// Write precomp
		m_precomp = data;
		break;

	case 2:
		// Sector Count
		m_sector_count = data;
		break;

	case 3:
		// Sector Number
		m_sector_number = data;
		break;

	case 4:
		// Cylinder Low
		m_cylinder = (m_cylinder & 0xff00) | (data << 0);
		break;

	case 5:
		// Cylinder High
		m_cylinder = (m_cylinder & 0x00ff) | (data << 8);
		break;

	case 6:
	  {
		// Size / drive / head
		//   bit 7    : Honour error correction (WD-1001 only?)
		//   bit 6,5  : sector size (0: 256, 1: 512, 3: 128)
		//   bit 4,3  : drive select (0,1,2,3)
		//   bit 2,1,0: head select (0-7)
		uint8_t drive = (data & 0x18) >> 3;
		m_sdh = data;

		// Update the drive ready flag in the status register which
		// indicates the ready state for the selected drive.
		// TODO should this also set the SC flag?
		if (m_drives[drive].drive->exists())
			m_status |= S_RDY;
		else
			m_status &= ~S_RDY;

		break;
	  }

	case 7:
		// Command register
		// Load while the controller is not busy.
		// Task file must have been loaded??
		// Not if 'Seek complete' or 'Ready' are false, or 'Write fault' is true.

		if (m_status & S_BSY)
		{
			logerror("%s: Unexpected command %02x issued when already busy with %02x\n", machine().describe_context(), data, m_command);
			// TODO should it be ignored?
			//return;
		}

		// Interrupts are cleared and the error bit is reset
		// on receipt of a new command.
		set_intrq(0);
		set_error(0);

		if (!(m_status & S_RDY))
		{
			logerror("%s: Unexpected command %02x when not ready\n", machine().describe_context(), m_command);
			set_error(ERR_AC);
			end_command();
		}
		else
		{
			m_command = data;

			switch (m_command >> 4)
			{
			case CMD_RESTORE:
			  {
				start_command();
				// Schedule an implied seek.
				m_status &= ~S_SC;
				int amount = m_drives[drive()].cylinder;
				int target = 0;
				m_seek_timer->adjust(get_stepping_rate() * amount, target);
				break;
			  }

			case CMD_SEEK:
			case CMD_READ_SECTOR:
			  {
				start_command();
				int amount = abs(m_drives[drive()].cylinder - m_cylinder);
				int target = m_cylinder;
				m_seek_timer->adjust(get_stepping_rate() * amount, target);
				break;
			  }

			case CMD_WRITE_SECTOR:
			case CMD_WRITE_FORMAT:
				// These commands do not perform an implied
				// seek until all the data has been
				// transferred, so just start the transfer
				// here.
				m_buffer_index = 0;
				m_buffer_end = 512;
				set_drq();
				break;
			}

		}

		break;
	}
}

void wd1000_device::cmd_restore()
{
	end_command();
}

void wd1000_device::cmd_read_sector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	uint8_t dma = BIT(m_command, 3);

	hard_disk_read(file, get_lbasector(), m_buffer);

	m_buffer_index = 0;
	m_buffer_end = 512;

	m_status &= ~S_BSY;

	set_drq();

	if (!dma)
	{
		// Interrupt now, rather than at the end of the DMA transfer.
		set_intrq(1);
	}
}

void wd1000_device::cmd_write_sector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();

	if (m_buffer_index != sector_bytes())
	{
		logerror("%s: Unexpected unfilled buffer on write, only %d or %d bytes filled\n", machine().describe_context(), m_buffer_index, sector_bytes());
	}

	hard_disk_write(file, get_lbasector(), m_buffer);

	end_command();
}

void wd1000_device::cmd_format_sector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	uint8_t buffer[512];

	// The m_buffer appears to be loaded with an interleave table which is
	// not used here. The sectors are zero filled.

	for (int i = 0; i < m_sector_count; i++)
	{
		std::fill(std::begin(buffer), std::end(buffer), 0);
		hard_disk_write(file, get_lbasector(), buffer);
	}

	m_sector_count = 0;

	end_command();
}

void wd1000_device::cmd_seek()
{
	end_command();
}
