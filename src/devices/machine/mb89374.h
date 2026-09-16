// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
/*
MB89374

Fujitsu
Data Link Controller

                _____   _____
       CLK   1 |*    \_/     | 42  Vcc
   FORMAT#   2 |             | 41  RxDACK#/PI3
    RESET#   3 |             | 40  TxDACK#/PI2
   RD#/DS#   4 |             | 39  RxDRQ/PO3
 WR#/R/W#    5 |             | 38  TxDRQ/PO2
       CS#   6 |             | 37  IRQT
        A4   7 |             | 36  Vss
        A3   8 |             | 35  IRQ
        A2   9 |             | 34  FD#/DTR#
        A1  10 |   MB89374   | 33  SCLK/DSR#
        A0  11 |             | 32  TxLAST#/CI#
       Vss  12 |             | 31  TxCI#/PI1
        D7  13 |             | 30  TxCO#/PO1
        D6  14 |             | 29  TxD
        D5  15 |             | 28  RxD
        D4  16 |             | 27  RxCI#/PI0
        D3  17 |             | 26  RxCO#/PO0
        D2  18 |             | 25  DCD#
        D1  19 |             | 24  CTS#
        D0  20 |             | 23  TCLK
       Vss  21 |_____________| 22  LOC#/RTS#
*/

#ifndef MAME_MACHINE_MB89374_H
#define MAME_MACHINE_MB89374_H

#pragma once

#include "osdfile.h"


class mb89374_device : public device_t,
					   public device_execute_interface
{
public:
	// construction/destruction
	mb89374_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	template <unsigned N> auto out_po_callback() { return m_out_po_cb[N].bind(); }

	// read/write handlers
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void pi0_w(int state);
	void pi1_w(int state);
	void pi2_w(int state);
	void pi3_w(int state);

	void ci_w(int state);

	uint8_t dma_r();
	void dma_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	int m_icount;

private:
	// internal state
	inline void set_irq(int state);
	inline void set_po0(int state);
	inline void set_po1(int state);
	inline void set_po2(int state);
	inline void set_po3(int state);

	devcb_write_line m_out_irq_cb;
	devcb_write_line::array<4> m_out_po_cb;

	// pins
	int m_irq;
	int m_po[4];
	int m_pi[4];
	int m_ci;

	// registers
	uint8_t m_smr0;
	uint8_t m_smr1;
	uint8_t m_smr2;
	uint8_t m_chrr0;
	uint8_t m_chrr1;
	uint8_t m_msr;
	uint8_t m_mcr;
	uint8_t m_rxsr0;
	uint8_t m_rxsr1;
	uint8_t m_rxcr;
	uint8_t m_rxier;
	uint8_t m_txsr;
	uint8_t m_txcr;
	uint8_t m_txier;
	uint8_t m_sdr;
	uint8_t m_txbcr0;
	uint8_t m_txbcr1;
	uint8_t m_txfr0;
	uint8_t m_txfr1;
	uint8_t m_smr3;
	uint8_t m_portr;
	uint8_t m_reqr;
	uint8_t m_maskr;
	uint8_t m_b1psr;
	uint8_t m_b1pcr;
	uint8_t m_bg1dr;
	uint8_t m_b2sr;
	uint8_t m_b2cr;
	uint8_t m_bg2dr;

	uint16_t m_intr_delay;
	uint16_t m_sock_delay;

	// 512 byte fifos (not in hardware)
	uint8_t  m_rx_buffer[0x200];
	uint16_t m_rx_offset;
	uint16_t m_rx_length;

	uint8_t  m_tx_buffer[0x200];
	uint16_t m_tx_offset;

	void    rxReset();
	uint8_t rxRead();

	void txReset();
	void txWrite(uint8_t data);
	void txComplete();

	osd_file::ptr m_line_rx;
	osd_file::ptr m_line_tx;
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_socket_buffer[0x200];
	void checkSockets();
};


// device type definition
DECLARE_DEVICE_TYPE(MB89374, mb89374_device)

#endif // MAME_MACHINE_MB89374_H
