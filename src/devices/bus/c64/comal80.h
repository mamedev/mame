// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMAL 80 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __COMAL80__
#define __COMAL80__


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_comal80_cartridge_device

class c64_comal80_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_comal80_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	uint8_t m_bank;
};


// device type definition
extern const device_type C64_COMAL80;


#endif
