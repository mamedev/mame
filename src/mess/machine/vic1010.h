/**********************************************************************

    Commodore VIC-1010 Expansion Module emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VIC1010__
#define __VIC1010__


#include "emu.h"
#include "machine/cbmipt.h"
#include "machine/vic20exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MAX_SLOTS 6



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1010_device

class vic1010_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// not really public
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( res_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "vic1010"; }

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3);
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3);
	virtual void vic20_res_w(int state);

private:
	vic20_expansion_slot_device *m_expansion_slot[MAX_SLOTS];
};


// device type definition
extern const device_type VIC1010;



#endif
