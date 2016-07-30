// license:BSD-3-Clause
// copyright-holders:Barry Rodewald,Vas Crabb
#ifndef MAME_MACHINE_X68K_KBD_H
#define MAME_MACHINE_X68K_KBD_H

#include "bus/rs232/keyboard.h"

class x68k_keyboard_device : public buffered_rs232_device<16U>, protected device_matrix_keyboard_interface<15U>
{
public:
	x68k_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void key_make(UINT8 row, UINT8 column) override;
	virtual void key_repeat(UINT8 row, UINT8 column) override;
	virtual void key_break(UINT8 row, UINT8 column) override;

private:
	virtual void received_byte(UINT8 data) override;

	int m_delay;  // keypress delay after initial press
	int m_repeat; // keypress repeat rate
	UINT8 m_enabled;  // keyboard enabled?
};

extern const device_type X68K_KEYBOARD;

#endif // MAME_MACHINE_X68K_KBD_H
