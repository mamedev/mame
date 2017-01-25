// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

	Amiga 1200 Keyboard

	Skeleton device, needs MC68HC05Cx device support

***************************************************************************/

#ifndef DEVICES_BUS_AMIGA_KEYBOARD_A1200_H
#define DEVICES_BUS_AMIGA_KEYBOARD_A1200_H

#pragma once

#include "emu.h"
#include "keyboard.h"
#include "cpu/m6805/m6805.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a1200_kbd_device

class a1200_kbd_device : public device_t, public device_amiga_keyboard_interface
{
public:
	// construction/destruction
	a1200_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual DECLARE_WRITE_LINE_MEMBER(kdat_w) override;

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<m6805_device> m_mpu;
};

// device type definition
extern const device_type A1200_KBD;

#endif // DEVICES_BUS_AMIGA_KEYBOARD_A1200_H
