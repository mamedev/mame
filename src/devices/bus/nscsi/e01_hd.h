// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore SCSI hard disk

**********************************************************************/

#ifndef MAME_BUS_NSCSI_E01_HD_H
#define MAME_BUS_NSCSI_E01_HD_H

#pragma once

#include "bus/nscsi/hd.h"

class nscsi_e01_device : public nscsi_harddisk_device
{
public:
	nscsi_e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// nscsi_full_device implementation
	virtual void scsi_command() override;

private:
	bool ro650_series() const;
};

DECLARE_DEVICE_TYPE(NSCSI_E01, nscsi_e01_device)

#endif // MAME_BUS_NSCSI_E01_HD_H
