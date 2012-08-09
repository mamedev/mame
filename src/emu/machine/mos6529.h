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

#define MCFG_MOS6529_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, MOS6529, 0)	\
	MCFG_DEVICE_CONFIG(_config)

#define MOS6529_INTERFACE(name) \
	const mos6529_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6529_interface

struct mos6529_interface
{
	devcb_read8				m_in_p_cb;
	devcb_write8			m_out_p_cb;
};


// ======================> mos6529_device

class mos6529_device :	public device_t,
                        public mos6529_interface
{
public:
    // construction/destruction
    mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();

private:
	devcb_resolved_read8		m_in_p_func;
	devcb_resolved_write8		m_out_p_func;
};


// device type definition
extern const device_type MOS6529;



#endif
