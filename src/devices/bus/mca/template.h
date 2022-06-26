// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    MCA card template

***************************************************************************/

#ifndef MAME_BUS_MCA_TEMPLATE_H
#define MAME_BUS_MCA_TEMPLATE_H

#pragma once

#include "mca.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_template_device

class mca16_template_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_template_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void unmap() override;
	virtual void remap() override;

protected:
	mca16_template_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t pos_r(offs_t offset) override;
	virtual void pos_w(offs_t offset, uint8_t data) override;

    void map_main(address_map &map);

private:
	uint8_t m_is_mapped;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_TEMPLATE, mca16_template_device)

#endif