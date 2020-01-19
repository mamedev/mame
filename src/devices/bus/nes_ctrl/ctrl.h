// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System controller port
    emulation

**********************************************************************

    Known Issues:
    - Currently the FC expansion port is emulated as a control port

**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_CTRL_H
#define MAME_BUS_NES_CTRL_CTRL_H

#pragma once

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nes_control_port_device;

// ======================> device_nes_control_port_interface

class device_nes_control_port_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_nes_control_port_interface();

	virtual uint8_t read_bit0() { return 0; }
	virtual uint8_t read_bit34() { return 0; }
	virtual uint8_t read_exp(offs_t offset) { return 0; }
	virtual void write(uint8_t data) { }

protected:
	device_nes_control_port_interface(const machine_config &mconfig, device_t &device);

	nes_control_port_device *m_port;
};


// ======================> nes_control_port_device

class nes_control_port_device : public device_t,
								public device_single_card_slot_interface<device_nes_control_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	nes_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: nes_control_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	nes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_control_port_device();

	uint8_t read_bit0();
	uint8_t read_bit34();
	uint8_t read_exp(offs_t offset);
	void write(uint8_t data);
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// for peripherals that interact with the machine's screen
	required_device<screen_device> m_screen;

protected:
	// device-level overrides
	virtual void device_start() override;

	// devices
	device_nes_control_port_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_CONTROL_PORT, nes_control_port_device)

void nes_control_port1_devices(device_slot_interface &device);
void nes_control_port2_devices(device_slot_interface &device);
void fc_control_port1_devices(device_slot_interface &device);
void fc_control_port2_devices(device_slot_interface &device);
void fc_expansion_devices(device_slot_interface &device);
void majesco_control_port1_devices(device_slot_interface &device);
void majesco_control_port2_devices(device_slot_interface &device);


#endif // MAME_BUS_NES_CTRL_CTRL_H
