// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 16KB EPROM cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_16KB_H
#define MAME_BUS_C64_16KB_H

#pragma once


#include "exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_16kb_cartridge_device

class c64_16kb_cartridge_device : public device_t, public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_16kb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_ioport m_sw1;
	required_device<generic_slot_device> m_low;
	required_device<generic_slot_device> m_high;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_16KB, c64_16kb_cartridge_device)

#endif // MAME_BUS_C64_16KB_H
