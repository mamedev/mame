// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  oric.h - Oric 1/Atmos extension port

***************************************************************************/

#ifndef MAME_BUS_ORICEXT_ORICEXT_H
#define MAME_BUS_ORICEXT_ORICEXT_H

#pragma once

#include "cpu/m6502/m6502.h"

class device_oricext_interface;

class oricext_connector: public device_t, public device_single_card_slot_interface<device_oricext_interface>
{
	friend class device_oricext_interface;

public:
	template <typename T>
	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: oricext_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oricext_connector();

	auto irq_callback() { return irq_handler.bind(); }
	auto reset_callback() { return reset_handler.bind(); }

	void irq_w(int state);
	void reset_w(int state);

	void set_view(memory_view &view);
	void map_io(address_space_installer &space);
	void map_rom();

protected:
	virtual void device_start() override ATTR_COLD;

	devcb_write_line irq_handler;
	devcb_write_line reset_handler;
};

class device_oricext_interface : public device_interface
{
public:
	void irq_w(int state);
	void reset_w(int state);

	void set_view(memory_view &view);
	virtual void map_io(address_space_installer &space) = 0;
	virtual void map_rom() = 0;

protected:
	device_oricext_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	memory_view *view;
	oricext_connector *connector;
};

DECLARE_DEVICE_TYPE(ORICEXT_CONNECTOR, oricext_connector)

void oricext_intf(device_slot_interface &device);

#endif // MAME_BUS_ORICEXT_ORICEXT_H
