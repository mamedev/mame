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

#ifndef MAME_MACHINE_MOS6702_H
#define MAME_MACHINE_MOS6702_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6702_device

class mos6702_device :  public device_t
{
public:
	// construction/destruction
	mos6702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MOS6702, mos6702_device)

#endif // MAME_MACHINE_MOS6702_H
