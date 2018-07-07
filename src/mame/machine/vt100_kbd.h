// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu, AJR
/***************************************************************************

        DEC VT100 keyboard emulation

***************************************************************************/

#ifndef MAME_MACHINE_VT100_KBD_H
#define MAME_MACHINE_VT100_KBD_H

#pragma once

#include "machine/ay31015.h"
#include "machine/ripple_counter.h"
#include "sound/beep.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vt100_keyboard_device

class vt100_keyboard_device : public device_t
{
public:
	// construction/destruction
	vt100_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto signal_out_callback() { return m_signal_out_cb.bind(); }

	DECLARE_WRITE_LINE_MEMBER(signal_line_w);

protected:
	vt100_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual bool scan_enabled() const { return m_uart->tbmt_r(); }
	virtual void scan_start() { }

private:
	// internal helpers
	DECLARE_WRITE_LINE_MEMBER(signal_out_w);
	DECLARE_WRITE_LINE_MEMBER(scan_disable_w);
	DECLARE_WRITE8_MEMBER(key_scan_w);

	devcb_write_line m_signal_out_cb;

	required_device<ay31015_device> m_uart;
	required_device<beep_device> m_speaker;
	required_device<ripple_counter_device> m_scan_counter;
	required_ioport_array<16> m_key_row;

	output_finder<> m_online_led;
	output_finder<> m_local_led;
	output_finder<> m_locked_led;
	output_finder<4> m_ln_led;

	bool m_signal_line;
	attotime m_last_signal_change;
	u8 m_last_scan;
};

// ======================> ms7002_device

class ms7002_device : public vt100_keyboard_device
{
public:
	// construction/destruction
	ms7002_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual bool scan_enabled() const override { return m_scan_enable; }
	virtual void scan_start() override { m_scan_enable = true; }

private:
	DECLARE_WRITE_LINE_MEMBER(scan_disable_w);

	bool m_scan_enable;
};

// device type definitions
DECLARE_DEVICE_TYPE(VT100_KEYBOARD, vt100_keyboard_device)
DECLARE_DEVICE_TYPE(MS7002, ms7002_device)

#endif // MAME_MACHINE_VT100_KBD_H
