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

#define MCFG_MOS6529_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, MOS6529, _clock)	\
	MCFG_DEVICE_CONFIG(_config)

#define MOS6529_INTERFACE(name) \
	const mos6529_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6529_interface

struct mos6529_interface
{
	devcb_read8				m_in_p_func;
	devcb_write8			m_out_p_func;
};


// ======================> mos6529_device_config

class mos6529_device_config :   public device_config,
                                public mos6529_interface
{
    friend class mos6529_device;

    // construction/destruction
    mos6529_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
    // allocators
    static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
    virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};


// ======================> mos6529_device

class mos6529_device :	public device_t
{
    friend class mos6529_device_config;

    // construction/destruction
    mos6529_device(running_machine &_machine, const mos6529_device_config &_config);

public:
    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

protected:
    // device-level overrides
    virtual void device_start();

private:
	devcb_resolved_read8		m_in_p_func;
	devcb_resolved_write8		m_out_p_func;

	const mos6529_device_config &m_config;
};


// device type definition
extern const device_type MOS6529;



#endif
