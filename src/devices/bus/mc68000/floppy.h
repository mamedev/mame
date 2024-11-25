// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    mc-68000-Computer Floppy Interface Card

***************************************************************************/

#ifndef MAME_BUS_MC68000_FLOPPY_H
#define MAME_BUS_MC68000_FLOPPY_H

#pragma once

#include "sysbus.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


class mc68000_floppy_device : public device_t, public device_mc68000_sysbus_card_interface
{
public:
	// construction/destruction
	mc68000_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t floppy_r(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void floppy_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 8> m_floppy;

	uint8_t m_latch;

	void intrq_w(int state);
	void drq_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(MC68000_FLOPPY, mc68000_floppy_device)


#endif // MAME_BUS_MC68000_FLOPPY_H
