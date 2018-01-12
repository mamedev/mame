// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Rombox Plus

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_ROMBOXP_H
#define MAME_BUS_ELECTRON_ROMBOXP_H

#include "exp.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_romboxp_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_romboxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE_LINE_MEMBER(busy_w);

	image_init_result load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_rom[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_rom[1]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom3_load) { return load_rom(image, m_rom[2]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom4_load) { return load_rom(image, m_rom[3]); }

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart1_load) { return load_cart(image, m_cart[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart2_load) { return load_cart(image, m_cart[1]); }

	required_memory_region m_exp_rom;
	required_device_array<generic_slot_device, 4> m_rom;
	required_device_array<generic_slot_device, 2> m_cart;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_ioport m_option;

	int m_centronics_busy;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_ROMBOXP, electron_romboxp_device)


#endif /* MAME_BUS_ELECTRON_ROMBOXP_H */
