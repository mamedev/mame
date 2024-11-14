// license:BSD-3-Clause
// copyright-holders:smf, Carl
/**********************************************************************

    8250 UART interface and emulation

**********************************************************************/

#ifndef MAME_MACHINE_INS8250_H
#define MAME_MACHINE_INS8250_H

#pragma once

#include "diserial.h"

/***************************************************************************
    CLASS DEFINITIONS
***************************************************************************/

class ins8250_uart_device : public device_t, public device_serial_interface
{
public:
	auto out_tx_callback() { return m_out_tx_cb.bind(); }
	auto out_dtr_callback() { return m_out_dtr_cb.bind(); }
	auto out_rts_callback() { return m_out_rts_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_out1_callback() { return m_out_out1_cb.bind(); }
	auto out_out2_callback() { return m_out_out2_cb.bind(); }

	void ins8250_w(offs_t offset, u8 data);
	u8 ins8250_r(offs_t offset);

	void dcd_w(int state);
	void dsr_w(int state);
	void ri_w(int state);
	void cts_w(int state);
	void rx_w(int state);
	int intrpt_r();

protected:
	enum class dev_type {
		INS8250,
		INS8250A,
		NS16450,
		NS16550,
		NS16550A
	};

	ins8250_uart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, dev_type device_type);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

	virtual void set_fcr(u8 data) { }
	virtual void push_tx(u8 data) { }
	virtual u8 pop_rx() { return 0; }

	void trigger_int(int flag);
	void clear_int(int flag);

	void update_baud_rate();

	const dev_type m_device_type;
	struct {
		u8 thr;  /* 0 -W transmitter holding register */
		u8 rbr;  /* 0 R- receiver buffer register */
		u8 ier;  /* 1 RW interrupt enable register */
		u16 dl;  /* 0/1 RW divisor latch (if DLAB = 1) */
		u8 iir;  /* 2 R- interrupt identification register */
		u8 fcr;
		u8 lcr;  /* 3 RW line control register (bit 7: DLAB) */
		u8 mcr;  /* 4 RW modem control register */
		u8 lsr;  /* 5 R- line status register */
		u8 msr;  /* 6 R- modem status register */
		u8 scr;  /* 7 RW scratch register */
	} m_regs;
private:
	u8 m_int_pending;

	devcb_write_line    m_out_tx_cb;
	devcb_write_line    m_out_dtr_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_out1_cb;
	devcb_write_line    m_out_out2_cb;

	void update_interrupt();
	void update_msr();

	int m_txd;
	int m_rxd;
	int m_dcd;
	int m_dsr;
	int m_ri;
	int m_cts;
};

class ins8250_device : public ins8250_uart_device
{
public:
	ins8250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class ns16450_device : public ins8250_uart_device
{
public:
	ns16450_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class ns16550_device : public ins8250_uart_device
{
public:
	ns16550_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void set_fcr(u8 data) override;
	virtual void push_tx(u8 data) override;
	virtual u8 pop_rx() override;

	TIMER_CALLBACK_MEMBER(timeout_expired);
private:
	void set_timer() { m_timeout->adjust(attotime::from_hz((clock()*4*8)/(m_regs.dl*16))); }
	int m_rintlvl;
	u8 m_rfifo[16];
	u8 m_efifo[16];
	u8 m_tfifo[16];
	int m_rhead, m_rtail, m_rnum;
	int m_thead, m_ttail;
	emu_timer *m_timeout;
};

class pc16552_device : public device_t
{
public:
	pc16552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset) { return ((offset & 8) ? m_chan1 : m_chan0)->ins8250_r(offset & 7); }
	void write(offs_t offset, u8 data) { ((offset & 8) ? m_chan1 : m_chan0)->ins8250_w(offset & 7, data); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	ns16550_device *m_chan0;
	ns16550_device *m_chan1;
};

DECLARE_DEVICE_TYPE(PC16552D, pc16552_device)
DECLARE_DEVICE_TYPE(INS8250,  ins8250_device)
DECLARE_DEVICE_TYPE(NS16450,  ns16450_device)
DECLARE_DEVICE_TYPE(NS16550,  ns16550_device)

#endif // MAME_MACHINE_INS8250_H
