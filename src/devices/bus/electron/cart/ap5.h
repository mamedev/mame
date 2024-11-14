// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ACP Advanced Plus 5

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_AP5_H
#define MAME_BUS_ELECTRON_CART_AP5_H

#include "slot.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/tube/tube.h"
#include "bus/bbc/userport/userport.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_ap5_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_ap5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// electron_cart_interface implementation
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_romslot[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_romslot[1]); }

	required_device<input_merger_device> m_irqs;
	required_device<via6522_device> m_via;
	required_device<bbc_tube_slot_device> m_tube;
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<bbc_userport_slot_device> m_userport;
	required_device_array<generic_slot_device, 2> m_romslot;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_AP5, electron_ap5_device)


#endif // MAME_BUS_ELECTRON_CART_AP5_H
