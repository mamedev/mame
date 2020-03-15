// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

******************************************************************************/

#ifndef MAME_VIDEO_PPU_SH6578_H
#define MAME_VIDEO_PPU_SH6578_H

#pragma once

#include "video/ppu2c0x.h"

class ppu_sh6578_device : public ppu2c0x_device
{
public:
	ppu_sh6578_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ppu_sh6578_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void device_start() override;
	virtual void device_reset() override;
};

class ppu_sh6578pal_device : public ppu_sh6578_device
{
public:
	ppu_sh6578pal_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(PPU_SH6578,    ppu_sh6578_device)
DECLARE_DEVICE_TYPE(PPU_SH6578PAL, ppu_sh6578pal_device)

#endif // MAME_VIDEO_PPU_SH6578_H
