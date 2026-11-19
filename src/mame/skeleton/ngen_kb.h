// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef MAME_SKELETON_NGEN_KB_H
#define MAME_SKELETON_NGEN_KB_H

#pragma once

#include "machine/keyboard.h"
#include "diserial.h"

class ngen_keyboard_device :
		public device_t,
		public device_buffered_serial_interface<16U>,
		protected device_matrix_keyboard_interface<4U>
{
public:
	ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	auto tx_handler() { return m_tx_handler.bind(); }
	void rx_w(int state) { device_buffered_serial_interface::rx_w(state); }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void tra_callback() override;
	virtual void received_byte(uint8_t byte) override;
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;

private:
	bool translate(uint8_t row, uint8_t column, uint8_t &data) const;
	attotime typematic_delay() const;
	attotime typematic_period() const;

	devcb_write_line m_tx_handler;
	required_ioport m_config;
	required_ioport m_modifiers;
	uint8_t m_keys_down;
};

DECLARE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device)

#endif // MAME_SKELETON_NGEN_KB_H
