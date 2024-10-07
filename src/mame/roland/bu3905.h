// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************
                            _____   _____
                  XCE2   1 |*    \_/     | 42  VDD
                  XCE1   2 |             | 41  Q7
                   CE0   3 |             | 40  Q6
                    D7   4 |             | 39  Q5
                    D6   5 |             | 38  Q4
                    D5   6 |             | 37  Q3
                    D4   7 |             | 36  Q2
                    D3   8 |             | 35  Q1
                    D2   9 |             | 34  Q0
                    D1  10 |             | 33  XQ7
                    NC  11 |   BU3905S   | 32  NC
                    D0  12 |             | 31  XQ6
                    A3  13 |             | 30  XQ5
                    A2  14 |             | 29  XQ4
                    A1  15 |             | 28  XQ3
                    A0  16 |             | 27  XQ2
                   CH0  17 |             | 26  XQ1
                   CH1  18 |             | 25  XQ0
                   CH2  19 |             | 24  AXO
                   XEN  20 |             | 23  AXI
                   GND  21 |_____________| 22  XRST

****************************************************************************/

#ifndef MAME_ROLAND_BU3905_H
#define MAME_ROLAND_BU3905_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bu3905_device

class bu3905_device : public device_t
{
public: // construction/destruction
	bu3905_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t offset, u8 data);

	void axi_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(BU3905, bu3905_device)

#endif // MAME_ROLAND_BU3905_H
