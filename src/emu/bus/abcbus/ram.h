// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ABC 80 16 KB RAM expansion card emulation

*********************************************************************/

#pragma once

#ifndef __ABC80_16KB_RAM_CARD__
#define __ABC80_16KB_RAM_CARD__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_16kb_ram_card_t

class abc80_16kb_ram_card_t :  public device_t,
								public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc80_16kb_ram_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data) { };
	virtual UINT8 abcbus_xmemfl(offs_t offset);
	virtual void abcbus_xmemw(offs_t offset, UINT8 data);

private:
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type ABC80_16KB_RAM_CARD;



#endif
