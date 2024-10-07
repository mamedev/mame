// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIB Disc Drive DD-001 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_TIB_DD_001_H
#define MAME_BUS_C64_TIB_DD_001_H

#pragma once


#include "exp.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_tib_dd_001_device

class c64_tib_dd_001_device : public device_t,
							  public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_tib_dd_001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	static void floppy_formats(format_registration &fr);

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	void motor_w(int state) { m_floppy->get_device()->mon_w(!state); }
};


// device type definition
DECLARE_DEVICE_TYPE(C64_TIB_DD_001, c64_tib_dd_001_device)


#endif // MAME_BUS_C64_TIB_DD_001_H
