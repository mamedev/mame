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
	template <class Object> devcb_base &set_out_tx_callback(Object &&cb) { return m_out_tx_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_dtr_callback(Object &&cb) { return m_out_dtr_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_rts_callback(Object &&cb) { return m_out_rts_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_int_callback(Object &&cb) { return m_out_int_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_out1_callback(Object &&cb) { return m_out_out1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_out2_callback(Object &&cb) { return m_out_out2_cb.set_callback(std::forward<Object>(cb)); }
	auto out_tx_callback() { return m_out_tx_cb.bind(); }
	auto out_dtr_callback() { return m_out_dtr_cb.bind(); }
	auto out_rts_callback() { return m_out_rts_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_out1_callback() { return m_out_out1_cb.bind(); }
	auto out_out2_callback() { return m_out_out2_cb.bind(); }

	DECLARE_WRITE8_MEMBER(ins8250_w);
	DECLARE_READ8_MEMBER(ins8250_r);

	DECLARE_WRITE_LINE_MEMBER(dcd_w);
	DECLARE_WRITE_LINE_MEMBER(dsr_w);
	DECLARE_WRITE_LINE_MEMBER(ri_w);
	DECLARE_WRITE_LINE_MEMBER(cts_w);
	DECLARE_WRITE_LINE_MEMBER(rx_w);
	DECLARE_READ_LINE_MEMBER(intrpt_r);

protected:
	enum class dev_type {
		INS8250,
		INS8250A,
		NS16450,
		NS16550,
		NS16550A
	};

	ins8250_uart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, dev_type device_type);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;

	virtual void set_fcr(uint8_t data) { }
	virtual void push_tx(uint8_t data) { }
	virtual uint8_t pop_rx() { return 0; }

	void trigger_int(int flag);
	void clear_int(int flag);

	void update_baud_rate();

	const dev_type m_device_type;
	struct {
		uint8_t thr;  /* 0 -W transmitter holding register */
		uint8_t rbr;  /* 0 R- receiver buffer register */
		uint8_t ier;  /* 1 RW interrupt enable register */
		uint16_t dl;  /* 0/1 RW divisor latch (if DLAB = 1) */
		uint8_t iir;  /* 2 R- interrupt identification register */
		uint8_t fcr;
		uint8_t lcr;  /* 3 RW line control register (bit 7: DLAB) */
		uint8_t mcr;  /* 4 RW modem control register */
		uint8_t lsr;  /* 5 R- line status register */
		uint8_t msr;  /* 6 R- modem status register */
		uint8_t scr;  /* 7 RW scratch register */
	} m_regs;
private:
	uint8_t m_int_pending;

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
	ins8250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ns16450_device : public ins8250_uart_device
{
public:
	ns16450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ns16550_device : public ins8250_uart_device
{
public:
	ns16550_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void set_fcr(uint8_t data) override;
	virtual void push_tx(uint8_t data) override;
	virtual uint8_t pop_rx() override;
private:
	void set_timer() { m_timeout->adjust(attotime::from_hz((clock()*4*8)/(m_regs.dl*16))); }
	int m_rintlvl;
	uint8_t m_rfifo[16];
	uint8_t m_tfifo[16];
	int m_rhead, m_rtail, m_rnum;
	int m_thead, m_ttail;
	emu_timer *m_timeout;
};

class pc16552_device : public device_t
{
public:
	pc16552_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(read) { return ((offset & 8) ? m_chan1 : m_chan0)->ins8250_r(space, offset & 7, mem_mask); }
	DECLARE_WRITE8_MEMBER(write) { ((offset & 8) ? m_chan1 : m_chan0)->ins8250_w(space, offset & 7, data, mem_mask); }

protected:
	virtual void device_start() override;

private:
	ns16550_device *m_chan0;
	ns16550_device *m_chan1;
};

DECLARE_DEVICE_TYPE(PC16552D, pc16552_device)
DECLARE_DEVICE_TYPE(INS8250,  ins8250_device)
DECLARE_DEVICE_TYPE(NS16450,  ns16450_device)
DECLARE_DEVICE_TYPE(NS16550,  ns16550_device)

#endif // MAME_MACHINE_INS8250_H
