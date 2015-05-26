// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    Texas Instruments TMS1024, TMS1025 I/O expander emulation

**********************************************************************

            ____   ____                         ____   ____
    Vss  1 |*   \_/    | 28 H2          Vss  1 |*   \_/    | 40 H2
     H3  2 |           | 27 H1           H3  2 |           | 39 H1
     H4  3 |           | 26 Vdd          H4  3 |           | 38 Vdd
     CE  4 |           | 25 S2           CE  4 |           | 37 S2
     MS  5 |           | 24 S1           MS  5 |           | 36 S1
    STD  6 |           | 23 S0          STD  6 |           | 35 S0
     A4  7 |  TMS1024  | 22 D7           A1  7 |           | 34 D3
     B4  8 |           | 21 C7           B1  8 |           | 33 C3
     C4  9 |           | 20 B7           C1  9 |           | 32 B3
     D4 10 |           | 19 A7           D1 10 |  TMS1025  | 31 A3
     A5 11 |           | 18 D6           A4 11 |           | 30 D7
     B5 12 |           | 17 C6           B4 12 |           | 29 C7
     C5 13 |           | 16 B6           C4 13 |           | 28 B7
     D5 14 |___________| 15 A6           D4 14 |           | 27 A7
                                         A5 15 |           | 26 D6
                                         B5 16 |           | 25 C6
                                         C5 17 |           | 24 B6
                                         D5 18 |           | 23 A6
                                         A2 19 |           | 22 D2
                                         B2 20 |___________| 21 C2

**********************************************************************/

#ifndef _TMS1024_H_
#define _TMS1024_H_

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS1024_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMS1024, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tms1024_device

class tms1024_device : public device_t
{
public:
	tms1024_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};

// device type definition
extern const device_type TMS1024;


#endif /* _TMS1024_H_ */
