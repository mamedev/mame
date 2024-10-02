// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion Condor

******************************************************************************/

#ifndef MAME_MACHINE_PSION_CONDOR_H
#define MAME_MACHINE_PSION_CONDOR_H

#pragma once

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_condor_device

class psion_condor_device : public device_t, public device_serial_interface
{
public:
	psion_condor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	auto txd_handler() { return m_txd_handler.bind(); }
	auto rxc_handler() { return m_rxc_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto dtr_handler() { return m_dtr_handler.bind(); }
	auto int_handler() { return m_int_handler.bind(); }

	void write_rxd(int state) { }
	void write_cts(int state) { m_cts = state; }
	void write_dsr(int state) { m_dsr = state; }
	void write_dcd(int state) { m_dcd = state; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write_line m_txd_handler;
	devcb_write_line m_rxc_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_dtr_handler;
	devcb_write_line m_int_handler;

	util::fifo<uint8_t, 8> m_rx_fifo;
	util::fifo<uint8_t, 8> m_tx_fifo;

	int m_cts;
	int m_dsr;
	int m_ri;
	int m_dcd;

	uint16_t m_uart_divisor;
	uint8_t m_uart_line_control;
	uint8_t m_int_mask;
	uint8_t m_control1;
	uint8_t m_parallel_ddr;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_CONDOR, psion_condor_device)

#endif // MAME_MACHINE_PSION_CONDOR_H
