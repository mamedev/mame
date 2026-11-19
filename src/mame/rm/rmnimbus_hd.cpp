// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Native SCSI hard disk for the RM Nimbus

    HDFORM.EXE reads the defect lists back after formatting, and fails the
    drive if READ DEFECT DATA is rejected.  This adds that command to the
    nscsi hard disk until the base device has it.
*/

#include "emu.h"
#include "rmnimbus_hd.h"

#include "multibyte.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(RMNIMBUS_HD, rmnimbus_hd_device, "rmnimbus_hd", "RM Nimbus SCSI Hard Disk")

rmnimbus_hd_device::rmnimbus_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, RMNIMBUS_HD, tag, owner, clock)
{
}

void rmnimbus_hd_device::scsi_command()
{
	switch (m_scsi_cmdbuf[0])
	{
		case SC_READ_DEFECT_DATA_10:
		{
			// header only: the emulated disk has no defects
			const uint8_t format = m_scsi_cmdbuf[2] & 0x1f;
			const uint16_t length = get_u16be(&m_scsi_cmdbuf[7]);
			m_scsi_cmdbuf[0] = 0;
			m_scsi_cmdbuf[1] = format;
			put_u16be(&m_scsi_cmdbuf[2], 0);
			if (length)
			{
				scsi_data_in(SBUF_MAIN, std::min<uint16_t>(length, 4));
			}
			scsi_status_complete(SS_GOOD);
			break;
		}

		default:
			nscsi_harddisk_device::scsi_command();
			break;
	}
}
