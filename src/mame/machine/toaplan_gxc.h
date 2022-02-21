// license:BSD-3-Clause
// copyright-holders:Quench
#ifndef MAME_MACHINE_TOAPLAN_GXC_H
#define MAME_MACHINE_TOAPLAN_GXC_H

#pragma once

#include "cpu/tms32010/tms32010.h"

// base class
class toaplan_gxc_01_device : public tms32010_device
{
public:
	toaplan_gxc_01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

};

class toaplan_gxc_02_device : public tms32010_device
{
public:
	toaplan_gxc_02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

};

class toaplan_gxc_03_device : public tms32010_device
{
public:
	toaplan_gxc_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

};

class toaplan_gxc_04_device : public tms32010_device
{
public:
	toaplan_gxc_04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;

};

DECLARE_DEVICE_TYPE(TOAPLAN_GXC_01,   toaplan_gxc_01_device)
DECLARE_DEVICE_TYPE(TOAPLAN_GXC_02,   toaplan_gxc_02_device)
DECLARE_DEVICE_TYPE(TOAPLAN_GXC_03,   toaplan_gxc_03_device)
DECLARE_DEVICE_TYPE(TOAPLAN_GXC_04,   toaplan_gxc_04_device)

#endif // MAME_MACHINE_TOAPLAN_GXC_H
