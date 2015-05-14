// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef RMNKBD_H_
#define RMNKBD_H_

#include "bus/rs232/keyboard.h"

class rmnimbus_keyboard_device : public serial_keyboard_device
{
public:
	rmnimbus_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

protected:
	virtual void device_start();

private:
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line);

	required_ioport m_io_kbd8;
	required_ioport m_io_kbd9;
	required_ioport m_io_kbda;
	UINT8 m_state[11];
};

extern const device_type RMNIMBUS_KEYBOARD;

#endif /* RMNKBD_H_ */
