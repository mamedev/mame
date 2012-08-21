/**********************************************************************

    Unknown Xebec Winchester controller card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC_XEBEC__
#define __ABC_XEBEC__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/harddriv.h"
#include "machine/abcbus.h"
#include "machine/scsibus.h"
#include "machine/scsihd.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_xebec_device

class abc_xebec_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_xebec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "abc_xebec"; }

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);

private:
	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_sasibus;
};


// device type definition
extern const device_type ABC_XEBEC;



#endif
