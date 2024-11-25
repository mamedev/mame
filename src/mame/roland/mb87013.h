// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB87013 QD (Quick Disk) Drive Interface Adapter

****************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 40  D1
                    D3   2 |             | 39  D0
                    D4   3 |             | 38  N. C.
                    D5   4 |             | 37  RXDA
                    D6   5 |             | 36  RXCA
                    D7   6 |             | 35  N. C.
                 N. C.   7 |             | 34  TXCA
                    A0   8 |             | 33  TXDA
                    A1   9 |             | 32  INIB
                   Vss  10 |             | 31  TST
                  ISCB  11 |   MB87013   | 30  VDD
                  RSTB  12 |             | 29  OCSB
                   RDB  13 |             | 28  INIT
                   WRB  14 |             | 27  IP0
                  DTRB  15 |             | 26  WRDT
                  RTSB  16 |             | 25  WRGA
                   OP4  17 |             | 24  RDT
                   OP3  18 |             | 23  IP1
                 N. C.  19 |             | 22  IP2
                  XTAL  20 |_____________| 21  EXTAL

***************************************************************************/

#ifndef MAME_ROLAND_MB87013_H
#define MAME_ROLAND_MB87013_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb87013_device

class mb87013_device : public device_t
{
public:
	// device type constructor
	mb87013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	// configuration
	auto sio_rd_callback() { return m_sio_rd_callback.bind(); }
	auto sio_wr_callback() { return m_sio_wr_callback.bind(); }
	auto txc_callback() { return m_txc_callback.bind(); }
	auto rxc_callback() { return m_rxc_callback.bind(); }
	auto rxd_callback() { return m_rxd_callback.bind(); }
	auto dsr_callback() { return m_dsr_callback.bind(); }
	auto op4_callback() { return m_op4_callback.bind(); }

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// line write handlers
	void dtr_w(int state);
	void txd_w(int state);
	void rts_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// callback objects
	devcb_read8 m_sio_rd_callback;
	devcb_write8 m_sio_wr_callback;
	devcb_write_line m_txc_callback;
	devcb_write_line m_rxc_callback;
	devcb_write_line m_rxd_callback;
	devcb_write_line m_dsr_callback;
	devcb_write_line m_op4_callback;
};


// device type declaration
DECLARE_DEVICE_TYPE(MB87013, mb87013_device)

#endif // MAME_ROLAND_MB87013_H
