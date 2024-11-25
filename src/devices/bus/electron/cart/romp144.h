// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    JAFA Systems ROMPlus-144

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_ROMP144_H
#define MAME_BUS_ELECTRON_CART_ROMP144_H

#include "slot.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_romp144_device :
	public device_t,
	public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_romp144_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom0) { return load_rom(image, m_romslot[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1) { return load_rom(image, m_romslot[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2) { return load_rom(image, m_romslot[2]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3) { return load_rom(image, m_romslot[3]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom4) { return load_rom(image, m_romslot[4]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom5) { return load_rom(image, m_romslot[5]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom6) { return load_rom(image, m_romslot[6]); }

	required_device_array<generic_slot_device, 7> m_romslot;

	uint8_t m_rom_select;
	uint8_t m_rom_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ROMP144, electron_romp144_device)


#endif // MAME_BUS_ELECTRON_CART_ROMP144_H
