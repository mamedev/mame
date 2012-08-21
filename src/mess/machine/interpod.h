/**********************************************************************

    Oxford Computer Systems Interpod IEC to IEEE interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __INTERPOD__
#define __INTERPOD__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/6850acia.h"
#include "machine/c2031.h"
#include "machine/c2040.h"
#include "machine/c8280.h"
#include "machine/d9060.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/ieee488.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define INTERPOD_TAG			"interpod"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTERPOD_ADD() \
    MCFG_DEVICE_ADD(INTERPOD_TAG, INTERPOD, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> interpod_device

class interpod_device :  public device_t,
					     public device_cbm_iec_interface
{
public:
    // construction/destruction
    interpod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete();

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<riot6532_device> m_riot;
	required_device<acia6850_device> m_acia;
	required_device<ieee488_device> m_ieee;
};


// device type definition
extern const device_type INTERPOD;



#endif
