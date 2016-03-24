// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dela 64KB EPROM cartridge emulation

**********************************************************************/

#pragma once

#ifndef __DELA_EP64__
#define __DELA_EP64__


#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_dela_ep64_cartridge_device

class c64_dela_ep64_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_dela_ep64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<generic_slot_device> m_eprom1;
	required_device<generic_slot_device> m_eprom2;

	UINT8 m_bank;
	int m_reset;
	int m_rom0_ce;
	int m_rom1_ce;
	int m_rom2_ce;
};


// device type definition
extern const device_type C64_DELA_EP64;



#endif
