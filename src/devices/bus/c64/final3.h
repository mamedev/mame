// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Final Cartridge III emulation

**********************************************************************/

#ifndef MAME_BUS_C64_FINAL3_H
#define MAME_BUS_C64_FINAL3_H

#pragma once


#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_final3_cartridge_device

class c64_final3_cartridge_device : public device_t,
									public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_final3_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER( freeze );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	uint8_t m_bank;
	int m_hidden;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_FINAL3, c64_final3_cartridge_device)


#endif // MAME_BUS_C64_FINAL3_H
