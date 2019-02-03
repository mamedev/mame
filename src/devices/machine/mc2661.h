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

#ifndef MAME_MACHINE_MC2661_H
#define MAME_MACHINE_MC2661_H

#pragma once

#include "diserial.h"


class mc2661_device :  public device_t,
						public device_serial_interface
{
public:
	// construction/destruction
	mc2661_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rxc(int clock) { m_rxc = clock; }
	void set_txc(int clock) { m_txc = clock; }

	auto txd_handler() { return m_write_txd.bind(); }
	auto rxrdy_handler() { return m_write_rxrdy.bind(); }
	auto txrdy_handler() { return m_write_txrdy.bind(); }
	auto rts_handler() { return m_write_rts.bind(); }
	auto dtr_handler() { return m_write_dtr.bind(); }
	auto txemt_dschg_handler() { return m_write_txemt_dschg.bind(); }
	auto bkdet_handler() { return m_write_bkdet.bind(); }
	auto xsync_handler() { return m_write_xsync.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( dsr_w );
	DECLARE_WRITE_LINE_MEMBER( dcd_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );

	DECLARE_READ_LINE_MEMBER( rxrdy_r );
	DECLARE_READ_LINE_MEMBER( txemt_r );

	DECLARE_WRITE_LINE_MEMBER( rx_w ) { device_serial_interface::rx_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

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

	uint8_t m_rhr;
	uint8_t m_thr;
	uint8_t m_cr;
	uint8_t m_sr;
	uint8_t m_mr[2];
	uint8_t m_sync[3];

	int m_mode_index;
	int m_sync_index;
};

DECLARE_DEVICE_TYPE(MC2661, mc2661_device)

#endif // MAME_MACHINE_MC2661_H
