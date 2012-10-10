/**********************************************************************

    CMD FD2000 disk drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __FD2000__
#define __FD2000__

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"
#include "machine/6522via.h"
#include "machine/cbmiec.h"
#include "machine/upd765.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define FD2000_TAG			"fd2000"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fd2000_device

class fd2000_device :  public device_t,
						   public device_cbm_iec_interface
{

public:
    // construction/destruction
    fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "fd2000"; }

	// device_cbm_iec_interface overrides
	void cbm_iec_srq(int state);
	void cbm_iec_atn(int state);
	void cbm_iec_data(int state);
	void cbm_iec_reset(int state);

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type FD2000;



#endif
