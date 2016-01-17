// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1210 3K RAM Expansion Cartridge emulation
    Commodore VIC-1211A Super Expander with 3K RAM Cartridge emulation

**********************************************************************/

#pragma once

#ifndef __VIC1210__
#define __VIC1210__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1210_device

class vic1210_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1210_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	optional_shared_ptr<UINT8> m_ram;
};


// device type definition
extern const device_type VIC1210;



#endif
