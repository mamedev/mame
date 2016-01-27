// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 SID cartridge emulation

**********************************************************************/

#pragma once

#ifndef __PLUS4_SID_CARTRIDGE__
#define __PLUS4_SID_CARTRIDGE__

#include "emu.h"
#include "exp.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "sound/dac.h"
#include "sound/mos6581.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_sid_cartridge_device

class plus4_sid_cartridge_device : public device_t,
										public device_plus4_expansion_card_interface
{
public:
	// construction/destruction
	plus4_sid_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_plus4_expansion_card_interface overrides
	virtual UINT8 plus4_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;
	virtual void plus4_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;
	virtual void plus4_breset_w(int state);

private:
	required_device<mos6581_device> m_sid;
	required_device<vcs_control_port_device> m_joy;
};


// device type definition
extern const device_type PLUS4_SID;


#endif
