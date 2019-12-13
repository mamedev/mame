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
public:
	template <typename T>
	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, const char *cputag)
		: oricext_connector(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_cputag(cputag);
	}

	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oricext_connector();

	void set_cputag(const char *tag);
	auto irq_callback() { return irq_handler.bind(); }
	void irq_w(int state);

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	devcb_write_line irq_handler;
	const char *cputag;
};

class device_oricext_interface : public device_interface
{
public:
	void set_cputag(const char *tag);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

protected:
	device_oricext_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	const char *cputag;
	m6502_device *cpu;
	oricext_connector *connector;
	memory_bank *bank_c000_r, *bank_e000_r, *bank_f800_r, *bank_c000_w, *bank_e000_w, *bank_f800_w;
	uint8_t *rom, *ram;
	uint8_t junk_read[8192], junk_write[8192];
};

DECLARE_DEVICE_TYPE(ORICEXT_CONNECTOR, oricext_connector)

void oricext_intf(device_slot_interface &device);

#endif // MAME_BUS_ORICEXT_ORICEXT_H
