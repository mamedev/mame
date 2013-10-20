// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Scandia Metric DOS floppy controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC_DOS__
#define __ABC_DOS__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_dos_device

class abc_dos_device :  public device_t,
						public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_dos_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data) { };
	virtual UINT8 abcbus_xmemfl(offs_t offset);

private:
	required_memory_region m_rom;
};


// device type definition
extern const device_type ABC_DOS;



#endif
