/***************************************************************************

    atapicdr.h

    ATAPI CDROM

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __ATAPICDR_H__
#define __ATAPICDR_H__

#include "atapihle.h"

class atapi_cdrom_device : public atapi_hle_device
{
public:
	atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void perform_diagnostic();
	virtual void identify_packet_device();
};

// device type definition
extern const device_type ATAPI_CDROM;

#endif
