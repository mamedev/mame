// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6702 Mystery Device emulation

**********************************************************************
                            _____   _____
                  R/_W   1 |*    \_/     | 20  Vcc
                    D7   2 |             | 19  CS0
                    D6   3 |             | 18  CS1
                    D5   4 |             | 17  CS2
                    D4   5 |   MOS6702   | 16  CS3
                    D3   6 |             | 15  _CS4
                    D2   7 |             | 14  _CS5
                    D1   8 |             | 13  _CS5
                    D0   9 |             | 12  _RTS
                   Vss  10 |_____________| 11  phi2

**********************************************************************/

#pragma once

#ifndef __MOS6702__
#define __MOS6702__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6702_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, MOS6702, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6702_device

class mos6702_device :  public device_t
{
public:
	// construction/destruction
	mos6702_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type MOS6702;



#endif
