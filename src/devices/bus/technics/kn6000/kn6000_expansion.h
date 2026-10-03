// license:GPL-2.0+
// copyright-holders:Felipe Sanches

#ifndef MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
#define MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H

#pragma once

// Expansion connector on the SX-KN6000 and SX-KN6500 main board. The KN6500
// service manual shows it as CN106, a 70-pin connector labelled "TO HDD", and
// the chip-select decoder on the same sheet emits EXP.CS0 and EXP.CS1 for it.
//
// Signals carried on CN106, per the KN6500 service manual's pin table:
//   HDDCS  HDDINT  PP.INT                chip select and interrupts for the unit
//   R/NW  RE  WE2  WE3  IORST  ISOROM    bus strobes, I/O reset and ROM select
//   D0-D15  A1-A20                       data and address bus
//   DO1  DO2  BCK  LRCK  DACCK           serial audio from the unit, with its clocks,
//                                        as the HD-AE5000 provides on the KN5000
//   +15M  +15A  -15A  +5A  +5D  +3.3D  E supplies and ground
//
// Modelled here: the slot, and a hook for the fitted unit to map itself into the
// host's program space.

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

	// let the fitted unit decode its own window in the host program space
	void program_map(address_space_installer &space);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};

class device_kn6000_expansion_interface : public device_interface
{
public:
	virtual void program_map(address_space_installer &space) = 0;

protected:
	device_kn6000_expansion_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(KN6000_EXPANSION, kn6000_expansion_connector)

void kn6000_expansion_intf(device_slot_interface &device);

#endif // MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
