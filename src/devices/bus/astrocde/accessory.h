// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_ASTROCDE_ACCESSORY_H
#define MAME_BUS_ASTROCDE_ACCESSORY_H

#pragma once

#include "screen.h"


/***************************************************************************
 FORWARD DECLARATIONS
 ***************************************************************************/

class device_astrocade_accessory_interface;


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> astrocade_accessory_port_device

class astrocade_accessory_port_device : public device_t, public device_single_card_slot_interface<device_astrocade_accessory_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	astrocade_accessory_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&screen_tag, U &&opts, char const *dflt)
		: astrocade_accessory_port_device(mconfig, tag, owner, 0U)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	astrocade_accessory_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~astrocade_accessory_port_device();

	auto ltpen_handler() { return m_ltpen_handler.bind(); }

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	int m_ltpen;
	devcb_write_line m_ltpen_handler;
	required_device<screen_device> m_screen;

private:
	device_astrocade_accessory_interface *m_device;

	friend class device_astrocade_accessory_interface;
};


// ======================> device_astrocade_accessory_interface

class device_astrocade_accessory_interface : public device_interface
{
public:
	virtual ~device_astrocade_accessory_interface();

	void write_ltpen(int state) { m_port->m_ltpen = state; m_port->m_ltpen_handler(state); }

protected:
	device_astrocade_accessory_interface(machine_config const &mconfig, device_t &device);

	// device_interface implementation
	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void interface_pre_start() override;

	void set_screen(screen_device *screen) { m_screen = screen; }
	screen_device *m_screen;

private:
	astrocade_accessory_port_device *const m_port;

	friend class astrocade_accessory_port_device;
};


/***************************************************************************
 FUNCTIONS
 ***************************************************************************/

void astrocade_accessories(device_slot_interface &device);


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(ASTROCADE_ACCESSORY_PORT, astrocade_accessory_port_device)

#endif // MAME_BUS_ASTROCDE_ACCESSORY_H
