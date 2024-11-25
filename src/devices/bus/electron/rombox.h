// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Rombox

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_ROMBOX_H
#define MAME_BUS_ELECTRON_ROMBOX_H

#include "exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_rombox_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_rombox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom[2]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom4_load) { return load_rom(image, m_rom[3]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom5_load) { return load_rom(image, m_rom[4]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom6_load) { return load_rom(image, m_rom[5]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom7_load) { return load_rom(image, m_rom[6]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom8_load) { return load_rom(image, m_rom[7]); }

	required_device<electron_expansion_slot_device> m_exp;
	required_device_array<generic_slot_device, 8> m_rom;
	required_ioport m_option;

	uint8_t m_romsel;
	uint8_t m_rom_base;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ROMBOX, electron_rombox_device)


#endif /* MAME_BUS_ELECTRON_ROMBOX_H */
