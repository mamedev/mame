// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Personal Peripheral Products Speakeasy cartridge emulation
    (aka Protecto Enterprizes VIC-20 Voice Synthesizer)

**********************************************************************/

#ifndef MAME_BUS_VIC20_SPEAKEASY_H
#define MAME_BUS_VIC20_SPEAKEASY_H

#pragma once

#include "exp.h"
#include "sound/votrax.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic20_speakeasy_device

class vic20_speakeasy_device :  public device_t,
						   public device_vic20_expansion_card_interface
{
public:
	// construction/destruction
	vic20_speakeasy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vic20_expansion_card_interface overrides
	virtual uint8_t vic20_cd_r(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;
	virtual void vic20_cd_w(offs_t offset, uint8_t data, int ram1, int ram2, int ram3, int blk1, int blk2, int blk3, int blk5, int io2, int io3) override;

private:
	required_device<votrax_sc01_device> m_votrax;
};


// device type definition
DECLARE_DEVICE_TYPE(VIC20_SPEAKEASY, vic20_speakeasy_device)

#endif // MAME_BUS_VIC20_SPEAKEASY_H
