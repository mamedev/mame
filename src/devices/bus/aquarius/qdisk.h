// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Radofin Aquarius Quick Disk

**********************************************************************/

#ifndef MAME_BUS_AQUARIUS_QDISK_H
#define MAME_BUS_AQUARIUS_QDISK_H

#pragma once

#include "slot.h"
#include "machine/mc6852.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class aquarius_qdisk_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_qdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mreq_ce_r(offs_t offset) override;
	virtual void mreq_ce_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

private:
	required_device<mc6852_device> m_ssda;
	required_device<aquarius_cartridge_slot_device> m_exp;
	required_ioport m_sw1;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_QDISK, aquarius_qdisk_device)


#endif // MAME_BUS_AQUARIUS_QDISK_H
