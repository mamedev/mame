/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   R/W   1 |*    \_/     | 20  Vdd
                    P0   2 |             | 19  _CS
                    P1   3 |             | 18  D0
                    P2   4 |             | 17  D1
                    P3   5 |   MOS6529   | 16  D2
                    P4   6 |             | 15  D3
                    P5   7 |             | 14  D4
                    P6   8 |             | 13  D5
                    P7   9 |             | 12  D6
                   Vss  10 |_____________| 11  D7

**********************************************************************/

#pragma once

#ifndef __MOS6529__
#define __MOS6529__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6529_ADD(_tag, _read, _write) \
	MCFG_DEVICE_ADD(_tag, MOS6529, 0) \
	downcast<mos6529_device *>(device)->set_callbacks(DEVCB2_##_read, DEVCB2_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6529_device

class mos6529_device :  public device_t
{
public:
	// construction/destruction
	mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _read, class _write> void set_callbacks(_read rd, _write wr) {
		m_read_port.set_callback(rd);
		m_write_port.set_callback(wr);
	}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();

private:
	devcb2_read8  m_read_port;
	devcb2_write8 m_write_port;
};


// device type definition
extern const device_type MOS6529;



#endif
