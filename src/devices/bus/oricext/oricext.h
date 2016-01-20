// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

  oric.h - Oric 1/Atmos extension port

***************************************************************************/

#ifndef __ORICEXT_H__
#define __ORICEXT_H__

#include "emu.h"
#include "cpu/m6502/m6502.h"

#define MCFG_ORICEXT_ADD(_tag, _slot_intf, _def_slot, _cputag, _irq)    \
	MCFG_DEVICE_ADD(_tag, ORICEXT_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	downcast<oricext_connector *>(device)->set_cputag(_cputag); \
	devcb = &oricext_connector::set_irq_handler(*device, DEVCB_##_irq);


class oricext_device;

class oricext_connector: public device_t,
							public device_slot_interface
{
public:
	oricext_connector(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~oricext_connector();

	void set_cputag(const char *tag);
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<oricext_connector &>(device).irq_handler.set_callback(object); }
	void irq_w(int state);

protected:
	devcb_write_line irq_handler;
	const char *cputag;
	virtual void device_start() override;
	virtual void device_config_complete() override;
};

class oricext_device : public device_t,
						public device_slot_card_interface
{
public:
	oricext_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void set_cputag(const char *tag);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

protected:
	const char *cputag;
	m6502_device *cpu;
	oricext_connector *connector;
	memory_bank *bank_c000_r, *bank_e000_r, *bank_f800_r, *bank_c000_w, *bank_e000_w, *bank_f800_w;
	UINT8 *rom, *ram;
	UINT8 junk_read[8192], junk_write[8192];

	virtual void device_start() override;
};

extern const device_type ORICEXT_CONNECTOR;
SLOT_INTERFACE_EXTERN( oricext_intf );

#endif  /* __ORICEXT_H__ */
