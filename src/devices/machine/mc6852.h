// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MC6852 Synchronous Serial Data Adapter emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 24  _CTS
               Rx DATA   2 |             | 23  _DCD
                Rx CLK   3 |             | 22  D0
                Tx CLK   4 |             | 21  D1
               SM/_DTR   5 |             | 20  D2
               Tx DATA   6 |   MC6852    | 19  D3
                  _IRQ   7 |             | 18  D4
                   TUF   8 |             | 17  D5
                _RESET   9 |             | 16  D6
                   _CS   9 |             | 15  D7
                    RS   9 |             | 14  E
                   Vcc  10 |_____________| 13  R/_W

**********************************************************************/

#ifndef MAME_MACHINE_MC6852_H
#define MAME_MACHINE_MC6852_H

#pragma once

#include "diserial.h"

#include <queue>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc6852_device

class mc6852_device :   public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	mc6852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rx_clock(int clock) { m_rx_clock = clock; }
	void set_tx_clock(int clock) { m_tx_clock = clock; }

	auto tx_data_callback() { return m_write_tx_data.bind(); }
	auto irq_callback() { return m_write_irq.bind(); }
	auto sm_dtr_callback() { return m_write_sm_dtr.bind(); }
	auto tuf_callback() { return m_write_tuf.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void rx_data_w(int state) { device_serial_interface::rx_w(state); }
	void rx_clk_w(int state) { rx_clock_w(state); }
	void tx_clk_w(int state) { tx_clock_w(state); }
	void cts_w(int state) { m_cts = state; }
	void dcd_w(int state) { m_dcd = state; }

	int sm_dtr_r() { return m_sm_dtr; }
	int tuf_r() { return m_tuf; }

	// These are to allow integration of this driver with code
	// controlling floppy disks.
	void receive_byte(uint8_t data);
	uint8_t get_tx_byte(int *tuf);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

private:
	enum
	{
		S_IRQ = 0x80,
		S_PE = 0x40,
		S_RX_OVRN = 0x20,
		S_TUF = 0x10,
		S_CTS = 0x08,
		S_DCD = 0x04,
		S_TDRA = 0x02,
		S_RDA = 0x01
	};

	enum
	{
		C1_AC_MASK = 0xc0,
		C1_AC_C2 = 0x00,
		C1_AC_C3 = 0x40,
		C1_AC_SYNC = 0x80,
		C1_AC_TX_FIFO = 0xc0,
		C1_AC2 = 0x80,
		C1_AC1 = 0x40,
		C1_RIE = 0x20,
		C1_TIE = 0x10,
		C1_CLEAR_SYNC = 0x08,
		C1_STRIP_SYNC = 0x04,
		C1_TX_RS = 0x02,
		C1_RX_RS = 0x01
	};

	enum
	{
		C2_EIE = 0x80,
		C2_TX_SYNC = 0x40,
		C2_WS_MASK = 0x38,
		C2_WS3 = 0x20,
		C2_WS2 = 0x10,
		C2_WS1 = 0x08,
		C2_1_2_BYTE = 0x04,
		C2_PC_MASK = 0x03,
		C2_PC2 = 0x02,
		C2_PC1 = 0x01
	};

	enum
	{
		C3_CTUF = 0x08,
		C3_CTS = 0x04,
		C3_1_2_SYNC = 0x02,
		C3_E_I_SYNC = 0x01
	};

	devcb_write_line       m_write_tx_data;
	devcb_write_line       m_write_irq;
	devcb_write_line       m_write_sm_dtr;
	devcb_write_line       m_write_tuf;

	uint8_t m_status;         // status register
	uint8_t m_cr[3];          // control registers
	uint8_t m_scr;            // sync code register
	uint8_t m_tdr;            // transmit data register
	uint8_t m_tsr;            // transmit shift register
	uint8_t m_rdr;            // receive data register
	uint8_t m_rsr;            // receive shift register

	std::queue<uint8_t> m_rx_fifo;
	std::queue<uint8_t> m_tx_fifo;

	int m_rx_clock;
	int m_tx_clock;
	int m_cts;              // clear to send
	int m_dcd;              // data carrier detect
	int m_sm_dtr;           // sync match/data terminal ready
	int m_tuf;              // transmitter underflow
	int m_in_sync;
};


// device type definition
DECLARE_DEVICE_TYPE(MC6852, mc6852_device)

#endif // MAME_MACHINE_MC6852_H
