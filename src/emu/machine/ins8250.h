/**********************************************************************

    8250 UART interface and emulation

**********************************************************************/

#ifndef __INS8250_H_
#define __INS8250_H_

#include "emu.h"

/***************************************************************************
    CLASS DEFINITIONS
***************************************************************************/
struct ins8250_interface
{
	devcb_write_line    m_out_tx_cb;
	devcb_write_line    m_out_dtr_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_write_line    m_out_int_cb;
	devcb_write_line    m_out_out1_cb;
	devcb_write_line    m_out_out2_cb;
};

class ins8250_uart_device : public device_t,
							public device_serial_interface,
							public ins8250_interface
{
public:
	ins8250_uart_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE8_MEMBER( ins8250_w );
	DECLARE_READ8_MEMBER( ins8250_r );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( ri_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );
	DECLARE_WRITE_LINE_MEMBER( rx_w ) { check_for_start(state); }
	void input_callback(UINT8 state) { m_input_state = state; }

protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();
	virtual void rcv_complete();
	virtual void tra_complete();
	virtual void tra_callback();

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
			TYPE_NS16550A,
			TYPE_PC16550D,
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

	devcb_resolved_write_line   m_out_tx_func;
	devcb_resolved_write_line   m_out_dtr_func;
	devcb_resolved_write_line   m_out_rts_func;
	devcb_resolved_write_line   m_out_int_func;
	devcb_resolved_write_line   m_out_out1_func;
	devcb_resolved_write_line   m_out_out2_func;

	void update_interrupt();
	void update_clock();
	void update_msr(int bit, UINT8 state);
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
	virtual void device_start() { m_timeout = timer_alloc(); ins8250_uart_device::device_start(); }
	virtual void device_reset();
	virtual void rcv_complete();
	virtual void tra_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void set_fcr(UINT8 data);
	virtual void push_tx(UINT8 data);
	virtual UINT8 pop_rx();
private:
	void set_timer() { m_timeout->adjust(attotime::from_hz((clock()*4*8)/(m_regs.dl*16))); }
	int m_rintlvl;
	UINT8 m_rfifo[16];
	UINT8 m_tfifo[16];
	int m_rhead, m_rtail, m_rnum;
	int m_thead, m_ttail;
	emu_timer *m_timeout;
};

extern const device_type INS8250;
extern const device_type NS16450;
extern const device_type NS16550;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_INS8250_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, INS8250, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_NS16450_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, NS16450, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)


#define MCFG_NS16550_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, NS16550, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif
