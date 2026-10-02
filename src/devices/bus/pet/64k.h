// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET 64KB RAM Expansion emulation

**********************************************************************/

#ifndef MAME_BUS_PET_64K_H
#define MAME_BUS_PET_64K_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_64k_expansion_device

class pet_64k_expansion_device : public device_t, public device_pet_expansion_card_interface
{
public:
	// construction/destruction
	pet_64k_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void update_window();
	void ctrl_w(uint8_t data);

	memory_share_creator<uint8_t> m_ram;

	uint8_t m_ctrl;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_64K, pet_64k_expansion_device)

#endif // MAME_BUS_PET_64K_H
