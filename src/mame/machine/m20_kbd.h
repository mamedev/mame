// license:BSD-3-Clause
// copyright-holders:Carl,Vas Crabb
#ifndef MAME_MACHINE_M20KBD_H
#define MAME_MACHINE_M20KBD_H

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"

class m20_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<9U>
{
public:
	m20_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void key_make(UINT8 row, UINT8 column) override;

private:
	virtual void received_byte(UINT8 byte) override;

	required_ioport m_modifiers;
};

extern const device_type M20_KEYBOARD;

#endif // MAME_MACHINE_M20KBD_H
