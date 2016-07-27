// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_QX10KBD_H
#define MAME_MACHINE_QX10KBD_H

#include "bus/rs232/keyboard.h"

class qx10_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<16U>
{
public:
	qx10_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void key_make(UINT8 row, UINT8 column) override;

private:
	virtual void received_byte(UINT8 data) override;
};

extern const device_type QX10_KEYBOARD;

#endif // MAME_MACHINE_QX10KBD_H
