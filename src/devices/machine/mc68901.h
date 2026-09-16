// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Motorola MC68901 Multi Function Peripheral emulation

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

#ifndef MAME_MACHINE_MC68901_H
#define MAME_MACHINE_MC68901_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> mc68901_device

class mc68901_device :  public device_t
{
public:
	// construction/destruction
	mc68901_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_timer_clock(int timer_clock) { m_timer_clock = timer_clock; }
	void set_timer_clock(const XTAL &xtal) { set_timer_clock(xtal.value()); }

	auto out_irq_cb() { return m_out_irq_cb.bind(); }
	auto out_gpio_cb() { return m_out_gpio_cb.bind(); }
	auto out_tao_cb() { return m_out_tao_cb.bind(); }
	auto out_tbo_cb() { return m_out_tbo_cb.bind(); }
	auto out_tco_cb() { return m_out_tco_cb.bind(); }
	auto out_tdo_cb() { return m_out_tdo_cb.bind(); }
	auto out_so_cb() { return m_out_so_cb.bind(); }
	//auto out_rr_cb() { return m_out_rr_cb.bind(); }
	//auto out_tr_cb() { return m_out_tr_cb.bind(); }
	auto iack_chain_cb() { return m_iack_chain_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 get_vector();

	void i0_w(int state);
	void i1_w(int state);
	void i2_w(int state);
	void i3_w(int state);
	void i4_w(int state);
	void i5_w(int state);
	void i6_w(int state);
	void i7_w(int state);

	void tai_w(int state);
	void tbi_w(int state);

	void si_w(int state);
	void rc_w(int state);
	void tc_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void check_interrupts();
	void take_interrupt(u16 mask);
	void tx_buffer_empty();
	void tx_error();
	void rx_buffer_full();
	void rx_error();
	TIMER_CALLBACK_MEMBER(timer_count);
	void timer_input(int index, int value);
	void gpio_input(int bit, int state);
	void gpio_output();

	void set_so(bool state);
	void rx_frame_start();
	void rx_sync_found();
	void rx_async_frame_complete();
	void rx_sync_frame_complete();
	void rx_clock(bool si);
	void tx_frame_load(u8 data);
	void tx_clock();

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
		REGISTER_GPIP = 0,
		REGISTER_AER,
		REGISTER_DDR,
		REGISTER_IERA,
		REGISTER_IERB,
		REGISTER_IPRA,
		REGISTER_IPRB,
		REGISTER_ISRA,
		REGISTER_ISRB,
		REGISTER_IMRA,
		REGISTER_IMRB,
		REGISTER_VR,
		REGISTER_TACR,
		REGISTER_TBCR,
		REGISTER_TCDCR,
		REGISTER_TADR,
		REGISTER_TBDR,
		REGISTER_TCDR,
		REGISTER_TDDR,
		REGISTER_SCR,
		REGISTER_UCR,
		REGISTER_RSR,
		REGISTER_TSR,
		REGISTER_UDR
	};

	enum
	{
		INT_GPI0 = 0,
		INT_GPI1,
		INT_GPI2,
		INT_GPI3,
		INT_TIMER_D,
		INT_TIMER_C,
		INT_GPI4,
		INT_GPI5,
		INT_TIMER_B,
		INT_XMIT_ERROR,
		INT_XMIT_BUFFER_EMPTY,
		INT_RCV_ERROR,
		INT_RCV_BUFFER_FULL,
		INT_TIMER_A,
		INT_GPI6,
		INT_GPI7
	};

	enum
	{
		GPIP_0 = 0,
		GPIP_1,
		GPIP_2,
		GPIP_3,
		GPIP_4,
		GPIP_5,
		GPIP_6,
		GPIP_7
	};

	enum
	{
		SERIAL_START = 0,
		SERIAL_DATA,
		SERIAL_PARITY,
		SERIAL_STOP
	};

	enum
	{
		XMIT_OFF = 0,
		XMIT_STARTING,
		XMIT_ON,
		XMIT_BREAK,
		XMIT_STOPPING
	};

	static const u16 INT_MASK_GPIO[];
	static const u16 INT_MASK_TIMER[];
	static const int GPIO_TIMER[];
	static const int PRESCALER[];

	int m_timer_clock;      /* timer clock */

	devcb_write_line        m_out_irq_cb;

	devcb_write8            m_out_gpio_cb;

	devcb_write_line        m_out_tao_cb;
	devcb_write_line        m_out_tbo_cb;
	devcb_write_line        m_out_tco_cb;
	devcb_write_line        m_out_tdo_cb;

	devcb_write_line        m_out_so_cb;
	//devcb_write_line        m_out_rr_cb;
	//devcb_write_line        m_out_tr_cb;

	devcb_read8             m_iack_chain_cb;

	// registers
	u8 m_gpip;                      // general purpose I/O register
	u8 m_aer;                       // active edge register
	u8 m_ddr;                       // data direction register

	u16 m_ier;                      // interrupt enable register
	u16 m_ipr;                      // interrupt pending register
	u16 m_isr;                      // interrupt in-service register
	u16 m_imr;                      // interrupt mask register
	u8 m_vr;                        // vector register

	u8 m_tacr;                      // timer A control register
	u8 m_tbcr;                      // timer B control register
	u8 m_tcdcr;                     // timers C and D control register
	u8 m_tdr[4];                    // timer data registers

	u8 m_scr;                       // synchronous character register
	bool m_scr_parity;              // parity of sync character
	u8 m_ucr;                       // USART control register
	u8 m_tsr;                       // transmitter status register
	u8 m_rsr;                       // receiver status register
	u8 m_transmit_buffer;           // USART data register
	u8 m_receive_buffer;
	u8 m_gpio_input;
	u8 m_gpio_output;

	// counter timer state
	u8 m_tmc[4];                    // timer main counters
	int m_ti[4];                    // timer in latch
	int m_to[4];                    // timer out latch

	// serial receiver state
	u16 m_rframe;                   // receiver frame shift register
	u8 m_rclk;                      // receiver clock counter
	u8 m_rbits;                     // receiver bit counter
	u8 m_si_scan;                   // receiver bitstream scan
	u8 m_next_rsr;                  // receiver status register latch
	bool m_rc;                      // receiver clock input
	bool m_si;                      // serial data input
	bool m_last_si;                 // synchronized serial data input
	bool m_rparity;                 // receiver data parity

	// serial transmitter state
	u16 m_osr;                      // output shift register
	u8 m_tclk;                      // transmit clock counter
	u8 m_tbits;                     // transmit bit counter
	bool m_tc;                      // transmit clock input
	bool m_so;                      // serial data output
	bool m_tparity;                 // transmit data transmit
	bool m_underrun;                // underrun preset time

	// timers
	emu_timer *m_timer[4];          // counter timers
};


// device type definition
DECLARE_DEVICE_TYPE(MC68901, mc68901_device)

#endif // MAME_MACHINE_MC68901_H
