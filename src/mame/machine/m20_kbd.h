// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef M20KBD_H_
#define M20KBD_H_

#include "bus/rs232/keyboard.h"

class m20_keyboard_device : public serial_keyboard_device
{
public:
	m20_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void rcv_complete() override;

private:
	void write(UINT8 data);
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line) override;
	UINT8 m_state[16];
};

extern const device_type M20_KEYBOARD;

#endif /* M20KBD_H_ */
