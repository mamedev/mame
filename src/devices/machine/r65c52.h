// license:BSD-3-Clause
// copyright-holders:jwallace
/**********************************************************************

    Rockwell 65C52  Dual Asynchronous Communication Interface Adapter

    A slightly tweaked combination of two 6551 ACIAs on a single chip

**********************************************************************
                            _____   _____
                   RES   1 |*    \_/     | 40  Vcc
                    NC   2 |             | 39  _CS
                 XTALI   3 |             | 38  R/_W
                 XTALO   4 |             | 37  RS2
                CLKOUT   5 |             | 36  RS1
                    NC   6 |             | 35  RS0
                 _DSR2   7 |             | 34  NC
                 _DCD2   8 |             | 33  _DSR1
                 _CTS2   9 |             | 32  _DCD1
                 _RTS2  10 |    R65C52   | 31  _CTS1
                 _IRQ2  11 |             | 30  _RTS1
                  RxD2  12 |             | 29  _IRQ1
                 _DTR2  13 |             | 28  RxD1
                  TxD2  14 |             | 27  _DTR1
                   TxC  15 |             | 26  TxD1
                    D7  16 |             | 25  RxC
                    D6  17 |             | 24  D0
                    D5  18 |             | 23  D1
                    D4  19 |             | 22  D2
                   Vss  20 |_____________| 21  D3



**********************************************************************/

#ifndef MAME_MACHINE_R65C52_H
#define MAME_MACHINE_R65C52_H

#pragma once

#include "machine/clock.h"

class r65c52_device : public device_t
{
public:
	r65c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq1_handler() { return m_irq_handler[0].bind(); }
	auto irq2_handler() { return m_irq_handler[1].bind(); }
	auto txd1_handler() { return m_txd_handler[0].bind(); }
	auto txd2_handler() { return m_txd_handler[1].bind(); }
	auto rts1_handler() { return m_rts_handler[0].bind(); }
	auto rts2_handler() { return m_rts_handler[1].bind(); }
	auto dtr1_handler() { return m_dtr_handler[0].bind(); }
	auto dtr2_handler() { return m_dtr_handler[1].bind(); }

	void map(address_map &map);

	u8 read(offs_t offset);

	void write_txc(int state);
	void write_rxc(int state);
	void write_rxd1(int state);
	void write_rxd2(int state);

	void write_cts1(int state);
	void write_cts2(int state);
	void write_dsr1(int state);
	void write_dsr2(int state);
	void write_dcd1(int state);
	void write_dcd2(int state);

	void _write_cts(int idx, int state);
	void _write_dsr(int idx, int state);
	void _write_dcd(int idx, int state);

	void set_xtal(u32 clock);
	void set_xtal(const XTAL &clock) { set_xtal(clock.value()); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	enum
	{
		SR_RTS = 0x01,
		SR_DTR = 0x02,
		SR_BRK = 0x04,
		SR_DSR = 0x08,
		SR_DCD = 0x10,
		SR_CTS = 0x20,
		SR_TUR = 0x40,
		SR_FRAMING_ERROR = 0x80,
	};

	enum
	{
		PARITY_ODD = 0,
		PARITY_EVEN = 1,
		PARITY_MARK = 2,
		PARITY_SPACE = 3,
		PARITY_NONE = -1
	};

	enum
	{
		IRQ_RDRF= 0x01,
		IRQ_FOB = 0x02,
		IRQ_PAR = 0x04,
		IRQ_DSR = 0x08,
		IRQ_DCD = 0x10,
		IRQ_CTS = 0x20,
		IRQ_TDRE= 0x40,
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

	u8 stoplengthcounter(int idx);

	void output_irq(int idx, int irq);
	void output_txd(int idx, int txd);
	void output_rts(int idx, int rts);
	void output_dtr(int idx, int dtr);

	void update_irq(int idx);
	void update_divider(int idx);

	u8 read_isr(int idx);
	u8 read_rdr(int idx);
	u8 read_status(int idx);
	u8 read_command(int idx);


	void ier_0_w(u8 data) { write_ier(0, data); }
	void ier_1_w(u8 data) { write_ier(1, data); }

	u8 isr_0_r() { return read_isr(0); }
	u8 isr_1_r() { return read_isr(1); }

	u8 rdr_0_r() { return read_rdr(0); }
	u8 rdr_1_r() { return read_rdr(1); }

	u8 status_0_r() { return read_status(0); }
	u8 status_1_r() { return read_status(1); }

	void tdr_0_w(u8 data) { write_tdr(0, data); }
	void tdr_1_w(u8 data) { write_tdr(1, data); }

	void write_ier(int idx, u8 data);
	void write_tdr(int idx, u8 data);

	void aux_compare_0_w(u8 data);
	void aux_compare_1_w(u8 data);

	void format_ctrl_0_w(u8 data);
	void format_ctrl_1_w(u8 data);

	void write_aux_ctrl(int idx, u8 data);
	void write_compare(int idx, u8 data);
	void write_control(int idx, u8 data);
	void write_format(int idx, u8 data);
	void isr_bit_set(int idx);

	void internal_clock1(int state);
	void internal_clock2(int state);
	void receiver_clock(int idx, int state);
	void transmitter_clock(int idx, int state);

	static const int internal_divider[16];
	static const int transmitter_controls[4][3];

	required_device<clock_device> m_internal_clock1;
	required_device<clock_device> m_internal_clock2;
	devcb_write_line::array<2> m_irq_handler;
	devcb_write_line::array<2> m_txd_handler;
	devcb_write_line m_rxc_handler;
	devcb_write_line::array<2> m_rts_handler;
	devcb_write_line::array<2> m_dtr_handler;

	u8 m_aux_ctrl[2];
	u8 m_control[2];
	u8 m_compare[2];
	u8 m_format[2];
	u8 m_status[2];
	u8 m_tdr[2];
	bool m_tdre[2];
	u8 m_rdr[2];
	bool m_rdrf[2];

	u8 m_ier[2];
	u8 m_isr[2];

	u8 m_irq[2];

	bool m_overrun[2];
	bool m_parity_err[2];
	u8 m_parity_err_mode[2];

	u8 m_txd[2];
	u8 m_rxc;
	u8 m_rts[2];
	u8 m_dtr[2];

	u32 m_xtal;
	u8 m_divide[2];
	u8 m_cts[2];
	u8 m_dsr[2];
	u8 m_dcd[2];
	u8 m_rxd[2];

	u8 m_wordlength[2];
	u8 m_stoplength[2];
	u8 m_brk[2];
	u8 m_echo_mode[2];
	int m_parity[2];

	u8 m_rx_state[2];
	u8 m_rx_clock[2];
	u8 m_rx_bits[2];
	u8 m_rx_shift[2];
	int m_rx_parity[2];
	u8 m_rx_counter[2];

	u8 m_tx_state[2];
	u8 m_tx_output[2];
	u8 m_tx_clock[2];
	u8 m_tx_bits[2];
	u8 m_tx_shift[2];
	int m_tx_parity[2];
	u8 m_tx_counter[2];
};

DECLARE_DEVICE_TYPE(R65C52, r65c52_device)

#endif // MAME_MACHINE_R65C52_H
