// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Dallas Clock for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_DALLAS_H
#define MAME_BUS_SAMCOUPE_EXPANSION_DALLAS_H

#pragma once

#include "expansion.h"
#include "machine/ds128x.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_dallas_clock_device

class sam_dallas_clock_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_dallas_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void print_w(int state) override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<ds12885_device> m_rtc;

	int m_print;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_DALLAS_CLOCK, sam_dallas_clock_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_DALLAS_H
