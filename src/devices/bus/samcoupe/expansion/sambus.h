// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAMBUS 4-slot Expansion Interface for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_SAMBUS_H
#define MAME_BUS_SAMCOUPE_EXPANSION_SAMBUS_H

#pragma once

#include "expansion.h"
#include "machine/msm6242.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_sambus_device

class sam_sambus_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_sambus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void xmem_w(int state) override;
	virtual void print_w(int state) override;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<msm6242_device> m_rtc;
	required_device_array<samcoupe_expansion_device, 4> m_exp;

	int m_print;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_SAMBUS, sam_sambus_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_SAMBUS_H
