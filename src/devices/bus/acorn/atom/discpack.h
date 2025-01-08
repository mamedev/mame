// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Disc Pack

**********************************************************************/


#ifndef MAME_BUS_ACORN_ATOM_DISCPACK_H
#define MAME_BUS_ACORN_ATOM_DISCPACK_H

#pragma once

#include "bus/acorn/bus.h"
#include "imagedev/floppy.h"
#include "machine/i8271.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class atom_discpack_device :
	public device_t,
	public device_acorn_bus_interface
{
public:
	// construction/destruction
	atom_discpack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void fdc_intrq_w(int state);
	void motor_w(int state);
	void side_w(int state);

	required_memory_region m_dos_rom;
	required_device<i8271_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	u8 m_ram[0x800 + 0x400];
};


// device type definition
DECLARE_DEVICE_TYPE(ATOM_DISCPACK, atom_discpack_device)


#endif // MAME_BUS_ACORN_ATOM_DISCPACK_H
