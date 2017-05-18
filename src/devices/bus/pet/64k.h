// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET 64KB RAM Expansion emulation

**********************************************************************/

#pragma once

#ifndef __PET_64K__
#define __PET_64K__

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_64k_expansion_device

class pet_64k_expansion_device : public device_t,
									public device_pet_expansion_card_interface
{
public:
	// construction/destruction
	pet_64k_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pet_expansion_card_interface overrides
	virtual int pet_norom_r(address_space &space, offs_t offset, int sel) override;
	virtual uint8_t pet_bd_r(address_space &space, offs_t offset, uint8_t data, int &sel) override;
	virtual void pet_bd_w(address_space &space, offs_t offset, uint8_t data, int &sel) override;

private:
	inline uint8_t read_ram(offs_t offset);
	inline void write_ram(offs_t offset, uint8_t data);

	optional_shared_ptr<uint8_t> m_ram;

	uint8_t m_ctrl;
};


// device type definition
extern const device_type PET_64K;


#endif
