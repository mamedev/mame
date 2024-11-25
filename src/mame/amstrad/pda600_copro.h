// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Amstrad PDA600 Coprocessor HLE

**********************************************************************/

#ifndef MAME_AMSTRAD_PDA600_COPRO_H
#define MAME_AMSTRAD_PDA600_COPRO_H

#pragma once

#include "diserial.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pda600_copro_device

class pda600_copro_device : public device_t,
							public device_buffered_serial_interface<1 + 1 + 255 + 1>
{
public:
	pda600_copro_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto tx_callback()    { return m_tx_cb.bind(); }
	auto tone_callback()  { return m_tone_cb.bind(); }

	void wakeup_w(int state);
	void reset_w(int state)    { if (state)  device_reset(); }
	void write_txd(int state)  { rx_w(state); }

private:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }
	void received_byte(u8 byte) override;

	TIMER_CALLBACK_MEMBER(update_timer);
	void send_byte(u8 byte);
	void send_resp();
	void exec_beep();
	void exec_train();
	void exec_recognize();
	u8 recognize_char();

	devcb_write_line m_tx_cb;
	devcb_write8     m_tone_cb;
	required_ioport_array<4> m_fake_ioport;
	emu_timer *      m_update_timer;
	u8               m_state;
	u8               m_resp_type;
	u8               m_resp_data;
	u8               m_buf_size;
	std::array<u8,256> m_buf;
};


DECLARE_DEVICE_TYPE(PDA600_COPRO_HLE, pda600_copro_device)

#endif // MAME_AMSTRAD_PDA600_COPRO_H
