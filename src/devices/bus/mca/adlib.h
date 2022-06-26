// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    Ad Lib MCA

***************************************************************************/

#ifndef MAME_BUS_MCA_ADLIB_H
#define MAME_BUS_MCA_ADLIB_H

#pragma once

#include "mca.h"
#include "sound/ymopl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_adlib_device

class mca16_adlib_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void remap() override;
	virtual void unmap() override;

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

protected:
	mca16_adlib_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual bool map_has_changed() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void update_pos_data_1(uint8_t data) override;
	void update_pos_data_2(uint8_t data) override;

	required_device<ym3812_device> m_ym3812;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_ADLIB, mca16_adlib_device)

#endif