// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Personal Peripheral Products Speakeasy 64 cartridge emulation

**********************************************************************/

#pragma once

#ifndef __C64_SPEAKEASY__
#define __C64_SPEAKEASY__

#include "exp.h"
#include "sound/votrax.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_speakeasy_t

class c64_speakeasy_t :  public device_t,
						 public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_speakeasy_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<votrax_sc01_device> m_votrax;
};


// device type definition
extern const device_type C64_SPEAKEASY;



#endif
