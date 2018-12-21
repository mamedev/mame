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

	DECLARE_READ8_MEMBER(bml3_mp1802_r);
	DECLARE_WRITE8_MEMBER(bml3_mp1802_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(bml3_wd17xx_intrq_w);

	required_device<mb8866_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

	uint8_t *m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(BML3BUS_MP1802, bml3bus_mp1802_device)

#endif // MAME_BUS_BML3_BML3MP1802_H
