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
	scsicd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	scsicd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	virtual void device_start();
};

// device type definition
extern const device_type SCSICD;

#endif
