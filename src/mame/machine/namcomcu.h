// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
#pragma once

#ifndef __NAMCOMCU_H__
#define __NAMCOMCU_H__

#include "cpu/m37710/m37710.h"


class namco_c69_device : public m37702m2_device
{
public:
	namco_c69_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
protected:
	virtual const rom_entry *device_rom_region() const override;
};


class namco_c70_device : public m37702m2_device
{
public:
	namco_c70_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
protected:
	virtual const rom_entry *device_rom_region() const override;
};


class namco_c74_device : public m37702m2_device
{
public:
	namco_c74_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
protected:
	virtual const rom_entry *device_rom_region() const override;
};


class namco_c75_device : public m37702m2_device
{
public:
	namco_c75_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
protected:
	virtual const rom_entry *device_rom_region() const override;
};


class namco_c76_device : public m37702m2_device
{
public:
	namco_c76_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
protected:
	virtual const rom_entry *device_rom_region() const override;
};


extern const device_type NAMCO_C69;
extern const device_type NAMCO_C70;
extern const device_type NAMCO_C74;
extern const device_type NAMCO_C75;
extern const device_type NAMCO_C76;


#endif
