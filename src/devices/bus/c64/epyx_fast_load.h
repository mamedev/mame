// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Epyx Fast Load cartridge emulation

**********************************************************************/

#pragma once

#ifndef __EPYX_FAST_LOAD__
#define __EPYX_FAST_LOAD__


#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_epyx_fast_load_cartridge_device

class c64_epyx_fast_load_cartridge_device : public device_t,
											public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_epyx_fast_load_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	emu_timer *m_exrom_timer;
};


// device type definition
extern const device_type C64_EPYX_FAST_LOAD;


#endif
