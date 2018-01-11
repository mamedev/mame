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
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	image_init_result load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom4_load) { return load_rom(image, m_rom4); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom5_load) { return load_rom(image, m_rom5); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom6_load) { return load_rom(image, m_rom6); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom7_load) { return load_rom(image, m_rom7); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom8_load) { return load_rom(image, m_rom8); }

	required_device<generic_slot_device> m_rom1;
	required_device<generic_slot_device> m_rom2;
	required_device<generic_slot_device> m_rom3;
	required_device<generic_slot_device> m_rom4;
	required_device<generic_slot_device> m_rom5;
	required_device<generic_slot_device> m_rom6;
	required_device<generic_slot_device> m_rom7;
	required_device<generic_slot_device> m_rom8;
	required_ioport m_option;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ROMBOX, electron_rombox_device)


#endif /* MAME_BUS_ELECTRON_ROMBOX_H */
