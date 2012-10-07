/*

scsibus.h

*/

#pragma once

#ifndef _SCSIBUS_H_
#define _SCSIBUS_H_

#include "machine/scsidev.h"

class scsibus_device : public device_t
{
public:
	// construction/destruction
	scsibus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	/* SCSI Bus read/write */

	void scsi_update();

protected:
	// device-level overrides
	virtual void device_start();

private:
	scsidev_device *devices[16];

	UINT32 data;
	int deviceCount;
};

#define MCFG_SCSIBUS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SCSIBUS, 0)

// device type definition
extern const device_type SCSIBUS;

#endif
