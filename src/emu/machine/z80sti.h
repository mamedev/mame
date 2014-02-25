// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   TAO   1 |*    \_/     | 40  Vcc
                   TBO   2 |             | 39  RC
                   TCO   3 |             | 38  SI
                   TDO   4 |             | 37  SO
                   TCK   5 |             | 36  TC
                   _M1   6 |             | 35  A0
                  _RES   7 |             | 34  A1
                    I0   8 |             | 33  A2
                    I1   9 |             | 32  A3
                    I2  10 |   MK3801    | 31  _WR
                    I3  11 |   Z80-STI   | 30  _CE
                    I4  12 |             | 29  _RD
                    I5  13 |             | 28  D7
                    I6  14 |             | 27  D6
                    I7  15 |             | 26  D5
                   IEI  16 |             | 25  D4
                  _INT  17 |             | 24  D3
                   IEO  18 |             | 23  D2
                 _IORQ  19 |             | 22  D1
                   Vss  20 |_____________| 21  D0

**********************************************************************/

#ifndef __Z80STI__
#define __Z80STI__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80STI_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), Z80STI, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define Z80STI_INTERFACE(name) \
	const z80sti_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80sti_interface

struct z80sti_interface
{
	int m_rx_clock;         // serial receive clock
	int m_tx_clock;         // serial transmit clock

	// this gets called on each change of the _INT pin (pin 17)
	devcb_write_line        m_out_int_cb;

	// this is called on each read of the GPIO pins
	devcb_read8             m_in_gpio_cb;

	// this is called on each write of the GPIO pins
	devcb_write8            m_out_gpio_cb;

	// this gets called for each change of the SO pin (pin 37)
	devcb_write_line        m_out_so_cb;

	// this gets called for each change of the TAO pin (pin 1)
	devcb_write_line        m_out_tao_cb;

	// this gets called for each change of the TBO pin (pin 2)
	devcb_write_line        m_out_tbo_cb;

	// this gets called for each change of the TCO pin (pin 3)
	devcb_write_line        m_out_tco_cb;

	// this gets called for each change of the TDO pin (pin 4)
	devcb_write_line        m_out_tdo_cb;
};


// ======================> z80sti_device

class z80sti_device :   public device_t,
						public device_serial_interface,
						public device_z80daisy_interface,
						public z80sti_interface
{
public:
	// construction/destruction
	z80sti_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( i0_w );
	DECLARE_WRITE_LINE_MEMBER( i1_w );
	DECLARE_WRITE_LINE_MEMBER( i2_w );
	DECLARE_WRITE_LINE_MEMBER( i3_w );
	DECLARE_WRITE_LINE_MEMBER( i4_w );
	DECLARE_WRITE_LINE_MEMBER( i5_w );
	DECLARE_WRITE_LINE_MEMBER( i6_w );
	DECLARE_WRITE_LINE_MEMBER( i7_w );

	DECLARE_WRITE_LINE_MEMBER( tc_w );
	DECLARE_WRITE_LINE_MEMBER( rc_w );

private:
	enum
	{
		TIMER_A = 0,
		TIMER_B,
		TIMER_C,
		TIMER_D
	};

	enum
	{
		REGISTER_IR = 0,
		REGISTER_GPIP,
		REGISTER_IPRB,
		REGISTER_IPRA,
		REGISTER_ISRB,
		REGISTER_ISRA,
		REGISTER_IMRB,
		REGISTER_IMRA,
		REGISTER_PVR,
		REGISTER_TABC,
		REGISTER_TBDR,
		REGISTER_TADR,
		REGISTER_UCR,
		REGISTER_RSR,
		REGISTER_TSR,
		REGISTER_UDR
	};

	enum
	{
		REGISTER_IR_SCR = 0,
		REGISTER_IR_TDDR,
		REGISTER_IR_TCDR,
		REGISTER_IR_AER,
		REGISTER_IR_IERB,
		REGISTER_IR_IERA,
		REGISTER_IR_DDR,
		REGISTER_IR_TCDC
	};

	enum
	{
		IR_P0 = 0,
		IR_P1,
		IR_P2,
		IR_P3,
		IR_TD,
		IR_TC,
		IR_P4,
		IR_P5,
		IR_TB,
		IR_XE,
		IR_XB,
		IR_RE,
		IR_RB,
		IR_TA,
		IR_P6,
		IR_P7
	};

	static const int INT_LEVEL_GPIP[];
	static const int INT_LEVEL_TIMER[];
	static const UINT8 INT_VECTOR[];
	static const int PRESCALER[];

	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	void check_interrupts();
	void take_interrupt(int level);
	void timer_count(int index);
	void gpip_input(int bit, int state);

	// device callbacks
	devcb_resolved_read8                m_in_gpio_func;
	devcb_resolved_write8               m_out_gpio_func;
	devcb_resolved_write_line           m_out_so_func;
	devcb_resolved_write_line           m_out_timer_func[4];
	devcb_resolved_write_line           m_out_int_func;

	// I/O state
	UINT8 m_gpip;                       // general purpose I/O register
	UINT8 m_aer;                        // active edge register
	UINT8 m_ddr;                        // data direction register

	// interrupt state
	UINT16 m_ier;                       // interrupt enable register
	UINT16 m_ipr;                       // interrupt pending register
	UINT16 m_isr;                       // interrupt in-service register
	UINT16 m_imr;                       // interrupt mask register
	UINT8 m_pvr;                        // interrupt vector register
	int m_int_state[16];                // interrupt state

	// timer state
	UINT8 m_tabc;                       // timer A/B control register
	UINT8 m_tcdc;                       // timer C/D control register
	UINT8 m_tdr[4];                     // timer data registers
	UINT8 m_tmc[4];                     // timer main counters
	int m_to[4];                        // timer out latch

	// serial state
	UINT8 m_scr;                        // synchronous character register
	UINT8 m_ucr;                        // USART control register
	UINT8 m_tsr;                        // transmitter status register
	UINT8 m_rsr;                        // receiver status register
	UINT8 m_udr;                        // USART data register

	// timers
	emu_timer *m_timer[4];              // counter timers
};


// device type definition
extern const device_type Z80STI;



#endif
