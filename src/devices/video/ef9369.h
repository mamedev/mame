// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Thomson EF9369

    Single Chip Color Palette

               ___ ___
      VSS   1 |*  u   | 28  HP
     VDDC   2 |       | 27  P3
      SMI   3 |       | 26  P2
       CC   4 |       | 25  P1
       CA   5 |       | 24  P0
       CB   6 |       | 23  BLK
        M   7 |       | 22  AS
      AD0   8 |       | 21  R/W
      VCC   9 |       | 20  DS
    RESET  10 |       | 19  CS0
      AD1  11 |       | 18  /CS
      AD2  12 |       | 17  AD7
      AD3  13 |       | 16  AD6
      AD4  14 |_______| 15  AD5

***************************************************************************/

#pragma once

#ifndef __EF9369_H__
#define __EF9369_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_EF9369_ADD(_tag, _palette_tag) \
	MCFG_DEVICE_ADD(_tag, EF9369, 0) \
	ef9369_device::set_palette_tag(*device, owner, _palette_tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ef9369_device

class ef9369_device : public device_t
{
public:
	// construction/destruction
	ef9369_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	static void set_palette_tag(device_t &device, device_t *owner, const char *tag);

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_WRITE8_MEMBER(address_w);

	static const int NUMCOLORS = 16;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// configuration
	const char *m_palette_tag;
	palette_device *m_palette;

	// state
	uint8_t m_ca[NUMCOLORS], m_cb[NUMCOLORS], m_cc[NUMCOLORS];	// actually 4-bit
	bool m_m[NUMCOLORS];
	int m_address;
};

// device type definition
extern const device_type EF9369;

#endif // __EF9369_H__
