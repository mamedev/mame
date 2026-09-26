// license:BSD-3-Clause
// copyright-holders:buffi
/***************************************************************************

    Serial touch screen used by mmmbanc.

***************************************************************************/

#ifndef MAME_CAVE_CV1K_TOUCH_H
#define MAME_CAVE_CV1K_TOUCH_H

#pragma once

#include "diserial.h"

class cv1k_touchscreen_device : public device_t, public device_serial_interface
{
public:
	cv1k_touchscreen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Serial data out, to the host's RXD.
	auto txd_handler() { return m_txd_cb.bind(); }

	// Serial data in, from the host's TXD.
	void rxd_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	static constexpr unsigned TX_BUFFER_SIZE = 8;

	// Commands sent by the host.
	enum : uint8_t
	{
		CMD_RESET  = 0x55, // -> ACK
		CMD_ENQ    = 0x05, // followed by CMD_ENQ_ARG
		CMD_ENQ_E  = 0x45, // -> ACK
		CMD_ID     = 0x15, // -> two identification bytes
		CMD_STREAM = 0x21  // start reporting
	};

	// Bytes sent by the panel.
	enum : uint8_t
	{
		RSP_ACK     = 0x06,
		RSP_PEN_UP  = 0x10,
		RSP_PEN_DWN = 0x11
	};

	void queue_byte(uint8_t data);
	void queue_report();
	void start_transmit();

	devcb_write_line m_txd_cb;

	required_ioport m_touch;
	required_ioport m_touch_x;
	required_ioport m_touch_y;

	uint8_t m_tx_buffer[TX_BUFFER_SIZE];
	uint8_t m_tx_head, m_tx_count;
	bool m_streaming;
	bool m_got_enq;
};

DECLARE_DEVICE_TYPE(CV1K_TOUCHSCREEN, cv1k_touchscreen_device)

#endif // MAME_CAVE_CV1K_TOUCH_H
