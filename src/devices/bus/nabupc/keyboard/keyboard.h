// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/***************************************************************************

    NABU PC Keyboard Interface

***************************************************************************/


#ifndef MAME_BUS_NABUPC_KEYBOARD_H
#define MAME_BUS_NABUPC_KEYBOARD_H

#pragma once

#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"

namespace bus::nabupc::keyboard {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class keyboard_device: public device_t, public device_rs232_port_interface
{
public:
	// construction/destruction
	keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual tiny_rom_entry const *device_rom_region() const override;
private:
	void nabu_kb_mem(address_map &map);

	uint8_t port1_r();
	void port1_w(uint8_t data);

	uint8_t gameport_r(offs_t offset);

	required_device<m6803_cpu_device> m_mcu;

	uint8_t m_port1;
	uint8_t m_gameport[4];
};

} // namespace bus::nabupc::keyboard

DECLARE_DEVICE_TYPE_NS(NABUPC_KEYBOARD, bus::nabupc::keyboard, keyboard_device)

#endif // MAME_BUS_NABUPC_KEYBOARD_H
