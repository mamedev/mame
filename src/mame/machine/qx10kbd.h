// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_QX10KBD_H
#define MAME_MACHINE_QX10KBD_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"


class qx10_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<16U>
{
public:
	qx10_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_reset() override;
	virtual void key_make(uint8_t row, uint8_t column) override;

private:
	virtual void received_byte(uint8_t data) override;
};

DECLARE_DEVICE_TYPE(QX10_KEYBOARD, qx10_keyboard_device)

#endif // MAME_MACHINE_QX10KBD_H
