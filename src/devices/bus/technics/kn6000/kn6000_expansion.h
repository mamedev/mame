// license:GPL-2.0+
// copyright-holders:Felipe Sanches

#ifndef MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
#define MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H

#pragma once

// CN105, the 70-pin "TO HDD" connector on the SX-KN6000 and SX-KN6500 main
// board. The unit sees A1-A20, D16-D31 and two chip selects decoded on the
// main board: ISOROM at 0x97800000-0x97ffffff and HDDCS at 0x98060000-0x9806ffff.
// Not modelled: the two interrupts (HDDINT to IRQ1, PP.INT to IRQ4) and the
// tone generator's spare serial outputs, which the connector also carries.

class device_kn6000_expansion_interface;

class kn6000_expansion_connector : public device_t, public device_single_card_slot_interface<device_kn6000_expansion_interface>
{
public:
	template <typename T>
	kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: kn6000_expansion_connector(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// install the fitted unit's chip-select windows in the host program space
	void program_map(address_space_installer &space);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};

class device_kn6000_expansion_interface : public device_interface
{
public:
	// the unit sees A1-A20 of each chip select
	virtual void isorom_map(address_map &map) ATTR_COLD = 0;
	virtual void hddcs_map(address_map &map) ATTR_COLD = 0;

protected:
	device_kn6000_expansion_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(KN6000_EXPANSION, kn6000_expansion_connector)

void kn6000_expansion_intf(device_slot_interface &device);

#endif // MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
