// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System controller port
    emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_NES_CTRL_CTRL_H
#define MAME_BUS_NES_CTRL_CTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nes_control_port_device;

// ======================> device_nes_control_port_interface

class device_nes_control_port_interface : public device_slot_card_interface
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


typedef device_delegate<bool (int x, int y)> nesctrl_brightpixel_delegate;
#define NESCTRL_BRIGHTPIXEL_CB(name)  bool name(int x, int y)


// ======================> nes_control_port_device

class nes_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	nes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~nes_control_port_device();

	template <typename Object> void set_brightpixel_callback(Object &&cb) { m_brightpixel_cb = std::forward<Object>(cb); }

	uint8_t read_bit0();
	uint8_t read_bit34();
	uint8_t read_exp(offs_t offset);
	void write(uint8_t data);

	nesctrl_brightpixel_delegate m_brightpixel_cb;

protected:
	// device-level overrides
	virtual void device_start() override;
	device_nes_control_port_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_CONTROL_PORT, nes_control_port_device)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NES_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NES_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

// currently this is emulated as a control port...
#define MCFG_FC_EXPANSION_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, NES_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_NESCTRL_BRIGHTPIXEL_CB(_class, _method) \
	downcast<nes_control_port_device &>(*device).set_brightpixel_callback(nesctrl_brightpixel_delegate(&_class::_method, #_class "::" #_method, this));


void nes_control_port1_devices(device_slot_interface &device);
void nes_control_port2_devices(device_slot_interface &device);
void fc_control_port1_devices(device_slot_interface &device);
void fc_control_port2_devices(device_slot_interface &device);
void fc_expansion_devices(device_slot_interface &device);

void majesco_control_port1_devices(device_slot_interface &device);
void majesco_control_port2_devices(device_slot_interface &device);

#endif // MAME_BUS_NES_CTRL_CTRL_H
