// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3mp1802.h

    Hitachi MP-1802 floppy disk controller card for the MB-6890
    Hitachi MP-3550 floppy drive is attached

*********************************************************************/

#ifndef MAME_BUS_BML3_BML3MP1802_H
#define MAME_BUS_BML3_BML3MP1802_H

#pragma once

#include "bml3bus.h"
#include "imagedev/floppy.h"
#include "machine/input_merger.h"
#include "machine/wd_fdc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bml3bus_mp1802_device:
	public device_t,
	public device_bml3bus_card_interface
{
public:
	// construction/destruction
	bml3bus_mp1802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();
	void write(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void map_exrom(address_space_installer &space) override;
	virtual void map_io(address_space_installer &space) override;

private:
	void nmi_w(int state);

	required_device<mb8866_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<input_merger_device> m_nmigate;

	required_region_ptr<uint8_t> m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_MP1802, bml3bus_mp1802_device)

#endif // MAME_BUS_BML3_BML3MP1802_H
