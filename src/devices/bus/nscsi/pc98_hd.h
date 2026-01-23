// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_BUS_NSCSI_PC98_HD_H
#define MAME_BUS_NSCSI_PC98_HD_H

#pragma once

#include "bus/nscsi/hd.h"
#include "machine/nscsi_bus.h"
#include "imagedev/harddriv.h"

class nscsi_pc98_hd_device : public nscsi_harddisk_device
{
public:
	nscsi_pc98_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void scsi_command() override;
	virtual attotime scsi_data_byte_period() override;

};

DECLARE_DEVICE_TYPE(NSCSI_PC98_HD, nscsi_pc98_hd_device)

#endif // MAME_BUS_NSCSI_HD_H
