// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_VCS_CTRL_CTRL_H
#define MAME_BUS_VCS_CTRL_CTRL_H

#pragma once

#include "screen.h"


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

	// true if time_until_lightpen_pos() below can actually time anything for this port
	bool has_lightpen_timing() const;

	// time until the port's video chip's raster position matches a light pen/gun
	// crosshair (0-255 across the visible picture, matching IPT_LIGHTGUN_X/Y's default
	// range); prefers a driver-supplied chip-native timing callback (see
	// vcs_control_port_device::set_lightpen_time_callback), falling back to a generic
	// screen_device-based estimate
	attotime time_until_lightpen_pos(int x255, int y255) const;

	// true if the rendered picture near a light pen/gun crosshair (0-255 across the
	// visible picture, same convention as time_until_lightpen_pos()) is currently
	// bright, weighted by how recently the beam illuminated each point (CRT phosphor
	// persistence rather than the beam's raw current position) - same technique as
	// the NES/Vs./PC-10 zapper photodiode (bus/nes_ctrl/zapper_sensor.h). Real photo-
	// diode-based sensors only fire when actually pointed at something lit, so this
	// is what should gate whether a sensor pulse/latch actually happens, rather than
	// firing unconditionally once per frame regardless of what's on screen.
	bool light_detected(int x255, int y255) const;

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
		set_options(std::forward<T>(opts), dflt, false);
	}
	vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_delegate returning the time until a chip's own raster position matches a
	// light pen crosshair (0-255 across the visible picture, see vcs_lightpen_device);
	// lets a driver hook up a video chip's native raster timing (e.g. mos6566_device::
	// time_until_lightpen_pos) when the generic screen_device-based estimate isn't
	// accurate enough (legacy chip cores whose internal raster counters aren't phase-
	// locked to the screen device's hpos()/vpos())
	using lightpen_time_delegate = device_delegate<attotime (int, int)>;

	// static configuration helpers
	auto trigger_wr_callback() { return m_write_trigger.bind(); }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_lightpen_time_callback(T &&... args) { m_lightpen_time_cb.set(std::forward<T>(args)...); }

	// for peripherals that need to know the beam position (e.g. light pens)
	optional_device<screen_device> m_screen;
	lightpen_time_delegate m_lightpen_time_cb;

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
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_vcs_control_port_interface *m_device;

private:
	devcb_write_line m_write_trigger;
};

inline void device_vcs_control_port_interface::trigger_w(int state)
{
	m_port->trigger_w(state);
}


// device type declaration
DECLARE_DEVICE_TYPE(VCS_CONTROL_PORT, vcs_control_port_device)

void vcs_control_port_devices(device_slot_interface &device);
void a800_control_port_devices(device_slot_interface &device);

#endif // MAME_BUS_VCS_CTRL_CTRL_H
