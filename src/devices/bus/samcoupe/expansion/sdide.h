// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    S D IDE Adapter for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_SDIDE_H
#define MAME_BUS_SAMCOUPE_EXPANSION_SDIDE_H

#pragma once

#include "expansion.h"
#include "bus/ata/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_sdide_device

class sam_sdide_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_sdide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<ata_interface_device> m_ata;

	uint8_t m_address_latch;
	uint8_t m_data_latch;
	bool m_data_pending;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_SDIDE, sam_sdide_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_SDIDE_H
