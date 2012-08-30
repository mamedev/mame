/**********************************************************************

    Xebec S1410 5.25" Winchester Disk Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __S1410__
#define __S1410__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_S1410_ADD( _tag) \
    MCFG_DEVICE_ADD(_tag, S1410, 0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s1410_interface

struct s1410_interface
{
};

// ======================> s1410_device

class s1410_device :  public device_t,
                      public s1410_interface
{
public:
    // construction/destruction
    s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_config_complete();

private:
};


// device type definition
extern const device_type S1410;



#endif
