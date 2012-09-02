/**********************************************************************

    Commodore 64 16KB EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_16KB__
#define __C64_16KB__


#include "emu.h"
#include "imagedev/cartslot.h"
#include "machine/c64exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_16kb_cartridge_device

class c64_16kb_cartridge_device : public device_t,
								  public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_16kb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_16kb"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);
};


// device type definition
extern const device_type C64_16KB;



#endif
