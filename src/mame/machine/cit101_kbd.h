// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    CIT-101 85-key keyboard

**********************************************************************/

#ifndef MAME_MACHINE_CIT101_KBD_H
#define MAME_MACHINE_CIT101_KBD_H

#pragma once

#include "machine/keyboard.h"
#include "sound/beep.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cit101_hle_keyboard_device

class cit101_hle_keyboard_device : public device_t, public device_matrix_keyboard_interface<4U>, public device_buffered_serial_interface<16U>
{
public:
	// construction/destruction
	cit101_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration
	auto txd_callback() { return m_txd_callback.bind(); }

	DECLARE_WRITE_LINE_MEMBER(write_rxd);

protected:
	// device_t overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void received_byte(u8 byte) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;
	virtual void scan_complete() override;

	void send_translated(u8 row, u8 column);
	void send_key(u8 code);
private:
	required_ioport m_modifiers;
	required_device<beep_device> m_beeper;
	devcb_write_line m_txd_callback;
	u8 m_command[2];
};

// device type definition
DECLARE_DEVICE_TYPE(CIT101_HLE_KEYBOARD, cit101_hle_keyboard_device)

#endif
