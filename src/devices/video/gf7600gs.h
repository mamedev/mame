// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_VIDEO_GF7600GS_H
#define MAME_VIDEO_GF7600GS_H

#pragma once

#include "machine/pci.h"

#define MCFG_GEFORCE_7600GS_ADD(_tag, _subdevice_id) \
	MCFG_AGP_DEVICE_ADD(_tag, GEFORCE_7600GS, 0x10de02e1, 0xa1, _subdevice_id)

class geforce_7600gs_device : public pci_device {
public:
	geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map1(address_map &map);
	void map2(address_map &map);
	void map3(address_map &map);
};

DECLARE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device)

#endif // MAME_VIDEO_GF7600GS_H
