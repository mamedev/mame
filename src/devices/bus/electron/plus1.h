// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ALA11 - Acorn Plus 1

**********************************************************************/


#ifndef MAME_BUS_ELECTRON_PLUS1_H
#define MAME_BUS_ELECTRON_PLUS1_H

#include "exp.h"
#include "machine/adc0844.h"
#include "machine/input_merger.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/centronics/ctronics.h"
#include "bus/electron/cart/slot.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_plus1_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	// construction/destruction
	electron_plus1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	electron_plus1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

	required_device<input_merger_device> m_irqs;
	required_memory_region m_exp_rom;
	required_device<electron_cartslot_device> m_cart_sk1;
	required_device<electron_cartslot_device> m_cart_sk2;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<bbc_analogue_slot_device> m_analogue;
	required_device<adc0844_device> m_adc;

	uint8_t m_romsel;
	int m_centronics_busy;
	int m_adc_ready;
};


class electron_ap1_device : public electron_plus1_device
{
public:
	electron_ap1_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry* device_rom_region() const override;
};


class electron_ap6_device : public electron_plus1_device
{
public:
	electron_ap6_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry* device_rom_region() const override;
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

	required_device_array<generic_slot_device, 6> m_rom;
	required_ioport m_links;

	std::unique_ptr<uint8_t[]> m_ram;
	bool m_bank_locked[2];
};



// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_PLUS1, electron_plus1_device)
DECLARE_DEVICE_TYPE(ELECTRON_AP1, electron_ap1_device)
DECLARE_DEVICE_TYPE(ELECTRON_AP6, electron_ap6_device)


#endif /* MAME_BUS_ELECTRON_PLUS1_H */
