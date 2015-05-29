// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC Memory Card 55 10762-01 emulation

*********************************************************************/

#pragma once

#ifndef __ABC_MEMORY_CARD__
#define __ABC_MEMORY_CARD__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_memory_card_t

class abc_memory_card_t :  public device_t,
							public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_memory_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data) { };
	virtual UINT8 abcbus_xmemfl(offs_t offset);

private:
	required_memory_region m_dos_rom;
	required_memory_region m_iec_rom;
	required_memory_region m_opt_rom;
	required_memory_region m_prn_rom;
};


// device type definition
extern const device_type ABC_MEMORY_CARD;



#endif
