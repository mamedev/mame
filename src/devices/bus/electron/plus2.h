// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Plus 2

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_PLUS2_H
#define MAME_BUS_ELECTRON_PLUS2_H

#include "exp.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "bus/electron/cart/slot.h"
#include "bus/bbc/userport/userport.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_plus2_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_plus2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom[2]); }

	required_device<input_merger_device> m_irqs;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<via6522_device> m_via;
	required_device_array<generic_slot_device, 3> m_rom;
	required_device_array<electron_cartslot_device, 2> m_cart;
	required_device<bbc_userport_slot_device> m_userport;

	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_PLUS2, electron_plus2_device)


#endif /* MAME_BUS_ELECTRON_PLUS2_H */
