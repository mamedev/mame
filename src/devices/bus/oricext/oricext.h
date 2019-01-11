// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  oric.h - Oric 1/Atmos extension port

***************************************************************************/

#ifndef MAME_BUS_ORICEXT_ORICEXT_H
#define MAME_BUS_ORICEXT_ORICEXT_H

#pragma once

#include "cpu/m6502/m6502.h"

#define MCFG_ORICEXT_ADD(_tag, _slot_intf, _def_slot, _cputag, _irq)    \
	MCFG_DEVICE_ADD(_tag, ORICEXT_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<oricext_connector *>(device)->set_cputag(_cputag); \
	devcb = &downcast<oricext_connector &>(*device).set_irq_handler(DEVCB_##_irq);


class oricext_device;

class oricext_connector: public device_t, public device_slot_interface
{
public:
	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~oricext_connector();

	void set_cputag(const char *tag);
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return irq_handler.set_callback(std::forward<Object>(cb)); }
	void irq_w(int state);

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	devcb_write_line irq_handler;
	const char *cputag;
};

class oricext_device : public device_t,
						public device_slot_card_interface
{
public:
	void set_cputag(const char *tag);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

protected:
	oricext_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

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
