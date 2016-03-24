// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rex ExOS cartridge emulation

**********************************************************************/

#pragma once

#ifndef __EXOS__
#define __EXOS__


#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_exos_cartridge_device

class c64_exos_cartridge_device : public device_t,
									public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_exos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
};


// device type definition
extern const device_type C64_EXOS;


#endif
