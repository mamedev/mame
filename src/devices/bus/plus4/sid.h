// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 SID cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_PLUS4_SID_H
#define MAME_BUS_PLUS4_SID_H

#pragma once

#include "exp.h"
#include "bus/vcs_ctrl/ctrl.h"
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
	plus4_sid_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_plus4_expansion_card_interface overrides
	virtual uint8_t plus4_cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;
	virtual void plus4_cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) override;
	virtual void plus4_breset_w(int state);

private:
	required_device<mos6581_device> m_sid;
	required_device<vcs_control_port_device> m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(PLUS4_SID, plus4_sid_cartridge_device)


#endif // MAME_BUS_PLUS4_SID_H
