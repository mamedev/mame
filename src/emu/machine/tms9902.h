// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    TMS9902 Asynchronous Communication Controller
    See tms9902.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TMS9902_H__
#define __TMS9902_H__

#include "emu.h"

// Serial control protocol values
#define TYPE_TMS9902 0x01

// Configuration (output only)
#define CONFIG   0x80
#define RATERECV 0x70
#define RATEXMIT 0x60
#define DATABITS 0x50
#define STOPBITS 0x40
#define PARITY   0x30

// Exceptional states (BRK: both directions; FRMERR/PARERR: input only)
#define EXCEPT 0x40
#define BRK    0x02
#define FRMERR 0x04
#define PARERR 0x06

// Line states (RTS, DTR: output; CTS, DSR, RI, DCD: input)
#define LINES 0x00
#define RTS 0x20
#define CTS 0x10
#define DSR 0x08
#define DCD 0x04
#define DTR 0x02
#define RI  0x01

extern const device_type TMS9902;

struct tms9902_interface
{
	devcb_write_line    int_callback;
	devcb_write_line    rcv_callback;
	devcb_write8        xmit_callback;
	devcb_write8        ctrl_callback;
};

class tms9902_device : public device_t
{
public:
	tms9902_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void    set_clock(bool state);

	void    rcv_cts(line_state state);
	void    rcv_dsr(line_state state);
	void    rcv_data(UINT8 data);
	void    rcv_break(bool value);
	void    rcv_framing_error();
	void    rcv_parity_error();

	double  get_baudpoll();

	int     get_config_value();

	DECLARE_READ8_MEMBER( cruread );
	DECLARE_WRITE8_MEMBER( cruwrite );

protected:
	virtual void    device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void    device_start();
	virtual void    device_reset();
	virtual void    device_stop();

private:
	void    field_interrupts();
	void    reload_interval_timer();
	void    send_break(bool state);
	void    set_receive_data_rate();
	void    set_transmit_data_rate();
	void    set_stop_bits();
	void    set_data_bits();
	void    set_parity();
	void    transmit_line_state();
	void    set_rts(line_state state);
	void    initiate_transmit();
	void    reset_uart();

	devcb_resolved_write_line       int_callback;
	devcb_resolved_write_line       rcv_callback;
	devcb_resolved_write8           xmit_callback;
	devcb_resolved_write8           ctrl_callback;      // needs to be used with get_config_value

	// tms9902 clock rate (PHI* pin, normally connected to TMS9900 Phi3*)
	// Official range is 2MHz-3.3MHz.  Some tms9902s were sold as "MP9214", and
	// were tested for speeds up to 4MHz, provided the clk4m control bit is set.
	// (warning: 3MHz on a tms9900 is equivalent to 12MHz on a tms9995 or tms99000)
	double  m_clock_rate;

	/* Modes */
	bool    m_LDCTRL;       // Load control register
	bool    m_LDIR;         // Load interval register
	bool    m_LRDR;         // Load receive data register
	bool    m_LXDR;         // Load transmit data register
	bool    m_TSTMD;            // Test mode

	/* output pin */
	bool    m_RTSON;            // RTS-on request

	/* transmitter registers */
	bool    m_BRKON;            // BRK-on request
	bool    m_BRKout;       // indicates the current BRK state

	UINT8   m_XBR;          // transmit buffer register
	UINT8   m_XSR;          // transmit shift register

	/* receiver registers */
	UINT8   m_RBR;          // Receive buffer register

	/* Interrupt enable flags */
	bool    m_DSCENB;       // Data set change interrupt enable
	bool    m_RIENB;            // Receiver interrupt enable
	bool    m_XBIENB;       // Tansmit buffer interrupt enable
	bool    m_TIMENB;       // Timer interrupt enable

	/*
	    Rate registers. The receive bit rate calculates as
	    bitrate = clock1 / (2 * (8 ^ RDV8) * RDR)
	    (similarly for transmit)

	    where clock1 = clock_rate / (CLK4M? 4:3)
	*/
	UINT16  m_RDR;          // Receive data rate
	bool    m_RDV8;         // Receive data rate divider
	UINT16  m_XDR;          // Transmit data rate
	bool    m_XDV8;         // Transmit data rate divider

	/* Status flags */
	bool    m_INT;          // mirrors /INT output line, inverted
	bool    m_DSCH;         // Data set status change

	bool    m_CTSin;            // Inverted /CTS input (i.e. CTS)
	bool    m_DSRin;            // Inverted /DSR input (i.e. DSR)
	bool    m_RTSout;       // Current inverted /RTS line state (i.e. RTS)

	bool    m_TIMELP;       // Timer elapsed
	bool    m_TIMERR;       // Timer error

	bool    m_XSRE;         // Transmit shift register empty
	bool    m_XBRE;         // Transmit buffer register empty
	bool    m_RBRL;         // Receive buffer register loaded

	bool    m_RIN;          // State of the RIN pin
	bool    m_RSBD;         // Receive start bit detect
	bool    m_RFBD;         // Receive full bit detect
	bool    m_RFER;         // Receive framing error
	bool    m_ROVER;            // Receiver overflow
	bool    m_RPER;         // Receive parity error

	UINT8   m_RCL;          // Character length
	bool    m_ODDP;
	bool    m_PENB;
	UINT8   m_STOPB;
	bool    m_CLK4M;        // /PHI input divide select

	UINT8   m_TMR;      /* interval timer */

	/* clock registers */
	emu_timer *m_dectimer;          /* MESS timer, used to emulate the decrementer register */
	emu_timer *m_recvtimer;
	emu_timer *m_sendtimer;

	// This value is the ratio of data input versus the poll rate. The
	// data source should deliver data bytes at every 1/baudpoll call.
	// This is to ensure that data is delivered at a rate that is expected
	// from the emulated program.
	double  m_baudpoll;

	// Caches the last configuration setting (used with the ctrl_callback)
	int     m_last_config_value;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TMS9902_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, TMS9902, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __TMS9902_H__ */
