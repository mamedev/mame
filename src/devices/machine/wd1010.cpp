// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Western Digital WD1010-05 Winchester Disk Controller

***************************************************************************/

#include "emu.h"
#include "wd1010.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_CMD     (1U <<  1)
#define LOG_INT     (1U <<  2)
#define LOG_SEEK    (1U <<  3)
#define LOG_REGS    (1U <<  4)
#define LOG_DATA    (1U <<  5)

//#define VERBOSE  (LOG_CMD | LOG_INT | LOG_SEEK | LOG_REGS | LOG_DATA)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCMD(...)  LOGMASKED(LOG_CMD,  __VA_ARGS__)
#define LOGINT(...)  LOGMASKED(LOG_INT,  __VA_ARGS__)
#define LOGSEEK(...) LOGMASKED(LOG_SEEK, __VA_ARGS__)
#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGDATA(...) LOGMASKED(LOG_DATA, __VA_ARGS__)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WD1010, wd1010_device, "wd1010", "Western Digital WD1010-05")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wd1010_device - constructor
//-------------------------------------------------

wd1010_device::wd1010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WD1010, tag, owner, clock),
	m_out_intrq_cb(*this),
	m_out_bcr_cb(*this),
	m_in_data_cb(*this),
	m_out_data_cb(*this),
	m_intrq(0),
	m_brdy(0),
	m_stepping_rate(0x00),
	m_command(0x00),
	m_error(0x00),
	m_precomp(0x00),
	m_sector_count(0x00),
	m_sector_number(0x00),
	m_cylinder(0x0000),
	m_sdh(0x00),
	m_status(0x00)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wd1010_device::device_start()
{
	// get connected drives
	for (int i = 0; i < 4; i++)
	{
		char name[2];
		name[0] = '0' + i;
		name[1] = 0;
		m_drives[i].drive = subdevice<harddisk_image_device>(name);
		m_drives[i].head = 0;
		m_drives[i].cylinder = 0;
		m_drives[i].sector = 0;
	}

	// resolve callbacks
	m_out_intrq_cb.resolve_safe();
	m_out_bcr_cb.resolve_safe();
	m_in_data_cb.resolve_safe(0);
	m_out_data_cb.resolve_safe();

	// allocate timer
	m_seek_timer = timer_alloc(TIMER_SEEK);
	m_data_timer = timer_alloc(TIMER_DATA);

	// register for save states
	save_item(NAME(m_intrq));
	save_item(NAME(m_brdy));
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

void wd1010_device::device_reset()
{
}

//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void wd1010_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	case TIMER_SEEK:

		if ((m_command >> 4) != CMD_SCAN_ID)
		{
			LOGSEEK("Seek complete\n");
			m_drives[drive()].cylinder = param;
			m_status |= STATUS_SC;
		}

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
		case CMD_WRITE_FORMAT:
			cmd_write_sector();
			break;

		case CMD_SCAN_ID:
			cmd_scan_id();
			break;
		}

		break;

	case TIMER_DATA:

		// check if data is ready or continue waiting
		if (m_brdy)
			cmd_write_sector();
		else
			m_data_timer->adjust(attotime::from_usec(35));

		break;
	}
}

//-------------------------------------------------
//  set_error - set error and adjust status
//-------------------------------------------------

void wd1010_device::set_error(int error)
{
	if (error)
	{
		m_error |= error;
		m_status |= STATUS_ERR;
	}
	else
	{
		m_error = 0;
		m_status &= ~STATUS_ERR;
	}
}

//-------------------------------------------------
//  set_intrq - set interrupt status
//-------------------------------------------------

void wd1010_device::set_intrq(int state)
{
	if (m_intrq == 0 && state == 1)
	{
		LOGINT("INT 1\n");
		m_intrq = 1;
		m_out_intrq_cb(1);
	}
	else if (m_intrq == 1 && state == 0)
	{
		LOGINT("INT 0\n");
		m_intrq = 0;
		m_out_intrq_cb(0);
	}
}

//-------------------------------------------------
//  get_stepping_rate - calculate stepping rate
//-------------------------------------------------

attotime wd1010_device::get_stepping_rate()
{
	if (m_stepping_rate)
		return attotime::from_usec(500 * m_stepping_rate);
	else
		return attotime::from_usec(35);
}

//-------------------------------------------------
//  start_command - executed at the start of every command
//-------------------------------------------------

void wd1010_device::start_command()
{
	// now busy and command in progress, clear error
	m_status |= STATUS_BSY;
	m_status |= STATUS_CIP;
	set_error(0);

	switch (m_command >> 4)
	{
	case CMD_RESTORE:
		m_stepping_rate = m_command & 0x0f;
		LOGCMD("RESTORE\n");
		break;
	case CMD_READ_SECTOR:
		LOGCMD("READ SECTOR\n");
		break;
	case CMD_WRITE_SECTOR:
		LOGCMD("WRITE SECTOR\n");
		break;
	case CMD_SCAN_ID:
		LOGCMD("SCAN ID\n");
		break;
	case CMD_WRITE_FORMAT:
		LOGCMD("WRITE FORMAT ID\n");
		break;
	case CMD_SEEK:
		m_stepping_rate = m_command & 0x0f;
		LOGCMD("SEEK\n");
		break;
	}
}

//-------------------------------------------------
//  end_command - executed at end of every command
//-------------------------------------------------

void wd1010_device::end_command()
{
	m_out_bcr_cb(1);
	m_out_bcr_cb(0);

	m_status &= ~(STATUS_BSY | STATUS_CIP);

	set_intrq(1);
}

//-------------------------------------------------
//  get_lbasector - translate to lba
//-------------------------------------------------

int wd1010_device::get_lbasector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);
	int lbasector;

	lbasector = m_cylinder;
	lbasector *= info->heads;
	lbasector += head();
	lbasector *= info->sectors;
	lbasector += m_sector_number;

	return lbasector;
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( wd1010_device::drdy_w )
{
	if (state)
		m_status |= STATUS_RDY;
	else
		m_status &= ~STATUS_RDY;
}

WRITE_LINE_MEMBER( wd1010_device::brdy_w )
{
	m_brdy = state;
}

READ8_MEMBER( wd1010_device::read )
{
	// if the controller is busy all reads return the status register
	if (m_status & STATUS_BSY)
	{
		LOG("Read while busy, return STATUS: %02x\n", m_status);
		return m_status;
	}

	switch (offset & 0x07)
	{
	case 0:
		LOGREGS("RD INVALID\n");
		return 0;

	case 1:
		LOGREGS("RD ERROR: %02x\n", m_error);
		return m_error;

	case 2:
		LOGREGS("RD SECTOR COUNT: %02x\n", m_sector_count);
		return m_sector_count;

	case 3:
		LOGREGS("RD SECTOR NUMBER: %02x\n", m_sector_number);
		return m_sector_number;

	case 4:
		LOGREGS("RD CYLINDER L: %02x\n", m_cylinder & 0xff);
		return m_cylinder & 0xff;

	case 5:
		LOGREGS("RD CYLINDER H: %02x\n", m_cylinder >> 8);
		return m_cylinder >> 8;

	case 6:
		LOGREGS("RD SDH: %02x\n", m_sdh);
		return m_sdh;

	case 7:
		LOGREGS("RD STATUS: %02x\n", m_status);

		// reading the status register clears the interrupt
		if (!machine().side_effects_disabled())
			set_intrq(0);

		return m_status;
	}

	// should never get here
	return 0xff;
}

WRITE8_MEMBER( wd1010_device::write )
{
	switch (offset & 0x07)
	{
	case 0:
		LOGREGS("WR INVALID: %02x\n", data);
		break;

	case 1:
		LOGREGS("WR PRECOMP: %02x\n", data);
		m_precomp = data;
		break;

	case 2:
		LOGREGS("WR SECTOR COUNT: %02x\n", data);
		m_sector_count = data;
		break;

	case 3:
		LOGREGS("WR SECTOR NUMBER: %02x\n", data);
		m_sector_number = data;
		break;

	case 4:
		LOGREGS("WR CYLINDER L: %02x\n", data);
		m_cylinder = (m_cylinder & 0xff00) | (data << 0);
		break;

	case 5:
		LOGREGS("WR CYLINDER H: %02x\n", data);
		m_cylinder = (m_cylinder & 0x00ff) | (data << 8);
		break;

	case 6:
		LOGREGS("WR SDH: %02x\n", data);
		m_sdh = data;
		break;

	case 7:
		// writes to the command register are ignored when a command is in progress
		if (m_status & STATUS_CIP)
			return;

		// writing the command register clears the interrupt
		set_intrq(0);

		// check drive status
		if (!(m_status & STATUS_RDY))
		{
			// selected drive not ready
			LOG("--> Drive not ready, aborting\n");

			set_error(ERR_AC);
			end_command();
		}
		else
		{
			m_command = data;

			start_command();

			// all other command imply a seek
			if ((m_command >> 4) != CMD_SCAN_ID)
				m_status &= ~STATUS_SC;

			int amount = 0;
			int target = 0;

			switch (m_command >> 4)
			{
			case CMD_RESTORE:
				amount = m_drives[drive()].cylinder;
				target = 0;
				break;

			case CMD_SEEK:
			case CMD_READ_SECTOR:
			case CMD_WRITE_SECTOR:
			case CMD_WRITE_FORMAT:
				amount = abs(m_drives[drive()].cylinder - m_cylinder);
				target = m_cylinder;
				break;
			}

			if ((m_command >> 4) != CMD_SCAN_ID)
				LOGSEEK("Seeking %d cylinders to %d\n", amount, target);

			m_seek_timer->adjust(get_stepping_rate() * amount, target);
		}

		break;
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void wd1010_device::cmd_restore()
{
	LOGCMD("--> RESTORE done\n");
	end_command();
}

void wd1010_device::cmd_read_sector()
{
	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);

	m_out_bcr_cb(1);
	m_out_bcr_cb(0);

	// verify that we can read
	if (head() > info->heads)
	{
		// out of range
		LOG("--> Head out of range, aborting\n");

		set_error(ERR_AC);
		end_command();
	}
	else
	{
		uint8_t buffer[512];

		while (m_sector_count > 0)
		{
			LOGDATA("--> Transferring sector to buffer (lba = %08x)\n", get_lbasector());

			hard_disk_read(file, get_lbasector(), buffer);

			for (int i = 0; i < 512; i++)
				m_out_data_cb(buffer[i]);

			m_out_bcr_cb(1);
			m_out_bcr_cb(0);

			// save last read head and sector number
			m_drives[drive()].head = head();
			m_drives[drive()].sector = m_sector_number;

			if (BIT(m_command, 2))
			{
				m_sector_number++;
				m_sector_count--;
			}
			else
				break;
		}

		end_command();
	}
}

void wd1010_device::cmd_write_sector()
{
	if (!(m_status & STATUS_DRQ))
	{
		LOGDATA("Setting DATA REQUEST\n");
		m_status |= STATUS_DRQ;
		m_data_timer->adjust(attotime::from_usec(35));
		return;
	}

	if (m_brdy == 0)
	{
		m_data_timer->adjust(attotime::from_usec(35));
		return;
	}

	LOGDATA("Clearing DATA REQUEST\n");
	m_status &= ~STATUS_DRQ;

	hard_disk_file *file = m_drives[drive()].drive->get_hard_disk_file();
	uint8_t buffer[512];

	while (m_sector_count > 0)
	{
		if ((m_command >> 4) == CMD_WRITE_FORMAT)
		{
			// we ignore the format specification and fill everything with 0xe5
			std::fill(std::begin(buffer), std::end(buffer), 0xe5);
		}
		else
		{
			// get data for sector from buffer chip
			for (int i = 0; i < 512; i++)
				buffer[i] = m_in_data_cb();
		}

		hard_disk_write(file, get_lbasector(), buffer);

		// save last read head and sector number
		m_drives[drive()].head = head();
		m_drives[drive()].sector = m_sector_number;

		if (BIT(m_command, 2))
		{
			m_sector_number++;
			m_sector_count--;
		}
		else
			break;
	}

	end_command();
}

void wd1010_device::cmd_scan_id()
{
	// update head, sector size, sector number and cylinder
	m_sdh = (m_sdh & 0xf8) | (m_drives[drive()].head & 0x07);
	m_sector_number = m_drives[drive()].sector;
	m_cylinder = m_drives[drive()].cylinder;

	end_command();
}

void wd1010_device::cmd_seek()
{
	LOGCMD("--> SEEK done\n");
	end_command();
}
