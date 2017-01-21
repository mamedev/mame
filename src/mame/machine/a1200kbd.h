// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga 1200 Keyboard

	Skeleton device, needs MC68HC05Cx device support

***************************************************************************/

#pragma once

#ifndef MAME_MACHINE_A1200KBD_H
#define MAME_MACHINE_A1200KBD_H

#include "emu.h"
#include "cpu/m6805/m6805.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_A1200_KEYBOARD_KCLK_CALLBACK(_write) \
	devcb = &a1200kbd_device::set_kclk_wr_callback(*device, DEVCB_##_write);

#define MCFG_A1200_KEYBOARD_KDAT_CALLBACK(_write) \
	devcb = &a1200kbd_device::set_kdat_wr_callback(*device, DEVCB_##_write);

#define MCFG_A1200_KEYBOARD_KRST_CALLBACK(_write) \
	devcb = &a1200kbd_device::set_krst_wr_callback(*device, DEVCB_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a1200kbd_device

class a1200kbd_device : public device_t
{
public:
	// construction/destruction
	a1200kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_kclk_wr_callback(device_t &device, _Object object)
		{ return downcast<a1200kbd_device &>(device).m_write_kclk.set_callback(object); }

	template<class _Object> static devcb_base &set_kdat_wr_callback(device_t &device, _Object object)
		{ return downcast<a1200kbd_device &>(device).m_write_kdat.set_callback(object); }

	template<class _Object> static devcb_base &set_krst_wr_callback(device_t &device, _Object object)
		{ return downcast<a1200kbd_device &>(device).m_write_krst.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( kdat_w );

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_write_kclk;
	devcb_write_line m_write_kdat;
	devcb_write_line m_write_krst;

	required_device<m6805_device> m_mpu;
};

// device type definition
extern const device_type A1200KBD;

#endif // MAME_MACHINE_A1200KBD_H
