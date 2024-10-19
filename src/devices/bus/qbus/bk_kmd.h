// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK0011 floppy controller (device driver BY.SYS)

***************************************************************************/

#ifndef MAME_BUS_QBUS_BK_KMD_H
#define MAME_BUS_QBUS_BK_KMD_H

#pragma once

#include "machine/1801vp128.h"

#include "qbus.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> bk_kmd_device

class bk_kmd_device : public device_t,
					public device_qbus_card_interface
{
public:
	// construction/destruction
	bk_kmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<k1801vp128_device> m_fdc;

private:
	static void floppy_formats(format_registration &fr);
};


// device type definition
DECLARE_DEVICE_TYPE(BK_KMD, bk_kmd_device)

#endif
