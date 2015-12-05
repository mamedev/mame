// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8726R1 DMA Controller emulation

**********************************************************************
                            _____   _____
                /RESET   1 |*    \_/     | 64  Vcc
                  /IRQ   2 |             | 63  BS
                DOTCLK   3 |             | 62  CAS1
                   R/W   4 |             | 61  CAS0
                 1 MHz   5 |             | 60  RAS1
                   /CS   6 |             | 59  RAS0
                   /BA   7 |             | 58  /DWE
                  /DMA   8 |             | 57  DD0
                    D7   9 |             | 56  DD1
                    D6  10 |             | 55  DD2
                    D5  11 |             | 54  DD3
                    D4  12 |             | 53  DD4
                    D3  13 |             | 52  DD5
                    D2  14 |             | 51  DD6
                    D1  15 |   MOS8726   | 50  DD7
                    D0  16 |  MOS8726R1  | 49  Vss
                   Vss  17 |             | 48  MA8
                   A15  18 |             | 47  MA7
                   A14  19 |             | 46  MA6
                   A13  20 |             | 45  MA5
                   A12  21 |             | 44  MA4
                   A11  22 |             | 43  MA3
                   A10  23 |             | 42  MA2
                    A9  24 |             | 41  MA1
                    A8  25 |             | 40  MA0
                    A7  26 |             | 39  TEST
                    A6  27 |             | 38  Vss
                    A5  28 |             | 37  Vcc
                    A4  29 |             | 36  /ROMSEL
                    A3  30 |             | 35  /ROML
                    A2  31 |             | 34  /ROMH
                    A1  32 |_____________| 33  A0

**********************************************************************/

#pragma once

#ifndef __MOS8726__
#define __MOS8726__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS8726_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MOS8726, 1000000) // dummy clock



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8726_device

class mos8726_device :  public device_t,
						public device_execute_interface
{
public:
	// construction/destruction
	mos8726_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( bs_w );

	int romsel_r(int roml, int romh);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_run() override;

	int m_icount;
	int m_bs;
};


// device type definition
extern const device_type MOS8726;



#endif
