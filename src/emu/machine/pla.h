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

#ifndef __PLA__
#define __PLA__

#include "emu.h"
#include "jedparse.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MAX_TERMS       512



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PLS100_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PLS100, 0)

#define MCFG_MOS8721_ADD(_tag) \
    MCFG_DEVICE_ADD(_tag, MOS8721, 0)


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> pla_device

class pla_device :	public device_t
{
public:
    // construction/destruction
    pla_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int inputs, int outputs, int terms, UINT32 output_mask);

	UINT32 read(UINT32 input);

protected:
    // device-level overrides
    virtual void device_start();

	inline void parse_fusemap();
	inline int get_product(int term);
	inline void update_outputs();

    int m_inputs;
    int m_outputs;
    int m_terms;
    UINT32 m_output_mask;

	UINT32 m_i;
	UINT32 m_s;
	UINT32 m_and_true[MAX_TERMS];
	UINT32 m_and_comp[MAX_TERMS];
	UINT32 m_or[MAX_TERMS];
	UINT32 m_xor;
};


// ======================> pls100_device

class pls100_device : public pla_device
{
public:
    pls100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> mos8721_device

class mos8721_device : public pla_device
{
public:
    mos8721_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type PLS100;
extern const device_type MOS8721;



#endif
