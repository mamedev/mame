// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "pc9801_sasi.h"

#define SASI_CMD_SPECIFY 0xc2 // according to x68k_hdc.c

DEFINE_DEVICE_TYPE(PC9801_SASI, pc9801_sasi_device, "pc9801_sasi", "PC9801 SASI Controller")

pc9801_sasi_device::pc9801_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scsihd_device(mconfig, PC9801_SASI, tag, owner, clock)
{
}

void pc9801_sasi_device::ExecCommand()
{
	switch(command[0])
	{
		case SASI_CMD_SPECIFY:
			m_phase = SCSI_PHASE_DATAOUT;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = 10;
			break;

		default:
			scsihd_device::ExecCommand();
			break;
	}
}
