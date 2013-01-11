/**********************************************************************

    Motorola MC68901 Multi Function Peripheral emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  R/_W   1 |*    \_/     | 48  _CS
                   RS1   2 |             | 47  _DS
                   RS2   3 |             | 46  _DTACK
                   RS3   4 |             | 45  _IACK
                   RS4   5 |             | 44  D7
                   RS5   6 |             | 43  D6
                    TC   7 |             | 42  D5
                    SO   8 |             | 41  D4
                    SI   9 |             | 40  D3
                    RC  10 |             | 39  D2
                   Vcc  11 |             | 38  D1
                    NC  12 |   MC68901   | 37  D0
                   TAO  13 |   MK68901   | 36  GND
                   TBO  14 |             | 35  CLK
                   TCO  15 |             | 34  _IEI
                   TDO  16 |             | 33  _IEO
                 XTAL1  17 |             | 32  _IRQ
                 XTAL2  18 |             | 31  _RR
                   TAI  19 |             | 30  _TR
                   TBI  20 |             | 29  I7
                _RESET  21 |             | 28  I6
                    I0  22 |             | 27  I5
                    I1  23 |             | 26  I4
                    I2  24 |_____________| 25  I3

**********************************************************************/

#pragma once

#ifndef __MC68901__
#define __MC68901__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MC68901_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), MC68901, _clock)    \
	MCFG_DEVICE_CONFIG(_config)

#define MC68901_INTERFACE(name) \
	const mc68901_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc68901_interface

struct mc68901_interface
{
	int m_timer_clock;      /* timer clock */
	int m_rx_clock;         /* serial receive clock */
	int m_tx_clock;         /* serial transmit clock */

	devcb_write_line        m_out_irq_cb;

	devcb_read8             m_in_gpio_cb;
	devcb_write8            m_out_gpio_cb;

	devcb_write_line        m_out_tao_cb;
	devcb_write_line        m_out_tbo_cb;
	devcb_write_line        m_out_tco_cb;
	devcb_write_line        m_out_tdo_cb;

	devcb_read_line         m_in_si_cb;
	devcb_write_line        m_out_so_cb;
	devcb_write_line        m_out_rr_cb;
	devcb_write_line        m_out_tr_cb;
};



// ======================> mc68901_device

class mc68901_device :  public device_t,
						public mc68901_interface
{
public:
	// construction/destruction
	mc68901_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	int get_vector();

	DECLARE_WRITE_LINE_MEMBER( i0_w );
	DECLARE_WRITE_LINE_MEMBER( i1_w );
	DECLARE_WRITE_LINE_MEMBER( i2_w );
	DECLARE_WRITE_LINE_MEMBER( i3_w );
	DECLARE_WRITE_LINE_MEMBER( i4_w );
	DECLARE_WRITE_LINE_MEMBER( i5_w );
	DECLARE_WRITE_LINE_MEMBER( i6_w );
	DECLARE_WRITE_LINE_MEMBER( i7_w );

	DECLARE_WRITE_LINE_MEMBER( tai_w );
	DECLARE_WRITE_LINE_MEMBER( tbi_w );

	DECLARE_WRITE_LINE_MEMBER( rc_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void check_interrupts();
	void take_interrupt(UINT16 mask);
	void rx_buffer_full();
	void rx_error();
	void tx_buffer_empty();
	void tx_error();
	int get_parity_bit(UINT8 b);
	void serial_receive();
	void tx_disabled();
	void tx_starting();
	void tx_break();
	void tx_enabled();
	void serial_transmit();
	void timer_count(int index);
	void timer_input(int index, int value);
	void gpio_input(int bit, int state);
	void register_w(offs_t offset, UINT8 data);

private:
	static const device_timer_id TIMER_A = 0;
	static const device_timer_id TIMER_B = 1;
	static const device_timer_id TIMER_C = 2;
	static const device_timer_id TIMER_D = 3;
	static const device_timer_id TIMER_RX = 4;
	static const device_timer_id TIMER_TX = 5;

	devcb_resolved_read8        m_in_gpio_func;
	devcb_resolved_write8       m_out_gpio_func;
	devcb_resolved_read_line    m_in_si_func;
	devcb_resolved_write_line   m_out_so_func;
	devcb_resolved_write_line   m_out_tao_func;
	devcb_resolved_write_line   m_out_tbo_func;
	devcb_resolved_write_line   m_out_tco_func;
	devcb_resolved_write_line   m_out_tdo_func;
	devcb_resolved_write_line   m_out_irq_func;

	int m_device_type;                      /* device type */

	/* registers */
	UINT8 m_gpip;                           /* general purpose I/O register */
	UINT8 m_aer;                            /* active edge register */
	UINT8 m_ddr;                            /* data direction register */

	UINT16 m_ier;                           /* interrupt enable register */
	UINT16 m_ipr;                           /* interrupt pending register */
	UINT16 m_isr;                           /* interrupt in-service register */
	UINT16 m_imr;                           /* interrupt mask register */
	UINT8 m_vr;                             /* vector register */

	UINT8 m_tacr;                           /* timer A control register */
	UINT8 m_tbcr;                           /* timer B control register */
	UINT8 m_tcdcr;                          /* timers C and D control register */
	UINT8 m_tdr[4];     /* timer data registers */

	UINT8 m_scr;                            /* synchronous character register */
	UINT8 m_ucr;                            /* USART control register */
	UINT8 m_tsr;                            /* transmitter status register */
	UINT8 m_rsr;                            /* receiver status register */
	UINT8 m_udr;                            /* USART data register */

	/* counter timer state */
	UINT8 m_tmc[4];     /* timer main counters */
	int m_ti[4];            /* timer in latch */
	int m_to[4];            /* timer out latch */

	/* interrupt state */
	int m_irqlevel;                         /* interrupt level latch */

	/* serial state */
	UINT8 m_next_rsr;                       /* receiver status register latch */
	int m_rsr_read;                         /* receiver status register read flag */
	int m_rxtx_word;                        /* word length */
	int m_rxtx_start;                       /* start bits */
	int m_rxtx_stop;                        /* stop bits */

	/* receive state */
	UINT8 m_rx_buffer;                      /* receive buffer */
	int m_rx_bits;                          /* receive bit count */
	int m_rx_parity;                        /* receive parity bit */
	int m_rx_state;                         /* receive state */

	/* transmit state */
	UINT8 m_tx_buffer;                      /* transmit buffer */
	int m_tx_bits;                          /* transmit bit count */
	int m_tx_parity;                        /* transmit parity bit */
	int m_tx_state;                         /* transmit state */
	int m_xmit_state;                       /* transmitter state */

	// timers
	emu_timer *m_timer[4]; /* counter timers */
	emu_timer *m_rx_timer;                  /* receive timer */
	emu_timer *m_tx_timer;                  /* transmit timer */
};


// device type definition
extern const device_type MC68901;



#endif
