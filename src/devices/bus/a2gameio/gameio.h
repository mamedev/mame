// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II Game I/O Connector

*********************************************************************/

#ifndef MAME_BUS_A2GAMEIO_GAMEIO_H
#define MAME_BUS_A2GAMEIO_GAMEIO_H

#pragma once

#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class device_a2gameio_interface;

// ======================> apple2_gameio_device

class apple2_gameio_device : public device_t, public device_single_card_slot_interface<device_a2gameio_interface>
{
public:
	// construction/destruction
	apple2_gameio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T>
	apple2_gameio_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: apple2_gameio_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	template <typename T, typename U>
	apple2_gameio_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&screen_tag, U &&opts, const char *dflt)
		: apple2_gameio_device(mconfig, tag, owner, opts, dflt)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	// configuration
	void set_sw_pullups(bool enabled) { m_sw_pullups = enabled; }
	bool has_sw_pullups() const { return m_sw_pullups; }

	// standard options
	static void default_options(device_slot_interface &slot);
	static void iiandplus_options(device_slot_interface &slot);
	static void joystick_options(device_slot_interface &slot);

	// analog paddles
	u8 pdl0_r();
	u8 pdl1_r();
	u8 pdl2_r();
	u8 pdl3_r();

	// digital switches
	int sw0_r();
	int sw1_r();
	int sw2_r();
	int sw3_r();

	// annunciator outputs
	void an0_w(int state);
	void an1_w(int state);
	void an2_w(int state);
	void an3_w(int state);
	void an4_w(int state);

	// utility strobe (active low)
	void strobe_w(int state);

	// check if a device is connected
	bool is_device_connected() { return (m_intf != nullptr); }

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	optional_device<screen_device> m_screen;

private:
	// selected device
	device_a2gameio_interface *m_intf;

	bool m_sw_pullups;
};

// ======================> device_a2gameio_interface

class device_a2gameio_interface : public device_interface
{
	friend class apple2_gameio_device;

public:
	virtual ~device_a2gameio_interface();

protected:
	// construction/destruction
	device_a2gameio_interface(const machine_config &mconfig, device_t &device);

	// optional input overrides
	virtual u8 pdl0_r() { return 0; }
	virtual u8 pdl1_r() { return 0; }
	virtual u8 pdl2_r() { return 0; }
	virtual u8 pdl3_r() { return 0; }
	virtual int sw0_r() { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual int sw1_r() { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual int sw2_r() { return m_connector->has_sw_pullups() ? 1 : 0; }
	virtual int sw3_r() { return m_connector->has_sw_pullups() ? 1 : 0; }

	// optional output overrides
	virtual void an0_w(int state) { }
	virtual void an1_w(int state) { }
	virtual void an2_w(int state) { }
	virtual void an3_w(int state) { }
	virtual void an4_w(int state) { }
	virtual void strobe_w(int state) { }

	void set_screen(screen_device *screen) { m_screen = screen; }
	screen_device *m_screen;
private:
	apple2_gameio_device *m_connector;
};

// device type declaration
DECLARE_DEVICE_TYPE(APPLE2_GAMEIO, apple2_gameio_device)

#endif // MAME_BUS_A2GAMEIO_GAMEIO_H
