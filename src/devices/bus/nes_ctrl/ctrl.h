// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System controller port
    emulation

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __NES_CONTROL_PORT__
#define __NES_CONTROL_PORT__

#include "emu.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nes_control_port_device;

// ======================> device_nes_control_port_interface

class device_nes_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_nes_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nes_control_port_interface();

	virtual UINT8 read_bit0() { return 0; };
	virtual UINT8 read_bit34() { return 0; };
	virtual UINT8 read_exp(offs_t offset) { return 0; };
	virtual void write(UINT8 data) { };

protected:
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
	nes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nes_control_port_device();

	static void set_brightpixel_callback(device_t &device, nesctrl_brightpixel_delegate callback) { downcast<nes_control_port_device &>(device).m_brightpixel_cb = callback; }

	UINT8 read_bit0();
	UINT8 read_bit34();
	UINT8 read_exp(offs_t offset);
	void write(UINT8 data);

	nesctrl_brightpixel_delegate m_brightpixel_cb;

protected:
	// device-level overrides
	virtual void device_start() override;
	device_nes_control_port_interface *m_device;
};


// device type definition
extern const device_type NES_CONTROL_PORT;


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
	nes_control_port_device::set_brightpixel_callback(*device, nesctrl_brightpixel_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


SLOT_INTERFACE_EXTERN( nes_control_port1_devices );
SLOT_INTERFACE_EXTERN( nes_control_port2_devices );
SLOT_INTERFACE_EXTERN( fc_control_port1_devices );
SLOT_INTERFACE_EXTERN( fc_control_port2_devices );
SLOT_INTERFACE_EXTERN( fc_expansion_devices );


#endif
