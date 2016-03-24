// license:BSD-3-Clause
// copyright-holders:smf
#include "pc9801_cd.h"

// device type definition
const device_type PC9801_CD = &device_creator<pc9801_cd_device>;

pc9801_cd_device::pc9801_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	atapi_cdrom_device(mconfig, PC9801_CD, "PC9801 CD-ROM Drive", tag, owner, clock, "pc9801_cd", __FILE__)
{
}

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
