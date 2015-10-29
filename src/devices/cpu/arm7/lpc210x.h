// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __LPC2103__
#define __LPC2103__

#include "emu.h"
#include "arm7.h"
#include "arm7core.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class lpc210x_device : public arm7_cpu_device
{
public:
	lpc210x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

	// static configuration helpers

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();


private:
};


// device type definition
extern const device_type LPC2103;


#endif /// __LPC2103__
