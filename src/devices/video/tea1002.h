// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    TEA1002

    PAL colour encoder and video summer

                            _____   _____
                   INV   1 |*    \_/     | 18  CBLNK
                     R   2 |             | 17  3,54 MHz
                     G   3 |             | 16  GND
                     B   4 |             | 15  CBF
                _CSYNC   5 |   TEA1002   | 14  8,86 MHz
       lum. delay line   6 |             | 13  8,86 MHz
       lum. delay line   7 |             | 12  PAL switch
   comp. video to mod.   8 |             | 11  chroma band limiting
 d.c. adj. / colour bar  9 |_____________| 10  Vp

***************************************************************************/

#pragma once

#ifndef __TEA1002_H__
#define __TEA1002_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TEA1002_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TEA1002, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tea1002_device

class tea1002_device : public device_t
{
public:
	// construction/destruction
	tea1002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	rgb_t color(int index);

protected:
	// device_t overrides
	virtual void device_start() override;

private:
	static const int m_tint = -6; // what is this based on?
	static const float m_luminance[16];
	static const int m_phase[16];
	static const int m_amplitute[16];
};

// device type definition
extern const device_type TEA1002;

#endif // __TEA1002_H__
