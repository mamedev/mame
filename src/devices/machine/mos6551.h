// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    MOS Technology 6551 Asynchronous Communication Interface Adapter

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 28  R/_W
                   CS0   2 |             | 27  phi2
                  _CS1   3 |             | 26  _IRQ
                  _RES   4 |             | 25  DB7
                   RxC   5 |             | 24  DB6
                 XTAL1   6 |             | 23  DB5
                 XTAL2   7 |   MOS6551   | 22  DB4
                  _RTS   8 |             | 21  DB3
                  _CTS   9 |             | 20  DB2
                   TxD  10 |             | 19  DB1
                  _DTR  11 |             | 18  DB0
                   RxD  12 |             | 17  _DSR
                   RS0  13 |             | 16  _DCD
                   RS1  14 |_____________| 15  Vcc

**********************************************************************/

#ifndef MAME_MACHINE_MOS6551_H
#define MAME_MACHINE_MOS6551_H

#pragma once

#include "machine/clock.h"

class mos6551_device : public device_t
{
public:
	mos6551_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto txd_handler() { return m_txd_handler.bind(); }
	auto rxc_handler() { return m_rxc_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }
	auto dtr_handler() { return m_dtr_handler.bind(); }

	void map(address_map &map) ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void write_xtal1(int state); // txc
	void write_rxd(int state);
	void write_rxc(int state);
	void write_cts(int state);
	void write_dsr(int state);
	void write_dcd(int state);

	void set_xtal(uint32_t clock);
	void set_xtal(const XTAL &clock) { set_xtal(clock.value()); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	enum
	{
		SR_PARITY_ERROR = 0x01,
		SR_FRAMING_ERROR = 0x02,
		SR_OVERRUN = 0x04,
		SR_RDRF = 0x08,
		SR_TDRE = 0x10,
		SR_DCD = 0x20,
		SR_DSR = 0x40,
		SR_IRQ = 0x80
	};

	enum
	{
		PARITY_NONE = 0,
		PARITY_ODD = 1,
		PARITY_EVEN = 3,
		PARITY_MARK = 5,
		PARITY_SPACE = 7
	};

	enum
	{
		IRQ_DCD = 1,
		IRQ_DSR = 2,
		IRQ_RDRF = 4,
		IRQ_TDRE = 8,
		IRQ_CTS = 16
	};

	enum
	{
		STATE_START,
		STATE_DATA,
		STATE_STOP
	};

	enum
	{
		OUTPUT_TXD,
		OUTPUT_MARK,
		OUTPUT_BREAK
	};

	void output_irq(int irq);
	void output_txd(int txd);
	void output_rxc(int rxc);
	void output_rts(int rts);
	void output_dtr(int dtr);

	void update_irq();
	void update_divider();

	uint8_t read_rdr();
	uint8_t read_status();
	uint8_t read_command();
	uint8_t read_control();

	void write_tdr(uint8_t data);
	void write_reset(uint8_t data);
	void write_command(uint8_t data);
	void write_control(uint8_t data);

	int stoplength();

	void internal_clock(int state);
	void receiver_clock(int state);
	void transmitter_clock(int state);

	static const int internal_divider[16];
	static const int transmitter_controls[4][3];

	required_device<clock_device> m_internal_clock;
	devcb_write_line m_irq_handler;
	devcb_write_line m_txd_handler;
	devcb_write_line m_rxc_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_dtr_handler;

	uint8_t m_control;
	uint8_t m_command;
	uint8_t m_status;
	uint8_t m_tdr;
	uint8_t m_rdr;

	uint8_t m_irq_state;

	int m_irq;
	int m_txd;
	int m_rxc;
	int m_rts;
	int m_dtr;

	uint32_t m_xtal;
	int m_divide;
	int m_cts;
	int m_dsr;
	int m_dcd;
	int m_rxd;

	int m_wordlength;
	int m_extrastop;
	int m_brk;
	int m_echo_mode;
	int m_parity;

	int m_rx_state;
	int m_rx_clock;
	int m_rx_bits;
	int m_rx_shift;
	int m_rx_parity;
	int m_rx_counter;
	int m_rx_irq_enable;
	int m_rx_internal_clock;

	int m_tx_state;
	int m_tx_output;
	int m_tx_clock;
	int m_tx_bits;
	int m_tx_shift;
	int m_tx_parity;
	int m_tx_counter;
	int m_tx_enable;
	int m_tx_irq_enable;
	int m_tx_internal_clock;
};

DECLARE_DEVICE_TYPE(MOS6551, mos6551_device)

#endif // MAME_MACHINE_MOS6551_H
