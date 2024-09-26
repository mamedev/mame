// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Grull Osgo

#ifndef MAME_BUS_ATA_XM3301_H
#define MAME_BUS_ATA_XM3301_H

#pragma once

#include "atapicdr.h"

class toshiba_xm3301_device : public atapi_cdrom_device
{
public:
	toshiba_xm3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void identify_packet_device() override;
private:

};

// device type definition
DECLARE_DEVICE_TYPE(XM3301, toshiba_xm3301_device)

#endif // MAME_BUS_ATA_XM3301_H
