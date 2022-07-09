// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
// Lindbergh Sega baseboard

#ifndef MAME_MACHINE_SEGABB_H
#define MAME_MACHINE_SEGABB_H

#pragma once

#include "machine/pci.h"

class sega_lindbergh_baseboard_device : public pci_device {
public:
	sega_lindbergh_baseboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map1(address_map &map);
	void map2(address_map &map);
	void map3(address_map &map);
};

DECLARE_DEVICE_TYPE(SEGA_LINDBERGH_BASEBOARD, sega_lindbergh_baseboard_device)

#endif // MAME_MACHINE_SEGABB_H
