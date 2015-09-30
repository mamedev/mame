// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __MACKBD_H__
#define __MACKBD_H__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MACKBD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MACKBD, 0)

#define MCFG_MACKBD_REPLACE(_tag) \
	MCFG_DEVICE_REPLACE(_tag, MACKBD, 0)

#define MCFG_MACKBD_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_MACKBD_CLKOUT_HANDLER(_devcb) \
	devcb = &mackbd_device::set_clkout_handler(*device, DEVCB_##_devcb);

#define MCFG_MACKBD_DATAOUT_HANDLER(_devcb) \
	devcb = &mackbd_device::set_dataout_handler(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mackbd_device

class mackbd_device :  public device_t
{
public:
	// static config helper
	template<class _Object> static devcb_base &set_clkout_handler(device_t &device, _Object object) { return downcast<mackbd_device &>(device).m_clkout_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dataout_handler(device_t &device, _Object object) { return downcast<mackbd_device &>(device).m_dataout_handler.set_callback(object); }

	// construction/destruction
	mackbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(p0_r);
	DECLARE_WRITE8_MEMBER(p0_w);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(t1_r);

	DECLARE_WRITE_LINE_MEMBER(data_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;

	required_device<cpu_device> m_maincpu;

private:
	UINT8 p0, p1, p2, data_from_mac, data_to_mac;

	devcb_write_line m_clkout_handler;
	devcb_write_line m_dataout_handler;

	void scan_kbd_col(int col);
};

// device type definition
extern const device_type MACKBD;

#endif
