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
	aha1542_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	uint8_t aha1542_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void aha1542_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

};

// device type definition

extern const device_type AHA1542;

#endif
