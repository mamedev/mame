// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    1 Mb Interface for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_SAMCOUPE_EXPANSION_ONEMEG_H
#define MAME_BUS_SAMCOUPE_EXPANSION_ONEMEG_H

#pragma once

#include "expansion.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sam_onemeg_device

class sam_onemeg_device : public device_t, public device_samcoupe_expansion_interface
{
public:
	// construction/destruction
	sam_onemeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void xmem_w(int state) override;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_dip;

	int m_xmem;
	uint8_t m_expage[2];

	std::unique_ptr<uint8_t[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(SAM_ONEMEG, sam_onemeg_device)

#endif // MAME_BUS_SAMCOUPE_EXPANSION_ONEMEG_H
