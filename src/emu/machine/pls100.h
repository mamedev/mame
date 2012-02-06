/**********************************************************************

    PLS100 16x48x8 Programmable Logic Array emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    FE   1 |*    \_/     | 28  Vcc
                    I7   2 |             | 27  I8
                    I6   3 |             | 26  I9
                    I5   4 |             | 25  I10
                    I4   5 |             | 24  I11
                    I3   6 |    82S100   | 23  I12
                    I2   7 |    82S101   | 22  I13
                    I1   8 |    PLS100   | 21  I14
                    I0   9 |    PLS101   | 20  I15
                    F7  10 |             | 19  _CE
                    F6  11 |             | 18  F0
                    F5  12 |             | 17  F1
                    F4  13 |             | 16  F2
                   GND  14 |_____________| 15  F3

**********************************************************************/

#pragma once

#ifndef __PLS100__
#define __PLS100__

#include "emu.h"
#include "jedparse.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PAL_INPUTS		16
#define PAL_OUTPUTS		8
#define PAL_TERMS		48



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PLS100_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PLS100, 0)



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> pls100_device

class pls100_device :	public device_t
{
public:
    // construction/destruction
    pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT8 read(UINT16 input);

protected:
    // device-level overrides
    virtual void device_start();

private:
	inline void parse_fusemap();
	inline int get_product(int term);
	inline void update_outputs();

	UINT16 m_i;
	UINT8 m_s;
	UINT16 m_and_true[PAL_TERMS];
	UINT16 m_and_comp[PAL_TERMS];
	UINT16 m_or[PAL_TERMS];
	UINT8 m_xor;
};


// device type definition
extern const device_type PLS100;



#endif
