// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xebec S1410 5.25" Winchester Disk Controller emulation

**********************************************************************/

#pragma once

#ifndef __S1410__
#define __S1410__

#include "scsihd.h"

class s1410_device  : public scsihd_device
{
public:
	// construction/destruction
	s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void ExecCommand();
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );
};


// device type definition
extern const device_type S1410;

#endif
