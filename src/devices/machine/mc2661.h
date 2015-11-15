// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Motorola MC2661/MC68661 Enhanced Programmable Communications Interface

****************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 28  D1
                    D3   2 |             | 27  D0
                   RxD   3 |             | 26  Vcc
                   GND   4 |             | 25  _RxC/BKDET
                    D4   5 |             | 24  _DTR
                    D5   6 |             | 23  _RTS
                    D6   7 |   MC2661    | 22  _DSR
                    D7   8 |   MC68661   | 21  RESET
            _TxC/XSYNC   9 |             | 20  BRCLK
                    A1  10 |             | 19  TxD
                   _CE  11 |             | 18  _TxEMT/DSCHG
                    A0  12 |             | 17  _CTS
                  _R/W  13 |             | 16  _DCD
                _RxRDY  14 |_____________| 15  _TxRDY

***************************************************************************/

#pragma once

#ifndef __MC2661__
#define __MC2661__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MC2661_RXC(_clock) \
	mc2661_device::static_set_rxc(*device, _clock);

#define MCFG_MC2661_TXC(_clock) \
	mc2661_device::static_set_txc(*device, _clock);

#define MCFG_MC2661_TXD_HANDLER(_write) \
	devcb = &mc2661_device::set_txd_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_RXRDY_HANDLER(_write) \
	devcb = &mc2661_device::set_rxrdy_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_TXRDY_HANDLER(_write) \
	devcb = &mc2661_device::set_txrdy_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_RTS_HANDLER(_write) \
	devcb = &mc2661_device::set_rts_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_DTR_HANDLER(_write) \
	devcb = &mc2661_device::set_dtr_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_TXEMT_DSCHG_HANDLER(_write) \
	devcb = &mc2661_device::set_txemt_dschg_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_BKDET_HANDLER(_write) \
	devcb = &mc2661_device::set_bkdet_callback(*device, DEVCB_##_write);

#define MCFG_MC2661_XSYNC_HANDLER(_write) \
	devcb = &mc2661_device::set_xsync_callback(*device, DEVCB_##_write);



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> mc2661_device

class mc2661_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	mc2661_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_rxc(device_t &device, int clock) { downcast<mc2661_device &>(device).m_rxc = clock; }
	static void static_set_txc(device_t &device, int clock) { downcast<mc2661_device &>(device).m_txc = clock; }

	template<class _Object> static devcb_base &set_txd_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_txd.set_callback(object); }
	template<class _Object> static devcb_base &set_rxrdy_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_rxrdy.set_callback(object); }
	template<class _Object> static devcb_base &set_txrdy_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_txrdy.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_rts.set_callback(object); }
	template<class _Object> static devcb_base &set_dtr_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_dtr.set_callback(object); }
	template<class _Object> static devcb_base &set_txemt_dschg_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_txemt_dschg.set_callback(object); }
	template<class _Object> static devcb_base &set_bkdet_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_bkdet.set_callback(object); }
	template<class _Object> static devcb_base &set_xsync_callback(device_t &device, _Object object) { return downcast<mc2661_device &>(device).m_write_xsync.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );

	DECLARE_READ_LINE_MEMBER( rxrdy_r );
	DECLARE_READ_LINE_MEMBER( txemt_r );

	DECLARE_WRITE_LINE_MEMBER( rx_w ) { device_serial_interface::rx_w(state); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();

private:
	devcb_write_line   m_write_txd;
	devcb_write_line   m_write_rxrdy;
	devcb_write_line   m_write_txrdy;
	devcb_write_line   m_write_rts;
	devcb_write_line   m_write_dtr;
	devcb_write_line   m_write_txemt_dschg;
	devcb_write_line   m_write_bkdet;
	devcb_write_line   m_write_xsync;

	int m_rxc;
	int m_txc;

	UINT8 m_rhr;
	UINT8 m_thr;
	UINT8 m_cr;
	UINT8 m_sr;
	UINT8 m_mr[2];
	UINT8 m_sync[3];

	int m_mode_index;
	int m_sync_index;
};


// device type definition
extern const device_type MC2661;



#endif
