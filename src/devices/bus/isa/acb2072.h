// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec ACB-2072 RLL Drive Controller

***************************************************************************/

#ifndef MAME_BUS_ISA_ACB2072_H
#define MAME_BUS_ISA_ACB2072_H

#pragma once

#include "isa.h"

class acb2072_device : public device_t, public device_isa8_card_interface
{
public:
	acb2072_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cpu_device> m_mcu;
	required_region_ptr<u8> m_bios;
};

// device type declaration
DECLARE_DEVICE_TYPE(ACB2072, acb2072_device)

#endif // MAME_BUS_ISA_ACB2072_H
