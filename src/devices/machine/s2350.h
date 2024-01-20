// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/**********************************************************************

 American Systems Inc. S2350 Universal Synchronous Receiver/Transmitter

**********************************************************************/

#ifndef MAME_MACHINE_S2350_H
#define MAME_MACHINE_S2350_H

#pragma once

#include "machine/timer.h"


//
// AMI S2350 USRT
//
class s2350_device : public device_t
{
public:
	s2350_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void rx_w(int state) { serial_rx_w(state); }

	auto tx_handler() { return m_tx_cb.bind(); }

	void transmitter_holding_reg_w(uint8_t data);
	void transmit_fill_reg_w(uint8_t data);
	void receiver_sync_reg_w(uint8_t data);

	u8   receiver_output_reg_r();
	u8   status_word_r();
	u8   receiver_sync_search();

	// transmit statuses
	auto transmit_buffer_empty_cb()      { return m_tbmt.bind(); }
	auto fill_character_transmitted_cb() { return m_fct.bind(); }

	// receive statuses
	auto received_data_available_cb()    { return m_rda.bind(); }
	auto sync_character_received_cb()    { return m_scr.bind(); }
	auto receiver_overrun_cb()           { return m_ror.bind(); }
	auto receiver_parity_error_cb()      { return m_rpe.bind(); }

	void receiver_restart();

	// transmitter clock
	void tcp_w();
	// receiver clock
	void rcp_w();

protected:

	virtual void device_start() override;
	virtual void device_reset() override;

	void serial_rx_w(int state);
	void update_receiver_shift();

	void set_transmit_buffer_empty(int val);
	void set_fill_char_transmitted(int val);

	void set_received_data_available(int val);
	void set_receiver_overrun(int val);
	void set_receiver_parity_error(int val);
	void set_sync_character_received(int val);

	void receive_byte(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(rx_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(tx_timer_cb);

	u8   m_serial_rx_line;

	// transmit flags
	bool m_transmit_buffer_empty;
	bool m_fill_char_transmitted;

	// transmit registers
	u8   m_transmitter_holding_reg;
	u8   m_transmitter_fill_reg;
	u8   m_transmitter_shift_reg;

	// transmit status callbacks
	devcb_write_line m_tbmt; // Transmit Buffer Empty
	devcb_write_line m_fct;  // Fill Character Transmitted

	// transmit state
	uint8_t m_serial_tx_state;

	// receive flags
	bool m_received_data_available;
	bool m_receiver_overrun;
	bool m_receiver_parity_error; // not currently used
	bool m_sync_character_received;

	// receive registers
	u8   m_receiver_output_reg;
	u8   m_receiver_sync_reg;
	u8   m_receiver_shift_reg;

	// receiver status callbacks
	devcb_write_line m_rda;  // Received Data Available
	devcb_write_line m_scr;  // Sync Character Received
	devcb_write_line m_ror;  // Receiver Overrun
	devcb_write_line m_rpe;  // Receiver Parity Error

	// receiver state
	u8   m_serial_rx_state;
	bool m_sync_search_active;
	bool m_in_sync;

	devcb_write_line m_tx_cb;
};


DECLARE_DEVICE_TYPE(S2350, s2350_device)

#endif // MAME_MACHINE_S2350_H
