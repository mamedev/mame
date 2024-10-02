// license:BSD-3-Clause
// copyright-holders:Carl,Vas Crabb
#ifndef MAME_OLIVETTI_M20KBD_H
#define MAME_OLIVETTI_M20KBD_H

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"

class m20_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<9U>
{
public:
	m20_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	virtual void device_reset() override ATTR_COLD;
	virtual void key_make(uint8_t row, uint8_t column) override;

private:
	virtual void received_byte(uint8_t byte) override;

	required_ioport m_modifiers;
};

DECLARE_DEVICE_TYPE(M20_KEYBOARD, m20_keyboard_device)

#endif // MAME_OLIVETTI_M20KBD_H
