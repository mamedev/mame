// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef QX10KBD_H_
#define QX10KBD_H_

#include "bus/rs232/keyboard.h"

class qx10_keyboard_device : public serial_keyboard_device
{
public:
	qx10_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void rcv_complete() override;

private:
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line) override;
	void write(UINT8 data);

	required_ioport m_io_kbd8;
	required_ioport m_io_kbd9;
	required_ioport m_io_kbda;
	required_ioport m_io_kbdb;
	required_ioport m_io_kbdd;
	required_ioport m_io_kbde;
	required_ioport m_io_kbdf;

	UINT8 m_state[16];
};

extern const device_type QX10_KEYBOARD;

#endif /* QX10KBD_H_ */
