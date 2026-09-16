// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsicd.h

***************************************************************************/

#ifndef MAME_BUS_SCSI_SCSICD_H
#define MAME_BUS_SCSI_SCSICD_H

#include "scsihle.h"
#include "machine/t10mmc.h"

class scsicd_device : public scsihle_device, public t10mmc
{
public:
	// construction/destruction
	scsicd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	scsicd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual bool exists() const override;
};

// device type definition
DECLARE_DEVICE_TYPE(SCSICD, scsicd_device)

#endif // MAME_BUS_SCSI_SCSICD_H
