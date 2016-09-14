// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef MAME_MACHINE_NGEN_KB_H
#define MAME_MACHINE_NGEN_KB_H

#include "bus/rs232/keyboard.h"

class ngen_keyboard_device : public serial_keyboard_device
{
public:
	ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;
	virtual void key_make(UINT8 row, UINT8 column) override;
	virtual void key_break(UINT8 row, UINT8 column) override;

private:
	void write(UINT8 data);

	UINT8 m_keys_down;
	UINT8 m_last_reset;
};

extern const device_type NGEN_KEYBOARD;

#endif // MAME_MACHINE_NGEN_KB_H
