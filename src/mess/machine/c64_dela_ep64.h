/**********************************************************************

    Dela 64KB EPROM cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __DELA_EP64__
#define __DELA_EP64__


#include "emu.h"
#include "imagedev/cartslot.h"
#include "machine/c64exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_dela_ep64_cartridge_device

class c64_dela_ep64_cartridge_device : public device_t,
									   public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_dela_ep64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_dela_ep64"; }
	virtual void device_start();
	virtual void device_reset();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);

private:
	UINT8 *m_rom;

	UINT8 m_bank;
	int m_reset;
	int m_rom0_ce;
	int m_rom1_ce;
	int m_rom2_ce;
};


// device type definition
extern const device_type C64_DELA_EP64;



#endif
