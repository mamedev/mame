// license:BSD-3-Clause
// copyright-holders:68bit
/***************************************************************************

    Western Digital WD1000 Winchester Disk Controller

    References: WD1000 OEM Manual, sections 5-7; WD1001 OEM Manual
    80-031003-00 (February 1983), section 7.4.1.

    Sector images store logical sector data, not track ID fields. Format Track
    zeroes the sectors whose IDs the table lists with a good-block flag;
    bad-block marks, out-of-range and duplicate IDs cannot be represented and
    are ignored, so marked sectors stay readable and unlisted sectors keep
    their data. Rotational timing, internal retries and physical interleave
    are not modelled.

***************************************************************************/

#include "emu.h"
#include "wd1000.h"

#include <algorithm>
#include <iterator>

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
	, m_drives(*this, "%u", 0)
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
	// Allocate timers
	m_seek_timer = timer_alloc(FUNC(wd1000_device::update_seek), this);
	m_drq_timer = timer_alloc(FUNC(wd1000_device::delayed_drq), this);

	// Empty buffer.
	m_buffer_index = 0;

	for (int i = 0; i < 4; i++)
		m_drive_cylinder[i] = 0;

	// Initialize the status as ready if the initial drive exists,
	// and assume it has been restored so seek is complete.
	if (m_drives[drive()] && m_drives[drive()]->exists())
		m_status |= S_RDY | S_SC;

	save_item(NAME(m_drive_cylinder));
	save_item(NAME(m_sector_base));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_index));
	save_item(NAME(m_buffer_end));
	save_item(NAME(m_intrq));
	save_item(NAME(m_drq));
	save_item(NAME(m_stepping_rate));
	save_item(NAME(m_command));
	save_item(NAME(m_error));
	save_item(NAME(m_precomp));
	save_item(NAME(m_sector_count));
	save_item(NAME(m_sector_number));
	save_item(NAME(m_cylinder));
	save_item(NAME(m_sdh));
	save_item(NAME(m_status));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wd1000_device::device_reset()
{
	m_seek_timer->adjust(attotime::never);
	drop_drq();
	set_intrq(0);
	m_buffer_index = m_buffer_end = 0;
	m_command = 0;
	m_error = 0;
	m_status = 0;

	// Master reset state (OEM manual 2.2.9): 7.5 ms stepping rate, write
	// precompensation from cylinder 128, Sector Count 1, other registers clear.
	m_stepping_rate = 0x0f;
	m_precomp = 128 / 4;
	m_sector_count = 1;
	m_sector_number = 0;
	m_cylinder = 0;
	m_sdh = 0;

	for (int i = 0; i < 4; i++)
		m_drive_cylinder[i] = 0;

	drive_ready();
}

//-------------------------------------------------
//  update_seek -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wd1000_device::update_seek)
{
	if (!drive_ready())
	{
		set_error(ERR_AC);
		if ((m_command >> 4) == CMD_READ_SECTOR)
			cmd_read_sector();
		else
			end_command();
		return;
	}

	m_drive_cylinder[drive()] = param;
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
}

//-------------------------------------------------
//  delayed_drq - set DRQ after a necessary delay
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wd1000_device::delayed_drq)
{
	if (m_buffer_index < m_buffer_end)
		set_drq();
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
		m_drq = 1;
		m_status |= S_DRQ;
		m_drq_cb(true);
	}
}

void wd1000_device::drop_drq()
{
	m_drq_timer->adjust(attotime::never);
	m_drq = 0;
	if (m_status & S_DRQ)
	{
		m_status &= ~S_DRQ;
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
}

void wd1000_device::end_command()
{
	drop_drq();
	m_buffer_index = m_buffer_end = 0;
	m_status &= ~S_BSY;
	set_intrq(1);
}

bool wd1000_device::drive_ready()
{
	if (m_drives[drive()] && m_drives[drive()]->exists())
	{
		m_status |= S_RDY;
		if (!(m_status & S_BSY))
			m_status |= S_SC;
		return true;
	}
	m_status &= ~(S_RDY | S_SC);
	return false;
}

bool wd1000_device::validate_id_field()
{
	harddisk_image_device *file = m_drives[drive()];
	const auto &info = file->get_info();
	if ((m_cylinder >= info.cylinders) || (head() >= info.heads))
		return false;
	const int16_t sector = m_sector_number - m_sector_base;
	if ((sector < 0) || (sector >= info.sectors) || (sector_bytes() != info.sectorbytes))
		return false;
	return true;
}

int wd1000_device::get_lbasector()
{
	harddisk_image_device *file = m_drives[drive()];
	const auto &info = file->get_info();
	int lbasector;

	int16_t sector = m_sector_number - m_sector_base;

	lbasector = m_cylinder;
	lbasector *= info.heads;
	lbasector += head();
	lbasector *= info.sectors;
	lbasector += sector;

	return lbasector;
}

uint8_t wd1000_device::data_r()
{
	uint8_t data = 0x00;

	if (machine().side_effects_disabled())
		return data;

	drop_drq();

	if ((m_command >> 4) != CMD_READ_SECTOR || m_buffer_index >= m_buffer_end)
	{
		// No read transfer in progress.
		logerror("%s: Unexpected buffer read at %d beyond tail %d\n", machine().describe_context(), m_buffer_index, m_buffer_end);
		return data;
	}

	data = m_buffer[m_buffer_index++];

	if (m_buffer_index == m_buffer_end)
	{
		// Transfer completed: in DMA mode the interrupt follows the last byte.
		if (BIT(m_command, 3))
			set_intrq(1);
	}
	else
	{
		// Continue the transfer. A delay is implemented here to avoid
		// recursion in the DRQ handler.
		m_drq_timer->adjust(attotime::from_usec(1));
	}

	return data;
}

uint8_t wd1000_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
		drive_ready();
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

	if (((m_command >> 4) != CMD_WRITE_SECTOR && (m_command >> 4) != CMD_WRITE_FORMAT) || m_buffer_index >= m_buffer_end)
	{
		// No write transfer in progress.
		logerror("%s: Unexpected buffer write at %d beyond tail %d\n", machine().describe_context(), m_buffer_index, m_buffer_end);
		return;
	}

	m_buffer[m_buffer_index++] = data;

	if (m_buffer_index == m_buffer_end)
	{
		// Transfer completed: the command starts with an implied seek.
		m_status |= S_BSY;
		set_error(0);
		m_status &= ~S_SC;
		int amount = abs(m_drive_cylinder[drive()] - m_cylinder);
		int target = m_cylinder;
		m_seek_timer->adjust(get_stepping_rate() * amount, target);
	}
	else
	{
		// Continue the transfer
		m_drq_timer->adjust(attotime::from_usec(1));
	}
}

void wd1000_device::write(offs_t offset, uint8_t data)
{
	// The task file may only be loaded while the controller is not busy.
	if (m_status & S_BSY)
		return;

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
		m_sdh = data;
		drive_ready();

		break;
	  }

	case 7:
		// A new command replaces any unconsumed buffer transfer. In particular,
		// a delayed DRQ from the previous transfer must not survive it.
		drop_drq();
		m_buffer_index = m_buffer_end = 0;
		set_intrq(0);
		set_error(0);
		m_status &= ~S_WF;
		m_command = data;
		drive_ready();

		switch (m_command >> 4)
		{
		case CMD_RESTORE:
		case CMD_SEEK:
			// The stepping rate is stored, and Restore clears the cylinder
			// registers, before the drive status is checked.
			m_stepping_rate = m_command & 0x0f;
			if ((m_command >> 4) == CMD_RESTORE)
				m_cylinder = 0;
			if (!(m_status & S_RDY))
			{
				set_error(ERR_AC);
				end_command();
				break;
			}
			[[fallthrough]];
		case CMD_READ_SECTOR:
		  {
			start_command();
			m_status &= ~S_SC;
			int amount = abs(m_drive_cylinder[drive()] - m_cylinder);
			m_seek_timer->adjust(get_stepping_rate() * amount, m_cylinder);
			break;
		  }

		case CMD_WRITE_SECTOR:
		case CMD_WRITE_FORMAT:
			// Both commands consume one sector-sized buffer before checking
			// drive status and seeking (OEM manual 6.4). This also applies to
			// commands that will fail, so DMA/PIO hosts can finish normally.
			m_buffer_end = sector_bytes();
			if (m_buffer_end)
			{
				set_drq();
			}
			else
			{
				set_error(ERR_AC);
				end_command();
			}
			break;
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
	if (!sector_bytes())
	{
		set_error(ERR_AC);
		end_command();
		return;
	}

	// A failed read still offers the buffer and follows the normal PIO/DMA
	// interrupt sequence (OEM manual 6.3.1.4 and 7.3.4). These bytes are
	// invalid; a sector image cannot supply the physical read's damaged data.
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
	if (!drive_ready())
		set_error(ERR_AC);
	else if (!validate_id_field())
		set_error(ERR_ID);
	else if (!m_drives[drive()]->read(get_lbasector(), m_buffer))
		set_error(ERR_CRC);

	m_buffer_index = 0;
	m_buffer_end = sector_bytes();
	m_status &= ~S_BSY;

	// PIO interrupt precedes DRQ; DMA interrupt follows the last byte.
	if (!BIT(m_command, 3))
		set_intrq(1);
	set_drq();
}

void wd1000_device::cmd_write_sector()
{
	if (!drive_ready())
	{
		set_error(ERR_AC);
	}
	else if (!validate_id_field())
	{
		set_error(ERR_ID);
	}
	else if (!m_drives[drive()]->write(get_lbasector(), m_buffer))
	{
		// Map an image write failure to the drive's Write Fault indication.
		// Retrying a new command may succeed if the backend recovers.
		m_status |= S_WF;
		set_error(ERR_AC);
	}
	end_command();
}

void wd1000_device::cmd_format_sector()
{
	if (!drive_ready())
	{
		set_error(ERR_AC);
		end_command();
		return;
	}

	harddisk_image_device *file = m_drives[drive()];
	const auto &info = file->get_info();
	if (m_cylinder >= info.cylinders || head() >= info.heads || sector_bytes() != info.sectorbytes)
	{
		// The image has no such track, or a different sector size.
		set_error(ERR_AC);
		end_command();
		return;
	}

	// The table holds a (bad-block flag, sector ID) pair for each physical
	// sector, padded to the sector size; Sector Number is not used, and a
	// Sector Count of zero formats 256 sectors. The image only stores data for
	// its existing sector IDs, so zero each ID listed once with a good-block
	// flag and ignore what cannot be represented: bad-block marks (marked
	// sectors stay readable), out-of-range and duplicate IDs.
	const unsigned entries = std::min<unsigned>(m_sector_count ? m_sector_count : 256, m_buffer_end / 2);
	const uint32_t track = (m_cylinder * info.heads + head()) * info.sectors;
	const uint8_t buffer[512] = {};
	bool seen[256] = {};
	for (unsigned i = 0; i < entries; i++)
	{
		const uint8_t flag = m_buffer[2 * i];
		const uint8_t id = m_buffer[2 * i + 1];
		const int sector = id - m_sector_base;
		if (flag || sector < 0 || sector >= info.sectors || seen[sector])
		{
			logerror("Format C%u H%u: ignoring entry %u (flag %02x, ID %02x)\n", m_cylinder, head(), i, flag, id);
			continue;
		}

		seen[sector] = true;
		if (!file->write(track + sector, buffer))
		{
			// Sector Count is left at the number of sectors not formatted.
			m_sector_count -= i;
			m_status |= S_WF;
			set_error(ERR_AC);
			end_command();
			return;
		}
	}

	m_sector_count = 0;
	end_command();
}

void wd1000_device::cmd_seek()
{
	end_command();
}
