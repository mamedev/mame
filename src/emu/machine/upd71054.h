// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    NEC uPD71054 programmable timer/counter

    Copyright MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                 D7 <->  1 |*    \_/     | 24 --- Vdd
                 D6 <->  2 |             | 23 <-- _WR
                 D5 <->  3 |             | 22 <-- _RD
                 D4 <->  4 |             | 21 <-- _CS
                 D3 <->  5 |             | 20 <-- A1
                 D2 <->  6 |  uPD71054C  | 19 <-- A0
                 D1 <->  7 |             | 18 <-- CLK2
                 D0 <->  8 |             | 17 --> OUT2
               CLK0 -->  9 |             | 16 <-- GATE2
               OUT0 <-- 10 |             | 15 <-- CLK1
              GATE0 --> 11 |             | 14 <-- GATE1
                GND --- 12 |_____________| 13 --> OUT1

    uPD71054C (8MHz), uPD71054C-10 (10MHz)
    also available in 28-pin QFP and 44-pin PLCC (many pins NC)

**********************************************************************/

#pragma once

#ifndef _UPD71054_H
#define _UPD71054_H

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//..



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd71054_device

class upd71054_device : public device_t
{
public:
	upd71054_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:

};

// device type definition
extern const device_type UPD71054;


#endif /* _UPD71054_H */
