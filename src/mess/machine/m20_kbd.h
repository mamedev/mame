// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef M20KBD_H_
#define M20KBD_H_

#include "bus/rs232/keyboard.h"

class m20_keyboard_device : public serial_keyboard_device
{
public:
	m20_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

protected:
	virtual void device_start();
	virtual void rcv_complete();

private:
	void write(UINT8 data);
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line);
	UINT8 m_state[16];
};

extern const device_type M20_KEYBOARD;

#endif /* M20KBD_H_ */
