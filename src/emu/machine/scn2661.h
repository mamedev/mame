/***************************************************************************

    Philips SCN2661 Enhanced Programmable Communications Interface emulation

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 28  D1
                    D3   2 |             | 27  D0
                   RxD   3 |             | 26  Vcc
                   GND   4 |             | 25  _RxC/BKDET
                    D4   5 |             | 24  _DTR
                    D5   6 |             | 23  _RTS
                    D6   7 |   SCN2661   | 22  _DSR
                    D7   8 |             | 21  RESET
            _TxC/XSYNC   9 |             | 20  BRCLK
                    A1  10 |             | 19  TxD
                   _CE  11 |             | 18  _TxEMT/DSCHG
                    A0  12 |             | 17  _CTS
                  _R/W  13 |             | 16  _DCD
                _RxRDY  14 |_____________| 15  _TxRDY

***************************************************************************/

#pragma once

#ifndef __SCN2661__
#define __SCN2661__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SCN2661_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, SCN2661, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define SCN2661_INTERFACE(_name) \
	const scn2661_interface (_name) =



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> scn2661_interface

struct scn2661_interface
{
	int m_rxc;
	int m_txc;

	devcb_read_line		m_in_rxd_cb;
	devcb_write_line	m_out_txd_cb;

	devcb_write_line	m_out_rxrdy_cb;
	devcb_write_line	m_out_txrdy_cb;
	devcb_write_line	m_out_rts_cb;
	devcb_write_line	m_out_dtr_cb;
	devcb_write_line	m_out_txemt_dschg_cb;
	devcb_write_line	m_out_bkdet_cb;
	devcb_write_line	m_out_xsync_cb;
};


// ======================> scn2661_device

class scn2661_device :  public device_t,
					    public device_serial_interface,
					    public scn2661_interface
{
public:
	// construction/destruction
	scn2661_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( txc_w );

	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );

	DECLARE_READ_LINE_MEMBER( rxrdy_r );
	DECLARE_READ_LINE_MEMBER( txemt_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_serial_interface overrides
	virtual void input_callback(UINT8 state);

private:
	inline void receive();
	inline void transmit();

	enum 
	{
		TIMER_RX,
		TIMER_TX
	};

	devcb_resolved_read_line	m_in_rxd_func;
	devcb_resolved_write_line	m_out_txd_func;

	devcb_resolved_write_line	m_out_rxrdy_func;
	devcb_resolved_write_line	m_out_txrdy_func;
	devcb_resolved_write_line	m_out_rts_func;
	devcb_resolved_write_line	m_out_dtr_func;
	devcb_resolved_write_line	m_out_txemt_dschg_func;
	devcb_resolved_write_line	m_out_bkdet_func;
	devcb_resolved_write_line	m_out_xsync_func;

	UINT8 m_rhr;
	UINT8 m_thr;
	UINT8 m_cr;
	UINT8 m_sr;
	UINT8 m_mr1;
	UINT8 m_mr2;
	UINT8 m_syn1;
	UINT8 m_syn2;
	UINT8 m_dle;

	int m_mr_index;
	int m_sync_index;

	// timers
	emu_timer *m_rx_timer;
	emu_timer *m_tx_timer;
};


// device type definition
extern const device_type SCN2661;



#endif
