// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "pc9801_cd.h"

// device type definition
DEFINE_DEVICE_TYPE(PC9801_CD, pc9801_cd_device, "pc9801_cd", "PC9801 CD-ROM Drive")

pc9801_cd_device::pc9801_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, PC9801_CD, tag, owner, clock)
{
}

// TODO: pinpoint what os2warp3 is unhappy for autodetection

// CD-260 wants SCSI inquiry like this, it will install then fumble around redbook tracks
// t10mmc::set_model("NEC     CD-ROM DRIVE:260 1.0");

// CD-50, which will initially autodetect then purged during install
// t10mmc::set_model("NEC     CD-ROM DRIVE:98 1.0 ");
// np2 T10SPC_CMD_INQUIRY defaults for CD-50
// t10mmc::set_model("NEC     CD-ROM DRIVE    1.0 ");
//  data[0] = 0x05; CD-ROM
//  data[1] = 0x80; Removable medium bit
//  data[2] = 0x00; ANSI
//  data[3] = 0x21; ATAPI spec v2, response data format
//  data[4] = 0x1f;
//  data[5] = 0;
//  data[6] = 0;
//  data[7] = 0;


void pc9801_cd_device::fill_buffer()
{
	atapi_cdrom_device::fill_buffer();
	m_status |= IDE_STATUS_DRDY | IDE_STATUS_SERV;
}

void pc9801_cd_device::process_buffer()
{
	atapi_cdrom_device::process_buffer();
	m_status |= IDE_STATUS_DRDY | IDE_STATUS_SERV;
}

void pc9801_cd_device::process_command()
{
	atapi_cdrom_device::process_command();
	switch(m_command)
	{
		case IDE_COMMAND_CHECK_POWER_MODE:
			m_status = IDE_STATUS_DRDY | IDE_STATUS_SERV;
			break;
	}
}
