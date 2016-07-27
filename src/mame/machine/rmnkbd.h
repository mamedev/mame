// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_RMNKBD_H
#define MAME_MACHINE_RMNKBD_H

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"

class rmnimbus_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<11U>
{
public:
	rmnimbus_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void key_make(UINT8 row, UINT8 column) override;
	virtual void key_break(UINT8 row, UINT8 column) override;

private:
	virtual void received_byte(UINT8 byte) override;
};

extern const device_type RMNIMBUS_KEYBOARD;

#endif // MAME_MACHINE_RMNKBD_H
