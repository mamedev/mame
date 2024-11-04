// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsihd.h

***************************************************************************/

#ifndef MAME_BUS_SCSI_SCSIHD_H
#define MAME_BUS_SCSI_SCSIHD_H

#pragma once

#include "scsihle.h"
#include "machine/t10sbc.h"

class scsihd_device : public scsihle_device, public t10sbc
{
public:
	// construction/destruction
	scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	scsihd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual bool exists() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(SCSIHD, scsihd_device)

#endif // MAME_BUS_SCSI_SCSIHD_H
