// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 128 COMAL 80 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __C128_COMAL80__
#define __C128_COMAL80__


#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c128_comal80_cartridge_device

class c128_comal80_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c128_comal80_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

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
extern const device_type C128_COMAL80;


#endif
