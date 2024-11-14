// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Wizard Sidewinder Rom Expansion Board

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_SIDEWNDR_H
#define MAME_BUS_ELECTRON_SIDEWNDR_H

#include "exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_sidewndr_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_sidewndr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom[2]); }

	required_device<electron_expansion_slot_device> m_exp;
	required_memory_region m_exp_rom;
	required_device_array<generic_slot_device, 3> m_rom;
	required_ioport m_joy;

	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_SIDEWNDR, electron_sidewndr_device)


#endif /* MAME_BUS_ELECTRON_SIDEWNDR_H */
