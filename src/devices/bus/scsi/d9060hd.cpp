// license:BSD-3-Clause
// copyright-holders:smf
/*
 * D9060 - SASI + TANDON TM602S
 *
 */

#include "d9060hd.h"

// device type definition
const device_type D9060HD = &device_creator<d9060hd_device>;

d9060hd_device::d9060hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihd_device(mconfig, D9060HD, "D9060HD", tag, owner, clock, "d9060hd", __FILE__)
{
}

#define D9060HD_CMD_PHYSICAL_DEVICE_ID ( 0xc0 )
#define D9060HD_CMD_DRIVE_DIAGS ( 0xe3 )

void d9060hd_device::ExecCommand()
{
	switch( command[ 0 ] )
	{
	case D9060HD_CMD_PHYSICAL_DEVICE_ID:
	case D9060HD_CMD_DRIVE_DIAGS:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	default:
		scsihd_device::ExecCommand();
		break;
	}
}
