// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore IEEE-488 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_IEEE488_H
#define MAME_BUS_C64_IEEE488_H

#pragma once

#include "exp.h"
#include "bus/ieee488/ieee488.h"
#include "machine/6525tpi.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_ieee488_device

class c64_ieee488_device : public device_t,
							public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	uint8_t tpi_pa_r();
	void tpi_pa_w(uint8_t data);
	uint8_t tpi_pc_r(offs_t offset);
	void tpi_pc_w(uint8_t data);

	required_device<tpi6525_device> m_tpi;
	required_device<ieee488_device> m_bus;
	required_device<c64_expansion_slot_device> m_exp;

	int m_roml_sel;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_IEEE488, c64_ieee488_device)


#endif // MAME_BUS_C64_IEEE488_H
