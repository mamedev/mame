// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8706 Speech Glue Logic ASIC emulation

**********************************************************************
                            _____   _____
                  _RES   1 |*    \_/     | 28  Vdd
                  _IRQ   2 |             | 27  D0
                  R/_W   3 |             | 26  T6721A D0
                  phi0   4 |             | 25  D1
                   _CS   5 |             | 24  T6721A D1
                    A0   6 |             | 23  D2
                    A1   7 |   MOS8706   | 22  T6721A D2
                         8 |             | 21  D3
                  _EOS   9 |             | 20  T6721A D3
                   APD  10 |             | 19  D4
                  phi2  11 |             | 18  D5
                    DI  12 |             | 17  D6
                  DTRD  13 |             | 16  D7
                   GND  14 |_____________| 15  _WR

**********************************************************************/

#pragma once

#ifndef __MOS8706__
#define __MOS8706__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS8706_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD((_tag), MOS8706, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8706_device

class mos8706_device : public device_t
{
public:
	mos8706_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
extern const device_type MOS8706;



#endif
