// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

 scsicd.h

***************************************************************************/

#ifndef _SCSICD_H_
#define _SCSICD_H_

#include "scsihle.h"
#include "machine/t10mmc.h"

class scsicd_device : public scsihle_device,
	public t10mmc
{
public:
	// construction/destruction
	scsicd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	scsicd_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
};

// device type definition
extern const device_type SCSICD;

#endif
