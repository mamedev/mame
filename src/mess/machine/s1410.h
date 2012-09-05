/**********************************************************************

    Xebec S1410 5.25" Winchester Disk Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __S1410__
#define __S1410__

#include "emu.h"
#include "machine/scsihd.h"

class s1410_device  : public scsihd_device
{
public:
	// construction/destruction
	s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void ExecCommand( int *transferLength );
	virtual void WriteData( UINT8 *data, int dataLength );

protected:
	// device-level overrides
	virtual void device_config_complete();
};


// device type definition
extern const device_type S1410;

#endif
