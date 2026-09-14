// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS5710 Custom Floppy Controller and Gate Array

    Used in 1571CR

**********************************************************************
                            _____   _____
               SRQ OUT   1 |*    \_/     | 48  *DATA OUT
                   TED   2 |             | 47  SRQ IN
                  phi0   3 |             | 46  *DATA IN
                    CK   4 |             | 45  *IRQ
                 *ACCL   5 |             | 44  *RESET
              VIA1 PA5   6 |             | 43
                  phi2   7 |             | 42
                    D7   8 |             | 41  *INDEX
                    D6   9 |             | 40  WG
                    D5  10 |             | 39  *WPRT
                    D4  11 |             | 38  *RD
                   Vss  12 |   MOS5710   | 37  WD
                   Vcc  13 |             | 36  Vcc
                    D3  14 |             | 35  Vss
                    D2  15 |             | 34  *VIA1
                    D1  16 |             | 33  *RAM
                    D0  17 |             | 32  *VIA2
                   A15  18 |             | 31  R/W
                   A14  19 |             | 30  16MHz
                   A13  20 |             | 29  XTL1
                   A12  21 |             | 28  XTL2
                   A10  22 |             | 27  A0
                    A4  23 |             | 26  A1
                    A3  24 |_____________| 25  A2

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MOS5710_H
#define MAME_BUS_CBMIEC_MOS5710_H

#pragma once

#include "cbmiec.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6805/m68705.h"

class mos5710_device : public device_t
{
public:
	mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(MOS5710, mos5710_device)

#endif // MAME_BUS_CBMIEC_MOS5710_H
