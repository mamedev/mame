// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari 7800 controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_A7800_CTRL_CTRL_H
#define MAME_BUS_A7800_CTRL_CTRL_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A7800_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, A7800_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_A7800_CONTROL_PORT_TRIGGER_CALLBACK(_write) \
	devcb = &a7800_control_port_device::set_trigger_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a7800_control_port_device;


// ======================> device_a7800_control_port_interface

class device_a7800_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_a7800_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a7800_control_port_interface() { }

	virtual uint8_t a7800_joy_r() { return 0xff; };
	virtual uint8_t a7800_pot_x_r() { return 0xff; };
	virtual uint8_t a7800_pot_y_r() { return 0xff; };
	virtual uint8_t a7800_light_x_r() { return 0xff; };
	virtual uint8_t a7800_light_y_r() { return 0xff; };
	virtual void a7800_joy_w(uint8_t data) { };

	virtual bool has_pot_x() { return false; }
	virtual bool has_pot_y() { return false; }
	virtual bool is_paddle() { return false; }
	virtual bool is_lightgun() { return false; }
	virtual bool has_pro_buttons() { return false; }

protected:
	a7800_control_port_device *m_port;
};


// ======================> a7800_control_port_device

class a7800_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	a7800_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_trigger_wr_callback(device_t &device, Object &&cb) { return downcast<a7800_control_port_device &>(device).m_write_trigger.set_callback(std::forward<Object>(cb)); }

	// computer interface

	// Data returned by the joy_r methods:
	// bit 0 0x01 - pin 1 - Up
	// bit 1 0x02 - pin 2 - Down
	// bit 2 0x04 - pin 3 - Left             
	// bit 3 0x08 - pin 4 - Right
	// bit 4 0x10 - pin 5 - Proline A button
	// bit 5 0x20 - pin 6 - VCS button
	//              pin 7 - +5V
	//              pin 8 - GND
	// bit 6 0x40 - pin 9 - Proline B button
	//
	uint8_t joy_r() { uint8_t data = 0xff; if (exists()) data = m_device->a7800_joy_r(); return data; }
	DECLARE_READ8_MEMBER( joy_r ) { return joy_r(); }
	uint8_t pot_x_r() { uint8_t data = 0xff; if (exists()) data = m_device->a7800_pot_x_r(); return data; }
	DECLARE_READ8_MEMBER( pot_x_r ) { return pot_x_r(); }
	uint8_t pot_y_r() { uint8_t data = 0xff; if (exists()) data = m_device->a7800_pot_y_r(); return data; }
	DECLARE_READ8_MEMBER( pot_y_r ) { return pot_y_r(); }
	uint8_t light_x_r() { uint16_t data = 0xff; if (exists()) data = m_device->a7800_light_x_r(); return data; }
	DECLARE_READ8_MEMBER( light_x_r ) { return light_x_r(); }
	uint8_t light_y_r() { uint16_t data = 0xff; if (exists()) data = m_device->a7800_light_y_r(); return data; }
	DECLARE_READ8_MEMBER( light_y_r ) { return light_y_r(); }

	void joy_w( uint8_t data ) { if ( exists() ) m_device->a7800_joy_w( data ); }

	bool exists() { return m_device != nullptr; }
	bool has_pot_x() { return exists() && m_device->has_pot_x(); }
	bool has_pot_y() { return exists() && m_device->has_pot_y(); }
	bool is_paddle() { return exists() && m_device->is_paddle(); }
	bool is_lightgun() { return exists() && m_device->is_lightgun(); }
	bool has_pro_buttons() { return exists() && m_device->has_pro_buttons(); }

	void trigger_w(int state) { m_write_trigger(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_a7800_control_port_interface *m_device;

private:
	devcb_write_line m_write_trigger;
};


// device type definition
DECLARE_DEVICE_TYPE(A7800_CONTROL_PORT, a7800_control_port_device)

SLOT_INTERFACE_EXTERN( a7800_control_port_devices );

#endif // MAME_BUS_A7800_CTRL_CTRL_H
