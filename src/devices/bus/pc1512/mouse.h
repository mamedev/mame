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

#ifndef MAME_BUS_PC1512_MOUSE_H
#define MAME_BUS_PC1512_MOUSE_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pc1512_mouse_port_device;


// ======================> device_pc1512_mouse_port_interface

class device_pc1512_mouse_port_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_pc1512_mouse_port_interface() { }

protected:
	device_pc1512_mouse_port_interface(const machine_config &mconfig, device_t &device);

	pc1512_mouse_port_device *m_port;
};


// ======================> pc1512_mouse_port_device

class pc1512_mouse_port_device : public device_t, public device_single_card_slot_interface<device_pc1512_mouse_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	pc1512_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pc1512_mouse_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pc1512_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// static configuration helpers
	auto x_wr_callback() { return m_write_x.bind(); }
	auto y_wr_callback() { return m_write_y.bind(); }
	auto m1_wr_callback() { return m_write_m1.bind(); }
	auto m2_wr_callback() { return m_write_m2.bind(); }

	// peripheral interface
	void x_w(uint8_t data) { m_write_x(data); }
	void y_w(uint8_t data) { m_write_y(data); }
	void m1_w(int state) { m_write_m1(state); }
	void m2_w(int state) { m_write_m2(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write8 m_write_x;
	devcb_write8 m_write_y;
	devcb_write_line m_write_m1;
	devcb_write_line m_write_m2;

	device_pc1512_mouse_port_interface *m_device;
};


// ======================> pc1512_mouse_device

class pc1512_mouse_device : public device_t, public device_pc1512_mouse_port_interface
{
public:
	// construction/destruction
	pc1512_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER( mouse_x_changed ) { m_port->x_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_y_changed ) { m_port->y_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_1_changed ) { m_port->m1_w(newval); }
	DECLARE_INPUT_CHANGED_MEMBER( mouse_button_2_changed ) { m_port->m2_w(newval); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(PC1512_MOUSE_PORT, pc1512_mouse_port_device)
DECLARE_DEVICE_TYPE(PC1512_MOUSE,      pc1512_mouse_device)


// slot devices
void pc1512_mouse_port_devices(device_slot_interface &device);

#endif // MAME_BUS_PC1512_MOUSE_H
