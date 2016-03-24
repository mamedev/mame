// license:BSD-3-Clause
// copyright-holders:smf, Carl
/**********************************************************************

    8250 UART interface and emulation

**********************************************************************/

#ifndef __INS8250_H_
#define __INS8250_H_

#include "emu.h"

/***************************************************************************
    CLASS DEFINITIONS
***************************************************************************/

class ins8250_uart_device : public device_t,
							public device_serial_interface
{
public:
	ins8250_uart_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock, const char *shortname);

	template<class _Object> static devcb_base &set_out_tx_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_tx_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dtr_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_dtr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rts_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_rts_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_out1_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_out1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_out2_callback(device_t &device, _Object object) { return downcast<ins8250_uart_device &>(device).m_out_out2_cb.set_callback(object); }

	DECLARE_WRITE8_MEMBER( ins8250_w );
	DECLARE_READ8_MEMBER( ins8250_r );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( ri_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( rx_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void tra_callback() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void set_fcr(UINT8 data) {}
	virtual void push_tx(UINT8 data) {}
	virtual UINT8 pop_rx() { return 0; }

	void trigger_int(int flag);
	void clear_int(int flag);

	enum {
			TYPE_INS8250 = 0,
			TYPE_INS8250A,
			TYPE_NS16450,
			TYPE_NS16550,
			TYPE_NS16550A
	};
	int m_device_type;
	struct {
		UINT8 thr;  /* 0 -W transmitter holding register */
		UINT8 rbr;  /* 0 R- receiver buffer register */
		UINT8 ier;  /* 1 RW interrupt enable register */
		UINT16 dl;  /* 0/1 RW divisor latch (if DLAB = 1) */
		UINT8 iir;  /* 2 R- interrupt identification register */
		UINT8 fcr;
		UINT8 lcr;  /* 3 RW line control register (bit 7: DLAB) */
		UINT8 mcr;  /* 4 RW modem control register */
		UINT8 lsr;  /* 5 R- line status register */
		UINT8 msr;  /* 6 R- modem status register */
		UINT8 scr;  /* 7 RW scratch register */
	} m_regs;
private:
	UINT8 m_int_pending;

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
	ins8250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ns16450_device : public ins8250_uart_device
{
public:
	ns16450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ns16550_device : public ins8250_uart_device
{
public:
	ns16550_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void rcv_complete() override;
	virtual void tra_complete() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void set_fcr(UINT8 data) override;
	virtual void push_tx(UINT8 data) override;
	virtual UINT8 pop_rx() override;
private:
	void set_timer() { m_timeout->adjust(attotime::from_hz((clock()*4*8)/(m_regs.dl*16))); }
	int m_rintlvl;
	UINT8 m_rfifo[16];
	UINT8 m_tfifo[16];
	int m_rhead, m_rtail, m_rnum;
	int m_thead, m_ttail;
	emu_timer *m_timeout;
};

class pc16552_device : public device_t
{
public:
	pc16552_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read) { return ((offset & 8) ? m_chan1 : m_chan0)->ins8250_r(space, offset & 7, mem_mask); }
	DECLARE_WRITE8_MEMBER(write) { ((offset & 8) ? m_chan1 : m_chan0)->ins8250_w(space, offset & 7, data, mem_mask); }

protected:
	virtual void device_start() override;

private:
	ns16550_device *m_chan0;
	ns16550_device *m_chan1;
};

extern const device_type PC16552D;
extern const device_type INS8250;
extern const device_type NS16450;
extern const device_type NS16550;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_INS8250_OUT_TX_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_tx_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8250_OUT_DTR_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_dtr_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8250_OUT_RTS_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_rts_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8250_OUT_INT_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8250_OUT_OUT1_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_out1_callback(*device, DEVCB_##_devcb);

#define MCFG_INS8250_OUT_OUT2_CB(_devcb) \
	devcb = &ins8250_uart_device::set_out_out2_callback(*device, DEVCB_##_devcb);

#endif
