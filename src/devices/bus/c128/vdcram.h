// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 128 VDC video RAM emulation

**********************************************************************/

#ifndef MAME_BUS_C128_VDCRAM_H
#define MAME_BUS_C128_VDCRAM_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_c128_vdc_ram_interface

class device_c128_vdc_ram_interface : public device_interface
{
public:
	virtual void map(address_map &map) ATTR_COLD = 0;

protected:
	device_c128_vdc_ram_interface(const machine_config &mconfig, device_t &device);
};


// ======================> c128_vdc_ram_slot_device

class c128_vdc_ram_slot_device : public device_t, public device_single_card_slot_interface<device_c128_vdc_ram_interface>
{
public:
	template <typename T>
	c128_vdc_ram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: c128_vdc_ram_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	c128_vdc_ram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
};


DECLARE_DEVICE_TYPE(C128_VDC_RAM_SLOT, c128_vdc_ram_slot_device)
DECLARE_DEVICE_TYPE(C128_VDC_RAM_4416, device_c128_vdc_ram_interface)
DECLARE_DEVICE_TYPE(C128_VDC_RAM_4464, device_c128_vdc_ram_interface)

void c128_vdc_ram_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_C128_VDCRAM_H
