// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    SAA5070 Viewdata Input/Output Peripheral (VIOP)

**********************************************************************

    Pinning:                ____  ____
                           |    \/    |
                   Vss   1 |          | 40  DON
                   IMP   2 |          | 39  CARDET
                TFSKIN   3 |          | 38  FSKIN
               TFSKOUT   4 |          | 37  DOCDI
                FSKOUT   5 |          | 36  CS
                TXDATA   6 |          | 35  nWR
                RXDATA   7 |          | 34  nRD
                    F1   8 |          | 33  D7
                DLIM B   9 |          | 32  D6
               nDATA B  10 | SAA5070  | 31  D5
               nDLEN B  11 |          | 30  D4
               nDATA A  12 |          | 29  D3
        DLIM A/nDLEN A  13 |          | 28  D2
                 IBCLK  14 |          | 27  D1
                   PA4  15 |          | 26  D0
                   PA3  16 |          | 25  ALE
                   PA2  17 |          | 24  PB0
                   PA1  18 |          | 23  PB1
                   PA0  19 |          | 22  PB2
                   Vdd  20 |          | 21  PB3
                           |__________|

*********************************************************************/

#ifndef MAME_MACHINE_SAA5070_H
#define MAME_MACHINE_SAA5070_H

#pragma once

#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa5070_uart_device

class saa5070_uart_device : public device_t, public device_serial_interface
{
public:
	saa5070_uart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto txd_handler() { return m_txd_handler.bind(); }

	void set_baud_rate(int rxbaud, int txbaud, int parity);
	uint8_t status() { return m_status; }

	void tx_byte(uint8_t data);
	uint8_t rx_byte();

	void rx_enable(int state) { m_rx_enabled = state; }
	void tx_enable(int state) { m_tx_enabled = state; }

	void write_rxd(int state);
	void write_dcd(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	devcb_write_line m_txd_handler;

	uint8_t m_status;
	int m_rx_enabled;
	int m_tx_enabled;
	int m_rxd;
};


// ======================> saa5070_device

class saa5070_device : public device_t
{
public:
	saa5070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// port handlers
	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }
	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }

	// autodial handler
	auto imp_handler() { return m_imp_handler.bind(); }

	// uart handlers
	auto txdata_handler() { return m_txdata_handler.bind(); }

	void write_rxdata(int state) { m_line_uart->write_rxd(state); }
	void write_cardet(int state) { m_line_uart->write_dcd(state); }

	// read/write access
	void address_w(uint8_t data);
	uint8_t data_r();
	void data_w(uint8_t data);

	// direct-mapped read/write access
	uint8_t read_direct(offs_t offset);
	void write_direct(offs_t offset, uint8_t data);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<saa5070_uart_device> m_line_uart;
	required_device<saa5070_uart_device> m_tape_uart;

	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;
	devcb_write_line m_imp_handler;
	devcb_write_line m_txdata_handler;

	emu_timer *m_timeout_timer;
	emu_timer *m_dial_timer;

	void write_txdata(int state) { m_txdata_handler(state); }

	TIMER_CALLBACK_MEMBER(timeout);
	TIMER_CALLBACK_MEMBER(dial);

	uint8_t m_index = 0;
	uint8_t m_sr[16];
};


// device type definition
DECLARE_DEVICE_TYPE(SAA5070, saa5070_device)


#endif // MAME_MACHINE_SAA5070_H
