// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
#ifndef MAME_BUS_NSCSI_D9060HD_H
#define MAME_BUS_NSCSI_D9060HD_H

#pragma once

#include "bus/nscsi/hd.h"

class nscsi_d9060hd_device : public nscsi_harddisk_device
{
public:
	nscsi_d9060hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
	virtual void scsi_command() override;

private:
	enum : uint8_t
	{
		SC_PHYSICAL_DEVICE_ID = 0xc0,
		SC_DRIVE_DIAGNOSTICS  = 0xe3
	};
};

DECLARE_DEVICE_TYPE(NSCSI_D9060HD, nscsi_d9060hd_device)

#endif // MAME_BUS_NSCSI_D9060HD_H
