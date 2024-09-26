// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC DC305 Printer Controller

****************************************************************************
                            _____   _____
                   VSS   1 |*    \_/     | 40  A0
                   INT   2 |             | 39  A1
                 RESET   3 |             | 38  WR
                   CLK   4 |             | 37  RD
                   VCC   5 |             | 36  INTA
                   CH1   6 |             | 35  CS
                   CH2   7 |             | 34  D0
                 MINUS   8 |             | 33  D1
                  PLUS   9 |             | 32  D2
                  BELL  10 |             | 31  D3
                   RUN  11 |    DC305    | 30  D4
                   LF2  12 |             | 29  D5
                   LF1  13 |             | 28  D6
                   RXC  14 |             | 27  D7
                   TXC  15 |             | 26  S2
                   S11  16 |             | 25  S4
                    S9  17 |             | 24  S6
                    S7  18 |             | 23  S8
                    S5  19 |             | 22  S10
                    S3  20 |_____________| 21  S1

***************************************************************************/

#ifndef MAME_DEC_DC305_H
#define MAME_DEC_DC305_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dc305_device

class dc305_device : public device_t
{
public:
	// construction/destruction
	dc305_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto int_callback() { return m_int_callback.bind(); }
	auto rxc_callback() { return m_rxc_callback.bind(); }
	auto txc_callback() { return m_txc_callback.bind(); }

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 inta();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// callback objects
	devcb_write_line m_int_callback;
	devcb_write_line m_rxc_callback;
	devcb_write_line m_txc_callback;
};

// device type declaration
DECLARE_DEVICE_TYPE(DC305, dc305_device)

#endif // MAME_DEC_DC305_H
