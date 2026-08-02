// license:GPL2+
// copyright-holders:Felipe Sanches

#ifndef MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
#define MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H

#pragma once

// Expansion connector on the SX-KN6000 and SX-KN6500 main board. The KN6500
// service manual shows it as CN106, a 70-pin connector labelled "TO HDD", and
// the chip-select decoder on the same sheet emits EXP.CS0 and EXP.CS1 for it.
// The signals are not modelled yet; this exists so that an expansion unit's
// firmware can be declared against the device that carries it.

class device_kn6000_expansion_interface;

class kn6000_expansion_connector : public device_t, public device_single_card_slot_interface<device_kn6000_expansion_interface>
{
public:
	template <typename T>
	kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: kn6000_expansion_connector(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
};

class device_kn6000_expansion_interface : public device_interface
{
public:
	virtual ~device_kn6000_expansion_interface() = default;

protected:
	device_kn6000_expansion_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(KN6000_EXPANSION, kn6000_expansion_connector)

void kn6000_expansion_intf(device_slot_interface &device);

#endif // MAME_BUS_TECHNICS_KN6000_KN6000_EXPANSION_H
