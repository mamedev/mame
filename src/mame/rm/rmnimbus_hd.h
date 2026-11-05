// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_RM_RMNIMBUS_HD_H
#define MAME_RM_RMNIMBUS_HD_H

#pragma once

#include "bus/nscsi/hd.h"

class rmnimbus_hd_device : public nscsi_harddisk_device
{
public:
	rmnimbus_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void scsi_command() override;
};

DECLARE_DEVICE_TYPE(RMNIMBUS_HD, rmnimbus_hd_device)

#endif // MAME_RM_RMNIMBUS_HD_H
