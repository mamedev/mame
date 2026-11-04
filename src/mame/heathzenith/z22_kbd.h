// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Zenith Z-22 keyboard (high level emulation)

***************************************************************************/

#ifndef MAME_HEATHZENITH_Z22_KBD_H
#define MAME_HEATHZENITH_Z22_KBD_H

#pragma once

#include "diserial.h"
#include "machine/keyboard.h"


class z22_keyboard_device : public device_t,
	public device_buffered_serial_interface<16U>,
	protected device_matrix_keyboard_interface<7U>
{
public:
	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

	z22_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto txd_handler() { return m_txd_cb.bind(); }

	void rxd_w(int state) { device_buffered_serial_interface::rx_w(state); }

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_buffered_serial_interface implementation
	virtual void tra_callback() override;
	virtual void received_byte(u8 byte) override;

	// device_matrix_keyboard_interface implementation
	virtual void key_make(u8 row, u8 column) override;
	virtual void key_repeat(u8 row, u8 column) override;
	virtual void key_break(u8 row, u8 column) override;

private:
	u8 key_code(u8 row, u8 column) const;

	devcb_write_line m_txd_cb;

	bool m_shift_left;
	bool m_shift_right;
	bool m_caps;
	bool m_command;
};

DECLARE_DEVICE_TYPE(Z22_KEYBOARD, z22_keyboard_device)

#endif // MAME_HEATHZENITH_Z22_KBD_H
