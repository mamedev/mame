/**********************************************************************

    NEC uPD7201 Multiprotocol Serial Communications Controller

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   CLK   1 |*    \_/     | 40  Vcc
                _RESET   2 |             | 39  _CTSA
                 _DCDA   3 |             | 38  _RTSA
                 _RxCB   4 |             | 37  TxDA
                 _DCDB   5 |             | 36  _TxCA
                 _CTSB   6 |             | 35  _RxCA
                 _TxCB   7 |             | 34  RxDA
                  TxDB   8 |             | 33  _SYNCA
                  RxDB   9 |             | 32  _WAITA/DRQRxA
          _RTSB/_SYNCB  10 |   UPD7201   | 31  _DTRA/_HAO
        _WAITB/_DRQTxA  11 |             | 30  _PRO/DRQTxB
                    D7  12 |             | 29  _PRI/DRQRxB
                    D6  13 |             | 28  _INT
                    D5  14 |             | 27  _INTAK
                    D4  15 |             | 26  _DTRB/_HAI
                    D3  16 |             | 25  B/_A
                    D2  17 |             | 24  C/_D
                    D1  18 |             | 23  _CS
                    D0  19 |             | 22  _RD
                   Vss  20 |_____________| 21  _WR

**********************************************************************/

#pragma once

#ifndef __UPD7201__
#define __UPD7201__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD7201_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), UPD7201, _clock)    \
	MCFG_DEVICE_CONFIG(_config)

#define UPD7201_INTERFACE(name) \
	const upd7201_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd7201_interface

struct upd7201_interface
{
	devcb_write_line    m_out_int_cb;

	struct
	{
		int m_rx_clock;
		int m_tx_clock;

		devcb_write_line    m_out_drqrx_cb;
		devcb_write_line    m_out_drqtx_cb;

		devcb_read_line     m_in_rxd_cb;
		devcb_write_line    m_out_txd_cb;

		devcb_read_line     m_in_cts_cb;
		devcb_read_line     m_in_dcd_cb;
		devcb_write_line    m_out_rts_cb;
		devcb_write_line    m_out_dtr_cb;

		devcb_write_line    m_out_wait_cb;
		devcb_write_line    m_out_sync_cb;
	} m_channel[2];
};



// ======================> upd7201_device

class upd7201_device :  public device_t,
						public upd7201_interface
{
public:
	// construction/destruction
	upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( cd_ba_r );
	DECLARE_WRITE8_MEMBER( cd_ba_w );
	DECLARE_READ8_MEMBER( ba_cd_r );
	DECLARE_WRITE8_MEMBER( ba_cd_w );

	DECLARE_READ8_MEMBER( intak_r );

	DECLARE_WRITE_LINE_MEMBER( synca_w );
	DECLARE_WRITE_LINE_MEMBER( syncb_w );
	DECLARE_WRITE_LINE_MEMBER( ctsa_w );
	DECLARE_WRITE_LINE_MEMBER( ctsb_w );
	DECLARE_READ_LINE_MEMBER( dtra_r );
	DECLARE_READ_LINE_MEMBER( dtrb_r );
	DECLARE_WRITE_LINE_MEMBER( hai_w );
	DECLARE_WRITE_LINE_MEMBER( rxda_w );
	DECLARE_READ_LINE_MEMBER( txda_r );
	DECLARE_WRITE_LINE_MEMBER( rxdb_w );
	DECLARE_READ_LINE_MEMBER( txdb_r );
	DECLARE_WRITE_LINE_MEMBER( rxca_w );
	DECLARE_WRITE_LINE_MEMBER( rxcb_w );
	DECLARE_WRITE_LINE_MEMBER( txca_w );
	DECLARE_WRITE_LINE_MEMBER( txcb_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int m_param, void *ptr);

private:
	static const device_timer_id TIMER_RX_A = 0;
	static const device_timer_id TIMER_TX_A = 1;
	static const device_timer_id TIMER_RX_B = 2;
	static const device_timer_id TIMER_TX_B = 3;

	inline void receive(int channel);
	inline void transmit(int channel);

	devcb_resolved_write_line       m_out_int_func;

	// timers
	emu_timer *m_rx_a_timer;
	emu_timer *m_tx_a_timer;
	emu_timer *m_rx_b_timer;
	emu_timer *m_tx_b_timer;
};


// device type definition
extern const device_type UPD7201;



#endif
