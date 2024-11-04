// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Mouse Port

    8-pin connector

    1  K3
    2  K2
    3  K1
    4  K5
    5  K4
    6  MSEINT
    7  RDMSEL
    8  5V

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_MOUSE_MOUSEPORT_H
#define MAME_BUS_SAMCOUPE_MOUSE_MOUSEPORT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_samcoupe_mouse_interface;

// ======================> samcoupe_mouse_port_device

class samcoupe_mouse_port_device : public device_t, public device_single_card_slot_interface<device_samcoupe_mouse_interface>
{
public:
	// construction/destruction
	template <typename T>
	samcoupe_mouse_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts)
		: samcoupe_mouse_port_device(mconfig, tag, owner, uint32_t(0))
	{
		option_reset();
		opts(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	samcoupe_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~samcoupe_mouse_port_device();

	// callbacks
	auto mseint_handler() { return m_mseint_handler.bind(); }

	// called from cart device
	void mseint_w(int state) { m_mseint_handler(state); }

	// called from host
	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_mseint_handler;

	device_samcoupe_mouse_interface *m_module;
};

// ======================> device_samcoupe_mouse_interface

class device_samcoupe_mouse_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_samcoupe_mouse_interface();

	virtual uint8_t read() { return 0xff; }

protected:
	device_samcoupe_mouse_interface(const machine_config &mconfig, device_t &device);

	samcoupe_mouse_port_device *m_port;
};

// device type definition
DECLARE_DEVICE_TYPE(SAMCOUPE_MOUSE_PORT, samcoupe_mouse_port_device)

// include here so drivers don't need to
#include "modules.h"

#endif // MAME_BUS_SAMCOUPE_MOUSE_MOUSEPORT_H
