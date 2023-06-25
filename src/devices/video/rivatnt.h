// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_RIVATNT_H
#define MAME_VIDEO_RIVATNT_H

#pragma once

#include "machine/pci.h"
#include "video/pc_vga_nvidia.h"
#include "riva128.h"

class rivatnt_device : public riva128_device
{
public:
	rivatnt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
};

DECLARE_DEVICE_TYPE(RIVATNT, rivatnt_device)

#endif
