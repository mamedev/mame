// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PS-64 speech cartridge emulation

**********************************************************************/

#pragma once

#ifndef __PS64__
#define __PS64__


#include "emu.h"
#include "exp.h"
#include "sound/votrax.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ps64_cartridge_device

class c64_ps64_cartridge_device : public device_t,
									public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_ps64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
};


// device type definition
extern const device_type C64_PS64;


#endif
