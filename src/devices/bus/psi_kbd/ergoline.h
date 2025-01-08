// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Kontron Ergo Line Keyboard

    Sold with the PSI908/9C/98/980 systems

***************************************************************************/

#ifndef MAME_BUS_PSI_KBD_ERGOLINE_H
#define MAME_BUS_PSI_KBD_ERGOLINE_H

#pragma once

#include "psi_kbd.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ergoline_keyboard_device

class ergoline_keyboard_device : public device_t, public device_psi_keyboard_interface
{
public:
	// construction/destruction
	ergoline_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::KEYBOARD; }

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// from host
	virtual void tx_w(int state) override;

private:
	void kbd_io(address_map &map) ATTR_COLD;
	void kbd_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ERGOLINE_KEYBOARD, ergoline_keyboard_device)


#endif // MAME_BUS_PSI_KBD_ERGOLINE_H
