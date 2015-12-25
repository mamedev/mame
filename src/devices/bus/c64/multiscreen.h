// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Multiscreen cartridge emulation

**********************************************************************/

#pragma once

#ifndef __MULTISCREEN__
#define __MULTISCREEN__


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_multiscreen_cartridge_device

class c64_multiscreen_cartridge_device : public device_t,
											public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_multiscreen_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	UINT8 m_bank;
};


// device type definition
extern const device_type C64_MULTISCREEN;


#endif
