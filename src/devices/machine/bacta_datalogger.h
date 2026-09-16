// license:BSD-3-Clause
// copyright-holders:James Wallace
#ifndef MAME_MACHINE_BACTA_DATALOGGER_H
#define MAME_MACHINE_BACTA_DATALOGGER_H

#include "diserial.h"

class bacta_datalogger_device : public device_t,
	public device_serial_interface
{
public:
	bacta_datalogger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void write_txd(int state) {device_serial_interface::rx_w(state); }
	auto rxd_handler() { return m_rxd_handler.bind(); }

	void output_rxd(int state) { m_rxd_handler(state); }

protected:
	bacta_datalogger_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	void tx_queue();

	devcb_write_line m_rxd_handler;
	uint8_t m_last_input;
	uint8_t m_output_char;
};

DECLARE_DEVICE_TYPE(BACTA_DATALOGGER, bacta_datalogger_device)

#endif // MAME_MACHINE_BACTA_DATALOGGER_H
