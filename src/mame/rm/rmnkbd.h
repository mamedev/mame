// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_RM_RMNKBD_H
#define MAME_RM_RMNKBD_H

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"

class rmnimbus_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<11U>
{
public:
	rmnimbus_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

private:
	virtual void received_byte(uint8_t byte) override;
};

DECLARE_DEVICE_TYPE(RMNIMBUS_KEYBOARD, rmnimbus_keyboard_device)

#endif // MAME_RM_RMNKBD_H
