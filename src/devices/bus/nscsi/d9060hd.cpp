// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/*********************************************************************

    Commodore D9060/D9090 SASI controller and Tandon TM602S/TM603S drive

*********************************************************************/

#include "emu.h"
#include "d9060hd.h"

DEFINE_DEVICE_TYPE(NSCSI_D9060HD, nscsi_d9060hd_device, "scsi_d9060hd", "Commodore D9060 SASI Hard Disk")

nscsi_d9060hd_device::nscsi_d9060hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_D9060HD, tag, owner, clock)
{
}

bool nscsi_d9060hd_device::scsi_command_done(uint8_t command, uint8_t length)
{
	// the vendor-specific groups use 6-byte commands
	switch (command >> 5)
	{
		case 6:
		case 7:
			return length == 6;

		default:
			return nscsi_harddisk_device::scsi_command_done(command, length);
	}
}

void nscsi_d9060hd_device::scsi_command()
{
	switch (m_scsi_cmdbuf[0])
	{
		case SC_PHYSICAL_DEVICE_ID:
		case SC_DRIVE_DIAGNOSTICS:
			scsi_status_complete(SS_GOOD);
			break;

		default:
			nscsi_harddisk_device::scsi_command();
			break;
	}
}
