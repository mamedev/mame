// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_MOUSE_MOUSE_H
#define MAME_BUS_INTERPRO_MOUSE_MOUSE_H

#pragma once

class device_interpro_mouse_port_interface;

class interpro_mouse_port_device : public device_t, public device_single_card_slot_interface<device_interpro_mouse_port_interface>
{
	friend class device_interpro_mouse_port_interface;

public:
	template <typename T>
	interpro_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: interpro_mouse_port_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	interpro_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callback configuration
	auto state_func() { return m_state_func.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	devcb_write32 m_state_func;

	device_interpro_mouse_port_interface *m_device;
};

class device_interpro_mouse_port_interface : public device_interface
{
	friend class interpro_mouse_port_device;

protected:
	device_interpro_mouse_port_interface(machine_config const &mconfig, device_t &device);

	void state_w(u32 data, u32 mem_mask) { m_port->m_state_func(offs_t(0), data, mem_mask); }

private:
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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(INTERPRO_MOUSE_PORT, interpro_mouse_port_device)
DECLARE_DEVICE_TYPE(INTERPRO_MOUSE, interpro_mouse_device)

void interpro_mouse_devices(device_slot_interface &device);

#endif // MAME_BUS_INTERPRO_MOUSE_MOUSE_H
