// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_MOUSE_MOUSE_H
#define MAME_BUS_INTERPRO_MOUSE_MOUSE_H

#pragma once

#define MCFG_MOUSE_STATE_CB(_state_cb) \
	devcb = &downcast<interpro_mouse_port_device &>(*device).set_state_callback(DEVCB_##_state_cb);

class device_interpro_mouse_port_interface;

class interpro_mouse_port_device : public device_t, public device_slot_interface
{
	friend class device_interpro_mouse_port_interface;

public:
	interpro_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_state_callback(Object &&cb) { return m_state_func.set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_config_complete() override;

	devcb_write32 m_state_func;

private:
	device_interpro_mouse_port_interface *m_device;
};

class device_interpro_mouse_port_interface : public device_slot_card_interface
{
	friend class interpro_mouse_port_device;

public:
	DECLARE_WRITE32_MEMBER(state_w) { m_port->m_state_func(space, offset, data, mem_mask); }

protected:
	device_interpro_mouse_port_interface(machine_config const &mconfig, device_t &device);

	interpro_mouse_port_device *m_port;
};

class interpro_mouse_device : public device_t, public device_interpro_mouse_port_interface
{
public:
	enum state_mask
	{
		MOUSE_YPOS    = 0x000000ff,
		MOUSE_XPOS    = 0x0000ff00,
		MOUSE_LBUTTON = 0x00010000,
		MOUSE_MBUTTON = 0x00020000,
		MOUSE_RBUTTON = 0x00040000,

		MOUSE_BUTTONS = 0x00070000
	};

	// constructor/destructor
	interpro_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(mouse_button);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_x);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y);

protected:
	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;

private:
};

DECLARE_DEVICE_TYPE(INTERPRO_MOUSE_PORT, interpro_mouse_port_device)
DECLARE_DEVICE_TYPE(INTERPRO_MOUSE, interpro_mouse_device)

void interpro_mouse_devices(device_slot_interface &device);

#endif // MAME_BUS_INTERPRO_MOUSE_MOUSE_H
