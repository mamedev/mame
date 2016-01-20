// license:BSD-3-Clause
// copyright-holders:Darkstar
/**********************************************************************
 *
 *    Adaptec AHA-1542{,C,CF} SCSI Controller
 *
 **********************************************************************



 **********************************************************************/

#pragma once

#ifndef __AHA1542__
#define __AHA1542__


#include "emu.h"
#include "isa.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> aha1542_device

class aha1542_device : public device_t,
						public device_isa16_card_interface
{
public:
	// construction/destruction
	aha1542_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER( aha1542_r );
	DECLARE_WRITE8_MEMBER( aha1542_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

};

// device type definition

extern const device_type AHA1542;

#endif
