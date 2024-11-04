// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_SIS630_VGA_H
#define MAME_MACHINE_SIS630_VGA_H

#pragma once

#include "pci.h"
#include "video/pc_vga.h"

class mediagx_cs5530_video_device : public pci_device
{
public:
	mediagx_cs5530_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MEDIAGX_CS5530_VIDEO, mediagx_cs5530_video_device)


#endif
