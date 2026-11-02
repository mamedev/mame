// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*********************************************************************

    Adaptec ACB-4070 RLL SCSI disk controller

*********************************************************************/

#include "emu.h"
#include "acb4070.h"

#include "multibyte.h"

#include <iterator>

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_ACB4070, nscsi_acb4070_device, "scsi_acb4070", "Adaptec ACB-4070 RLL SCSI Disk Controller")

nscsi_acb4070_device::nscsi_acb4070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_ACB4070, tag, owner, clock),
	m_buffer{ },
	m_mode{ }
{
}

void nscsi_acb4070_device::device_start()
{
	nscsi_harddisk_device::device_start();

	save_item(NAME(m_buffer));
	save_item(NAME(m_mode));
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

		default:
			nscsi_harddisk_device::scsi_command();
			break;
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

		default:
			nscsi_harddisk_device::scsi_put_data(id, pos, data);
			break;
	}
}
