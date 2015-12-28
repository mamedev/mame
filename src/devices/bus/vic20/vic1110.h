// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1110 8K RAM Expansion Cartridge emulation

**********************************************************************/

#pragma once

#ifndef __VIC1110__
#define __VIC1110__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic1110_device

class vic1110_device :  public device_t,
						public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic1110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_vic20_expansion_card_interface overrides
	virtual UINT8 vic20_cd_r(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(address_space &space, offs_t offset, UINT8 data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	optional_shared_ptr<UINT8> m_ram;
	required_ioport m_sw;
};


// device type definition
extern const device_type VIC1110;



#endif
