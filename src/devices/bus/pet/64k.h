// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET 64KB RAM Expansion emulation

**********************************************************************/

#pragma once

#ifndef __PET_64K__
#define __PET_64K__

#include "emu.h"
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
	pet_64k_expansion_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_pet_expansion_card_interface overrides
	virtual int pet_norom_r(address_space &space, offs_t offset, int sel) override;
	virtual UINT8 pet_bd_r(address_space &space, offs_t offset, UINT8 data, int &sel) override;
	virtual void pet_bd_w(address_space &space, offs_t offset, UINT8 data, int &sel) override;

private:
	inline UINT8 read_ram(offs_t offset);
	inline void write_ram(offs_t offset, UINT8 data);

	optional_shared_ptr<UINT8> m_ram;

	UINT8 m_ctrl;
};


// device type definition
extern const device_type PET_64K;


#endif
