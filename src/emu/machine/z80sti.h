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
	MCFG_DEVICE_ADD((_tag), Z80STI, _clock)	\
	MCFG_DEVICE_CONFIG(_config)

#define Z80STI_INTERFACE(name) const z80sti_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80sti_interface

struct z80sti_interface
{
	int	m_rx_clock;			// serial receive clock
	int	m_tx_clock;			// serial transmit clock

	// this gets called on each change of the _INT pin (pin 17)
	devcb_write_line		m_out_int_cb;

	// this is called on each read of the GPIO pins
	devcb_read8				m_in_gpio_cb;

	// this is called on each write of the GPIO pins
	devcb_write8			m_out_gpio_cb;

	// this gets called for each read of the SI pin (pin 38)
	devcb_read_line			m_in_si_cb;

	// this gets called for each change of the SO pin (pin 37)
	devcb_write_line		m_out_so_cb;

	// this gets called for each change of the TAO pin (pin 1)
	devcb_write_line		m_out_tao_cb;

	// this gets called for each change of the TBO pin (pin 2)
	devcb_write_line		m_out_tbo_cb;

	// this gets called for each change of the TCO pin (pin 3)
	devcb_write_line		m_out_tco_cb;

	// this gets called for each change of the TDO pin (pin 4)
	devcb_write_line		m_out_tdo_cb;
};



// ======================> z80sti_device

class z80sti_device :	public device_t,
						public device_z80daisy_interface,
						public z80sti_interface
{
public:
	// construction/destruction
	z80sti_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O accessors
	UINT8 read(offs_t offset);
	void write(offs_t offset, UINT8 data);

	// communication I/O
	void serial_receive();
	void serial_transmit();
	void gpip_input(int bit, int state);

private:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	void check_interrupts();
	void take_interrupt(int level);

	void timer_count(int index);

	static TIMER_CALLBACK( static_rx_tick ) { reinterpret_cast<z80sti_device *>(ptr)->serial_receive(); }
	static TIMER_CALLBACK( static_tx_tick ) { reinterpret_cast<z80sti_device *>(ptr)->serial_transmit(); }

	static TIMER_CALLBACK( static_timer_count ) { reinterpret_cast<z80sti_device *>(ptr)->timer_count(param); }

	// device callbacks
	devcb_resolved_read8				m_in_gpio_func;
	devcb_resolved_write8				m_out_gpio_func;
	devcb_resolved_read_line			m_in_si_func;
	devcb_resolved_write_line			m_out_so_func;
	devcb_resolved_write_line			m_out_timer_func[4];
	devcb_resolved_write_line			m_out_int_func;

	// I/O state
	UINT8 m_gpip;						// general purpose I/O register
	UINT8 m_aer;						// active edge register
	UINT8 m_ddr;						// data direction register

	// interrupt state
	UINT16 m_ier;						// interrupt enable register
	UINT16 m_ipr;						// interrupt pending register
	UINT16 m_isr;						// interrupt in-service register
	UINT16 m_imr;						// interrupt mask register
	UINT8 m_pvr;						// interrupt vector register
	int m_int_state[16];				// interrupt state

	// timer state
	UINT8 m_tabc;						// timer A/B control register
	UINT8 m_tcdc;						// timer C/D control register
	UINT8 m_tdr[4];						// timer data registers
	UINT8 m_tmc[4];						// timer main counters
	int m_to[4];						// timer out latch

	// serial state
	UINT8 m_scr;						// synchronous character register
	UINT8 m_ucr;						// USART control register
	UINT8 m_tsr;						// transmitter status register
	UINT8 m_rsr;						// receiver status register
	UINT8 m_udr;						// USART data register

	// timers
	emu_timer *m_timer[4];				// counter timers
	emu_timer *m_rx_timer;				// serial receive timer
	emu_timer *m_tx_timer;				// serial transmit timer
};


// device type definition
extern const device_type Z80STI;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// register access
DECLARE_READ8_DEVICE_HANDLER( z80sti_r );
DECLARE_WRITE8_DEVICE_HANDLER( z80sti_w );

// receive clock
WRITE_LINE_DEVICE_HANDLER( z80sti_rc_w );

// transmit clock
WRITE_LINE_DEVICE_HANDLER( z80sti_tc_w );

// GPIP input lines
WRITE_LINE_DEVICE_HANDLER( z80sti_i0_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i1_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i2_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i3_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i4_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i5_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i6_w );
WRITE_LINE_DEVICE_HANDLER( z80sti_i7_w );


#endif
