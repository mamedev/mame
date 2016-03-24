// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rex Datentechnik 256KB EPROM cartridge emulation

**********************************************************************/

#pragma once

#ifndef __REX_EP256__
#define __REX_EP256__


#include "emu.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_rex_ep256_cartridge_device

class c64_rex_ep256_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_rex_ep256_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	generic_slot_device *m_eproms[8];

	UINT8 m_bank, m_socket;
	int m_reset;
};


// device type definition
extern const device_type C64_REX_EP256;



#endif
