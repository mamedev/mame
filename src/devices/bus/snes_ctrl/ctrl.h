// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Super Famicom & SNES controller port emulation

**********************************************************************/


#pragma once

#ifndef __SNES_CONTROL_PORT__
#define __SNES_CONTROL_PORT__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class snes_control_port_device;

// ======================> device_snes_control_port_interface

class device_snes_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_snes_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_snes_control_port_interface();

	virtual uint8_t read_pin4() { return 0; };
	virtual uint8_t read_pin5() { return 0; };
	virtual void write_pin6(uint8_t data) { };
	virtual void write_strobe(uint8_t data) { };
	virtual void port_poll() { };

protected:
	snes_control_port_device *m_port;
};

typedef device_delegate<bool (int16_t x, int16_t y)> snesctrl_onscreen_delegate;
#define SNESCTRL_ONSCREEN_CB(name)  bool name(int16_t x, int16_t y)

typedef device_delegate<void (int16_t x, int16_t y)> snesctrl_gunlatch_delegate;
#define SNESCTRL_GUNLATCH_CB(name)  void name(int16_t x, int16_t y)

// ======================> snes_control_port_device

class snes_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	snes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~snes_control_port_device();

	static void set_onscreen_callback(device_t &device, snesctrl_onscreen_delegate callback) { downcast<snes_control_port_device &>(device).m_onscreen_cb = callback; }
	static void set_gunlatch_callback(device_t &device, snesctrl_gunlatch_delegate callback) { downcast<snes_control_port_device &>(device).m_gunlatch_cb = callback; }

	uint8_t read_pin4();
	uint8_t read_pin5();
	void write_pin6(uint8_t data);
	void write_strobe(uint8_t data);
	void port_poll();

	snesctrl_onscreen_delegate m_onscreen_cb;
	snesctrl_gunlatch_delegate m_gunlatch_cb;

protected:
	// device-level overrides
	virtual void device_start() override;

	device_snes_control_port_interface *m_device;
};


// device type definition
extern const device_type SNES_CONTROL_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SNES_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, SNES_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_SNESCTRL_ONSCREEN_CB(_class, _method) \
	snes_control_port_device::set_onscreen_callback(*device, snesctrl_onscreen_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SNESCTRL_GUNLATCH_CB(_class, _method) \
	snes_control_port_device::set_gunlatch_callback(*device, snesctrl_gunlatch_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


SLOT_INTERFACE_EXTERN( snes_control_port_devices );


#endif
