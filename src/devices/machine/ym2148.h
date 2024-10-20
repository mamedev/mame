// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    ym2148.h

    Yamaha YM2148 Midi and keyboard interface

*********************************************************************/

#ifndef MAME_MACHINE_YM2148_H
#define MAME_MACHINE_YM2148_H

#pragma once

#include "diserial.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

class ym2148_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	ym2148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto txd_handler() { return m_txd_handler.bind(); }
	auto port_write_handler() { return m_port_write_handler.bind(); }
	auto port_read_handler() { return m_port_read_handler.bind(); }
	auto irq_handler() { return m_irq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void write_rxd(int state);
	uint8_t get_irq_vector();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(serial_clock_tick);

private:
	devcb_write_line m_txd_handler;
	devcb_write_line m_irq_handler;
	devcb_write8 m_port_write_handler;  // write ST0-ST7
	devcb_read8 m_port_read_handler;    // read SD0-SD7
	int m_irq_state;
	uint8_t m_irq_vector;
	uint8_t m_external_irq_vector;
	// Does this chip have 1 or 2 data registers?
	uint8_t m_data_out;
	uint8_t m_data_in;
	uint8_t m_control;
	uint8_t m_status;
	emu_timer *m_timer;
	int m_rxd;
	bool m_tx_busy;

	void receive_clock();
	void transmit_clock();
	void update_irq();

	enum
	{
		STATUS_TRANSMIT_READY = 0x01,
		STATUS_RECEIVE_BUFFER_FULL = 0x02,
		STATUS_OVERRUN_ERROR = 0x10,
		STATUS_FRAMING_ERROR = 0x20,
		CONTROL_TRANSMIT_ENABLE = 0x01,
		CONTROL_TRANSMIT_IRQ_ENABLE = 0x02,
		CONTROL_RECEIVE_ENABLE = 0x04,
		CONTROL_RECEIVE_IRQ_ENABLE = 0x08
	};
};


DECLARE_DEVICE_TYPE(YM2148, ym2148_device)

#endif // MAME_MACHINE_YM2148_H
