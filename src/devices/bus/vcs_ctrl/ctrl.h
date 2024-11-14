// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_CVS_CTRL_CTRL_H
#define MAME_BUS_CVS_CTRL_CTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class vcs_control_port_device;


// ======================> device_vcs_control_port_interface

class device_vcs_control_port_interface : public device_interface
{
public:
	virtual ~device_vcs_control_port_interface() { }

	virtual uint8_t vcs_joy_r() { return 0xff; }
	virtual uint8_t vcs_pot_x_r() { return 0xff; }
	virtual uint8_t vcs_pot_y_r() { return 0xff; }
	virtual void vcs_joy_w(uint8_t data) { }

	virtual bool has_pot_x() { return false; }
	virtual bool has_pot_y() { return false; }

	// FIXME: should be made protected when port definitions become member functions
	inline void trigger_w(int state);

protected:
	device_vcs_control_port_interface(const machine_config &mconfig, device_t &device);

	vcs_control_port_device *m_port;
};


// ======================> vcs_control_port_device

class vcs_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const* dflt)
		: vcs_control_port_device(mconfig, tag, owner)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// static configuration helpers
	auto trigger_wr_callback() { return m_write_trigger.bind(); }

	// computer interface

	// Data returned by the joy_r methods:
	// bit 0 - pin 1 - Up
	// bit 1 - pin 2 - Down
	// bit 2 - pin 3 - Left
	// bit 3 - pin 4 - Right
	//         pin 5 - Pot X
	// bit 5 - pin 6 - Button
	//         pin 7 - +5V
	//         pin 8 - GND
	//         pin 9 - Pot Y
	//
	uint8_t read_joy() { return exists() ? m_device->vcs_joy_r() : 0xff; }
	uint8_t read_pot_x() { return exists() ? m_device->vcs_pot_x_r() : 0xff; }
	uint8_t read_pot_y() { return exists() ? m_device->vcs_pot_y_r() : 0xff; }

	void joy_w(uint8_t data) { if (exists()) m_device->vcs_joy_w(data); }

	bool exists() { return m_device != nullptr; }
	bool has_pot_x() { return exists() && m_device->has_pot_x(); }
	bool has_pot_y() { return exists() && m_device->has_pot_y(); }

	void trigger_w(int state) { m_write_trigger(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_vcs_control_port_interface *m_device;

private:
	devcb_write_line m_write_trigger;
};

inline void device_vcs_control_port_interface::trigger_w(int state)
{
	m_port->trigger_w(state);
}


// device type definition
DECLARE_DEVICE_TYPE(VCS_CONTROL_PORT, vcs_control_port_device)

void vcs_control_port_devices(device_slot_interface &device);
void a800_control_port_devices(device_slot_interface &device);

#endif // MAME_BUS_CVS_CTRL_CTRL_H
