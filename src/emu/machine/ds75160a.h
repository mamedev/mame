/**********************************************************************

    National Semiconductor DS75160A IEEE-488 GPIB Transceiver emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    TE   1 |*    \_/     | 20  Vcc
                    D1   2 |             | 19  D1
                    D2   3 |             | 18  D2
                    D3   4 |             | 17  D3
                    D4   5 |   DS75160A  | 16  D4
                    D5   6 |             | 15  D5
                    D6   7 |             | 14  D6
                    D7   8 |             | 13  D7
                    D8   8 |             | 12  D8
                   GND  10 |_____________| 11  PE

**********************************************************************/

#pragma once

#ifndef __DS75160A__
#define __DS75160A__

#include "emu.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_DS75160A_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, DS75160A, 0)	\
	MCFG_DEVICE_CONFIG(_config)


#define DS75160A_INTERFACE(name) \
	const ds75160a_interface (name) =



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ds75160a_interface

struct ds75160a_interface
{
	devcb_read8		m_in_bus_cb;
	devcb_write8	m_out_bus_cb;
};


// ======================> ds75160a_device

class ds75160a_device :	public device_t,
                        public ds75160a_interface
{
public:
    // construction/destruction
    ds75160a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    DECLARE_READ8_MEMBER( read );
    DECLARE_WRITE8_MEMBER( write );

    DECLARE_WRITE_LINE_MEMBER( te_w );
    DECLARE_WRITE_LINE_MEMBER( pe_w );

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();

private:
	devcb_resolved_read8	m_in_bus_func;
	devcb_resolved_write8	m_out_bus_func;

	UINT8 m_data;
	
	int m_te;
	int m_pe;
};


// device type definition
extern const device_type DS75160A;



#endif
