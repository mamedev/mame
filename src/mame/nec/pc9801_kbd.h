// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    PC-9801 Keyboard simulation

***************************************************************************/
#ifndef MAME_NEC_PC9801_KBD_H
#define MAME_NEC_PC9801_KBD_H

#pragma once

#include "machine/keyboard.h"
#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_kbd_device

class pc9801_kbd_device : public device_t
						, public device_buffered_serial_interface<16U>
						, protected device_matrix_keyboard_interface<16>
{
public:
	// construction/destruction
	pc9801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	auto rxd_callback()    { return m_tx_cb.bind(); }
//  auto rdy_callback()    { return m_rdy_cb.bind(); }
//  auto rty_callback()    { return m_rty_cb.bind(); }

	// input_rts?
	void input_txd(int state)  { device_buffered_serial_interface::rx_w(state); }
	void input_rty(int state);
	// input_rdy?
	void input_kbde(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }
//  virtual void rcv_callback() override;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	void transmit_byte(u8 byte);
	virtual void received_byte(u8 byte) override;

	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
private:

	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_ODD;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 1'200;

	uint8_t translate(uint8_t row, uint8_t column);
	void send_key(uint8_t code);

	devcb_write_line m_tx_cb;
//  devcb_write_line m_rdy_cb;
//  devcb_write_line m_rty_cb;

//  bool m_repeat_state;
	int m_kbde_state;
	int m_rty_state;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_KBD, pc9801_kbd_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_NEC_PC9801_KBD_H
