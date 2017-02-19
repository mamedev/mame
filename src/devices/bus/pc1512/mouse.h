// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1512 mouse port emulation

**********************************************************************

                            1   XA
                            2   XB
                            3   YA
                            4   YB
                            5
                            6   M1
                            7   +5V
                            8   GND
                            9   M2

**********************************************************************/

#pragma once

#ifndef __PC1512_MOUSE_PORT__
#define __PC1512_MOUSE_PORT__




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define PC1512_MOUSE_PORT_TAG "mous"

#define MCFG_PC1512_MOUSE_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PC1512_MOUSE_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_PC1512_MOUSE_PORT_X_CB(_write) \
	devcb = &pc1512_mouse_port_t::set_x_wr_callback(*device, DEVCB_##_write);

#define MCFG_PC1512_MOUSE_PORT_Y_CB(_write) \
	devcb = &pc1512_mouse_port_t::set_y_wr_callback(*device, DEVCB_##_write);

#define MCFG_PC1512_MOUSE_PORT_M1_CB(_write) \
	devcb = &pc1512_mouse_port_t::set_m1_wr_callback(*device, DEVCB_##_write);

#define MCFG_PC1512_MOUSE_PORT_M2_CB(_write) \
	devcb = &pc1512_mouse_port_t::set_m2_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc1512_mouse_port_t;


// ======================> device_pc1512_mouse_port_interface

class device_pc1512_mouse_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_pc1512_mouse_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pc1512_mouse_port_interface() { }

protected:
	pc1512_mouse_port_t *m_port;
};


// ======================> pc1512_mouse_port_t

class pc1512_mouse_port_t : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	pc1512_mouse_port_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_x_wr_callback(device_t &device, _Object object) { return downcast<pc1512_mouse_port_t &>(device).m_write_x.set_callback(object); }
	template<class _Object> static devcb_base &set_y_wr_callback(device_t &device, _Object object) { return downcast<pc1512_mouse_port_t &>(device).m_write_y.set_callback(object); }
	template<class _Object> static devcb_base &set_m1_wr_callback(device_t &device, _Object object) { return downcast<pc1512_mouse_port_t &>(device).m_write_m1.set_callback(object); }
	template<class _Object> static devcb_base &set_m2_wr_callback(device_t &device, _Object object) { return downcast<pc1512_mouse_port_t &>(device).m_write_m2.set_callback(object); }

	// peripheral interface
	void x_w(uint8_t data) { m_write_x(data); }
	void y_w(uint8_t data) { m_write_y(data); }
	void m1_w(int state) { m_write_m1(state); }
	void m2_w(int state) { m_write_m2(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

	devcb_write8 m_write_x;
	devcb_write8 m_write_y;
	devcb_write_line m_write_m1;
	devcb_write_line m_write_m2;

	device_pc1512_mouse_port_interface *m_device;
};


// ======================> pc1512_mouse_t

class pc1512_mouse_t : public device_t,
					   public device_pc1512_mouse_port_interface
{
public:
	// construction/destruction
	pc1512_mouse_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed ) { m_port->x_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed ) { m_port->y_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_1_changed ) { m_port->m1_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_2_changed ) { m_port->m2_w(newval); }

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type PC1512_MOUSE_PORT;
extern const device_type PC1512_MOUSE;


// slot devices
SLOT_INTERFACE_EXTERN( pc1512_mouse_port_devices );

#endif
