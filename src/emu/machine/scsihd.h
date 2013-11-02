// license:MAME
// copyright-holders:smf
/***************************************************************************

 scsihd.h

***************************************************************************/

#ifndef _SCSIHD_H_
#define _SCSIHD_H_

#include "machine/scsihle.h"
#include "machine/t10sbc.h"

class scsihd_device : public scsihle_device,
	public t10sbc
{
public:
	// construction/destruction
	scsihd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	scsihd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual machine_config_constructor device_mconfig_additions() const;

	static struct harddisk_interface hd_intf;

protected:
	virtual void device_start();
};

// device type definition
extern const device_type SCSIHD;

#endif
