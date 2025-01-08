// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Electron Tube Interface

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_TUBE_H
#define MAME_BUS_ELECTRON_CART_TUBE_H

#include "slot.h"
#include "bus/bbc/tube/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_tube_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_device<bbc_tube_slot_device> m_tube;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_TUBE, electron_tube_device)


#endif // MAME_BUS_ELECTRON_CART_TUBE_H
