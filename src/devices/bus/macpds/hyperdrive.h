// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_BUS_PDS_HYPERDRIVE_H
#define MAME_BUS_PDS_HYPERDRIVE_H

#pragma once

#include "macpds.h"
#include "machine/wd2010.h"
#include "imagedev/harddriv.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> macpds_hyperdrive_device

class macpds_hyperdrive_device :
		public device_t,
		public device_macpds_card_interface
{
public:
	// construction/destruction
	macpds_hyperdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::DISK; }

protected:
	macpds_hyperdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<wd2010_device> m_hdc;

private:
	uint16_t hyperdrive_r(offs_t offset);
	void hyperdrive_w(offs_t offset, uint16_t data);

	uint8_t hdd_r(offs_t offset);
	void hdd_w(offs_t offset, uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(PDS_HYPERDRIVE, macpds_hyperdrive_device)

#endif // MAME_BUS_PDS_TPDFPD_H
