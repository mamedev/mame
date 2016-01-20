// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard

#ifndef NGEN_KB_H_
#define NGEN_KB_H_

#include "bus/rs232/keyboard.h"

class ngen_keyboard_device : public serial_keyboard_device
{
public:
	ngen_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const override;
	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override { device_serial_interface::rx_w(state); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;

private:
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line) override;
	UINT8 row_number(UINT8 code);
	void write(UINT8 data);

	bool m_keys_down;
	bool m_last_reset;
};

extern const device_type NGEN_KEYBOARD;

#endif /* NGENKB_H_ */
