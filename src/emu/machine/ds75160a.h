// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75160A IEEE-488 GPIB Transceiver emulation

**********************************************************************
                            _____   _____
                    TE   1 |*    \_/     | 20  Vcc
                    D1   2 |             | 19  D1
                    D2   3 |             | 18  D2
                    D3   4 |             | 17  D3
                    D4   5 |   DS75160A  | 16  D4
                    D5   6 |             | 15  D5
                    D6   7 |             | 14  D6
                    D7   8 |             | 13  D7
                    D8   8 |             | 12  D8
                   GND  10 |_____________| 11  PE

**********************************************************************/

#pragma once

#ifndef __DS75160A__
#define __DS75160A__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_DS75160A_ADD(_tag, _read, _write) \
	MCFG_DEVICE_ADD(_tag, DS75160A, 0)  \
	downcast<ds75160a_device *>(device)->set_callbacks(DEVCB_##_read, DEVCB_##_write);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ds75160a_device

class ds75160a_device : public device_t
{
public:
	// construction/destruction
	ds75160a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _read, class _write> void set_callbacks(_read rd, _write wr) {
		m_read.set_callback(rd);
		m_write.set_callback(wr);
	}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( te_w );
	DECLARE_WRITE_LINE_MEMBER( pe_w );

protected:
	// device-level overrides
	virtual void device_start();

private:
	devcb_read8  m_read;
	devcb_write8 m_write;

	UINT8 m_data;

	int m_te;
	int m_pe;
};


// device type definition
extern const device_type DS75160A;



#endif
