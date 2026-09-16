// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

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

#ifndef MAME_MACHINE_Z80STI_H
#define MAME_MACHINE_Z80STI_H

#pragma once

#include "machine/z80daisy.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z80sti_device

class z80sti_device :   public device_t,
						public device_serial_interface,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	z80sti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_int_cb() { return m_out_int_cb.bind(); }
	auto in_gpio_cb() { return m_in_gpio_cb.bind(); }
	auto out_gpio_cb() { return m_out_gpio_cb.bind(); }
	auto out_so_cb() { return m_out_so_cb.bind(); }
	auto out_tao_cb() { return m_out_tao_cb.bind(); }
	auto out_tbo_cb() { return m_out_tbo_cb.bind(); }
	auto out_tco_cb() { return m_out_tco_cb.bind(); }
	auto out_tdo_cb() { return m_out_tdo_cb.bind(); }

	void set_rx_clock(int clock) { m_rx_clock = clock; }
	void set_tx_clock(int clock) { m_tx_clock = clock; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void i0_w(int state);
	void i1_w(int state);
	void i2_w(int state);
	void i3_w(int state);
	void i4_w(int state);
	void i5_w(int state);
	void i6_w(int state);
	void i7_w(int state);

	void tc_w(int state);
	void rc_w(int state);

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
	static const uint8_t INT_VECTOR[];
	static const int PRESCALER[];

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal helpers
	void check_interrupts();
	void take_interrupt(int level);
	TIMER_CALLBACK_MEMBER(timer_count);
	void gpip_input(int bit, int state);

	// device callbacks
	devcb_write_line        m_out_int_cb;   // this gets called on each change of the _INT pin (pin 17)
	devcb_read8             m_in_gpio_cb;   // this is called on each read of the GPIO pins
	devcb_write8            m_out_gpio_cb;  // this is called on each write of the GPIO pins
	devcb_write_line        m_out_so_cb;    // this gets called for each change of the SO pin (pin 37)
	devcb_write_line        m_out_tao_cb;   // this gets called for each change of the TAO pin (pin 1)
	devcb_write_line        m_out_tbo_cb;   // this gets called for each change of the TBO pin (pin 2)
	devcb_write_line        m_out_tco_cb;   // this gets called for each change of the TCO pin (pin 3)
	devcb_write_line        m_out_tdo_cb;   // this gets called for each change of the TDO pin (pin 4)

	int m_rx_clock;                     // serial receive clock
	int m_tx_clock;                     // serial transmit clock

	// I/O state
	uint8_t m_gpip;                       // general purpose I/O register
	uint8_t m_aer;                        // active edge register
	uint8_t m_ddr;                        // data direction register

	// interrupt state
	uint16_t m_ier;                       // interrupt enable register
	uint16_t m_ipr;                       // interrupt pending register
	uint16_t m_isr;                       // interrupt in-service register
	uint16_t m_imr;                       // interrupt mask register
	uint8_t m_pvr;                        // interrupt vector register
	int m_int_state[16];                // interrupt state

	// timer state
	uint8_t m_tabc;                       // timer A/B control register
	uint8_t m_tcdc;                       // timer C/D control register
	uint8_t m_tdr[4];                     // timer data registers
	uint8_t m_tmc[4];                     // timer main counters
	int m_to[4];                        // timer out latch

	// serial state
	uint8_t m_scr;                        // synchronous character register
	uint8_t m_ucr;                        // USART control register
	uint8_t m_tsr;                        // transmitter status register
	uint8_t m_rsr;                        // receiver status register
	uint8_t m_udr;                        // USART data register

	// timers
	emu_timer *m_timer[4];              // counter timers
};


// device type definition
DECLARE_DEVICE_TYPE(Z80STI, z80sti_device)

#endif // MAME_MACHINE_Z80STI_H
