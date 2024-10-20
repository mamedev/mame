// license:BSD-3-Clause
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

#ifndef MAME_VIDEO_TEA1002_H
#define MAME_VIDEO_TEA1002_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tea1002_device

class tea1002_device : public device_t
{
public:
	// construction/destruction
	tea1002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	rgb_t color(int index);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	static const int m_tint = -6; // what is this based on?
	static const float m_luminance[16];
	static const int m_phase[16];
	static const int m_amplitute[16];
};

// device type definition
DECLARE_DEVICE_TYPE(TEA1002, tea1002_device)

#endif // MAME_VIDEO_TEA1002_H
