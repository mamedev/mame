// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MC6852 Synchronous Serial Data Adapter emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 24  _CTS
               Rx DATA   2 |             | 23  _DCD
                Rx CLK   3 |             | 22  D0
                Tx CLK   4 |             | 21  D1
               SM/_DTR   5 |             | 20  D2
               Tx DATA   6 |   MC6852    | 19  D3
                  _IRQ   7 |             | 18  D4
                   TUF   8 |             | 17  D5
                _RESET   9 |             | 16  D6
                   _CS   9 |             | 15  D7
                    RS   9 |             | 14  E
                   Vcc  10 |_____________| 13  R/_W

**********************************************************************/

#pragma once

#ifndef __MC6852__
#define __MC6852__

#include "emu.h"
#include <queue>



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MC6852_RX_CLOCK(_clock) \
	mc6852_device::set_rx_clock(*device, _clock);

#define MCFG_MC6852_TX_CLOCK(_clock) \
	mc6852_device::set_tx_clock(*device, _clock);

#define MCFG_MC6852_TX_DATA_CALLBACK(_write) \
	devcb = &mc6852_device::set_tx_data_wr_callback(*device, DEVCB_##_write);

#define MCFG_MC6852_IRQ_CALLBACK(_write) \
	devcb = &mc6852_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_MC6852_SM_DTR_CALLBACK(_write) \
	devcb = &mc6852_device::set_sm_dtr_wr_callback(*device, DEVCB_##_write);

#define MCFG_MC6852_TUF_CALLBACK(_write) \
	devcb = &mc6852_device::set_tuf_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc6852_device

class mc6852_device :   public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	mc6852_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_rx_clock(device_t &device, int clock) { downcast<mc6852_device &>(device).m_rx_clock = clock; }
	static void set_tx_clock(device_t &device, int clock) { downcast<mc6852_device &>(device).m_tx_clock = clock; }
	template<class _Object> static devcb_base &set_tx_data_wr_callback(device_t &device, _Object object) { return downcast<mc6852_device &>(device).m_write_tx_data.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<mc6852_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_sm_dtr_wr_callback(device_t &device, _Object object) { return downcast<mc6852_device &>(device).m_write_sm_dtr.set_callback(object); }
	template<class _Object> static devcb_base &set_tuf_wr_callback(device_t &device, _Object object) { return downcast<mc6852_device &>(device).m_write_tuf.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( rx_data_w ) { device_serial_interface::rx_w(state); }
	DECLARE_WRITE_LINE_MEMBER( rx_clk_w ) { rx_clock_w(state); }
	DECLARE_WRITE_LINE_MEMBER( tx_clk_w ) { tx_clock_w(state); }
	DECLARE_WRITE_LINE_MEMBER( cts_w ) { m_cts = state; }
	DECLARE_WRITE_LINE_MEMBER( dcd_w ) { m_dcd = state; }

	DECLARE_READ_LINE_MEMBER( sm_dtr_r ) { return m_sm_dtr; }
	DECLARE_READ_LINE_MEMBER( tuf_r ) { return m_tuf; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int m_param, void *ptr);

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();

private:
	devcb_write_line       m_write_tx_data;
	devcb_write_line       m_write_irq;
	devcb_write_line       m_write_sm_dtr;
	devcb_write_line       m_write_tuf;

	UINT8 m_status;         // status register
	UINT8 m_cr[3];          // control registers
	UINT8 m_scr;            // sync code register
	UINT8 m_tdr;            // transmit data register
	UINT8 m_tsr;            // transmit shift register
	UINT8 m_rdr;            // receive data register
	UINT8 m_rsr;            // receive shift register

	std::queue<UINT8> m_rx_fifo;
	std::queue<UINT8> m_tx_fifo;

	int m_rx_clock;
	int m_tx_clock;
	int m_cts;              // clear to send
	int m_dcd;              // data carrier detect
	int m_sm_dtr;           // sync match/data terminal ready
	int m_tuf;              // transmitter underflow
};


// device type definition
extern const device_type MC6852;



#endif
