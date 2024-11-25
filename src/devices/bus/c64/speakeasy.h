// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Personal Peripheral Products Speakeasy 64 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_SPEAKEASY_H
#define MAME_BUS_C64_SPEAKEASY_H

#pragma once

#include "exp.h"
#include "sound/votrax.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_speakeasy_cartridge_device

class c64_speakeasy_cartridge_device : public device_t, public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_speakeasy_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<votrax_sc01_device> m_votrax;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_SPEAKEASY, c64_speakeasy_cartridge_device)


#endif // MAME_BUS_C64_SPEAKEASY_H
