/**********************************************************************

    Mega-Cart cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __VIC20_MEGACART__
#define __VIC20_MEGACART__


#include "emu.h"
#include "machine/vic20exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_megacart_device

class vic20_megacart_device :  public device_t,
							   public device_vic20_expansion_card_interface,
							   public device_nvram_interface
{
public:
    // construction/destruction
    vic20_megacart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "megacart"; }

	// device_nvram_interface overrides
	virtual void nvram_default() { }
	virtual void nvram_read(emu_file &file) { if (m_nvram != NULL) { file.read(m_nvram, m_nvram_size); } }
	virtual void nvram_write(emu_file &file) { if (m_nvram != NULL) { file.write(m_nvram, m_nvram_size); } }

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3);
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3);

private:
	int m_nvram_en;
};


// device type definition
extern const device_type VIC20_MEGACART;



#endif
