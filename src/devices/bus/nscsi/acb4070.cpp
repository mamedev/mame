// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*********************************************************************

    Adaptec ACB-4070 RLL SCSI disk controller

*********************************************************************/

#include "emu.h"
#include "acb4070.h"

#include "multibyte.h"

#include <iterator>
#include <vector>

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_ACB4070, nscsi_acb4070_device, "scsi_acb4070", "Adaptec ACB-4070 RLL SCSI Disk Controller")

nscsi_acb4070_device::nscsi_acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_ACB4070, tag, owner, clock),
	m_buffer{ },
	m_mode{ },
	m_search{ },
	m_defect_header{ },
	m_format_fill(0)
{
}

void nscsi_acb4070_device::device_start()
{
	nscsi_harddisk_device::device_start();

	save_item(NAME(m_buffer));
	save_item(NAME(m_mode));
	save_item(NAME(m_search));
	save_item(NAME(m_defect_header));
	save_item(NAME(m_format_fill));
}

void nscsi_acb4070_device::scsi_command()
{
	switch (m_scsi_cmdbuf[0])
	{
		case SC_WRITE_DATA_BUFFER:
			scsi_data_out(BUF_DATA, sizeof(m_buffer));
			scsi_status_complete(SS_GOOD);
			break;

		case SC_READ_DATA_BUFFER:
			scsi_data_in(BUF_DATA, sizeof(m_buffer));
			scsi_status_complete(SS_GOOD);
			break;

		case SC_MODE_SELECT_6:
			if (m_scsi_cmdbuf[4])
			{
				scsi_data_out(BUF_MODE, m_scsi_cmdbuf[4]);
			}
			scsi_status_complete(SS_GOOD);
			break;

		case SC_FORMAT_UNIT:
			// byte 2 is the fill pattern
			m_format_fill = m_scsi_cmdbuf[2];
			if (BIT(m_scsi_cmdbuf[1], 4))
			{
				// FmtData: the defect list header comes first, then the list it announces
				scsi_data_out(BUF_DEFECT, sizeof(m_defect_header));
			}
			else
			{
				format_unit();
				scsi_status_complete(SS_GOOD);
			}
			break;

		case SC_SEND_DIAGNOSTIC:
		{
			// the parameter list selects a test; every test passes
			const uint16_t length = get_u16be(&m_scsi_cmdbuf[3]);
			if (length)
			{
				scsi_data_out(BUF_DIAG, length);
			}
			scsi_status_complete(SS_GOOD);
			break;
		}

		case SC_SEARCH_DATA_EQUAL_10:
			// the header gives the length of the pattern, which is requested once the header is in
			scsi_data_out(BUF_SEARCH, sizeof(m_search));
			break;

		case SC_RAM_DIAGNOSTIC:
		case SC_CONTROLLER_DIAG:
			scsi_status_complete(SS_GOOD);
			break;

		default:
			nscsi_harddisk_device::scsi_command();
			break;
	}
}

void nscsi_acb4070_device::format_unit()
{
	const auto &info = image->get_info();
	std::vector<uint8_t> block(info.sectorbytes, m_format_fill);
	const uint32_t blocks = info.cylinders * info.heads * info.sectors;
	for (uint32_t lba = 0; lba < blocks; lba++)
	{
		image->write(lba, block.data());
	}
}

bool nscsi_acb4070_device::scsi_command_done(uint8_t command, uint8_t length)
{
	// vendor commands in groups 6 and 7 have 6-byte CDBs
	switch (command >> 5)
	{
		case 6:
		case 7:
			return length == 6;

		default:
			return nscsi_harddisk_device::scsi_command_done(command, length);
	}
}

uint8_t nscsi_acb4070_device::scsi_get_data(int id, int pos)
{
	if (id == BUF_DATA)
	{
		return m_buffer[pos];
	}

	return nscsi_harddisk_device::scsi_get_data(id, pos);
}

void nscsi_acb4070_device::scsi_put_data(int id, int pos, uint8_t data)
{
	switch (id)
	{
		case BUF_DATA:
			m_buffer[pos] = data;
			break;

		case BUF_MODE:
			// parameter header, block descriptor and the Adaptec drive parameter list
			if (pos < std::size(m_mode))
			{
				m_mode[pos] = data;
			}
			if (pos == std::size(m_mode) - 1)
			{
				LOG("MODE SELECT: %d cylinders, %d heads, %d sectors/track\n", get_u16be(&m_mode[13]), m_mode[15], m_mode[23]);
			}
			break;

		case BUF_DEFECT:
			m_defect_header[pos] = data;
			if (pos == sizeof(m_defect_header) - 1)
			{
				// an emulated disk has no defects to map out, so the list is only read
				const uint16_t length = get_u16be(&m_defect_header[2]);
				if (length)
				{
					scsi_data_out(BUF_DIAG, length);
				}
				format_unit();
				scsi_status_complete(SS_GOOD);
			}
			break;

		case BUF_SEARCH:
			m_search[pos] = data;
			if (pos == sizeof(m_search) - 1)
			{
				const uint16_t length = get_u16be(&m_search[18]);
				if (length)
				{
					scsi_data_out(BUF_DIAG, length);
				}

				// the disk has no defects, so a search for blocks that differ from the format pattern never matches
				scsi_status_complete(SS_GOOD);
			}
			break;

		case BUF_DIAG:
			break;

		default:
			nscsi_harddisk_device::scsi_put_data(id, pos, data);
			break;
	}
}
