// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Tiny BASIC VP-700 emulation

**********************************************************************/

#pragma once

#ifndef __VP700__
#define __VP700__

#include "emu.h"
#include "exp.h"
#include "sound/cdp1863.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp700_device

class vp700_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp700_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_vip_expansion_card_interface overrides
	virtual UINT8 vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh);

private:
	required_memory_region m_rom;
};


// device type definition
extern const device_type VP700;


#endif
