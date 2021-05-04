// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2413_H
#define MAME_SOUND_YM2413_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm_opl.h"


// ======================> ym2413_device

DECLARE_DEVICE_TYPE(YM2413, ym2413_device);

class ym2413_device : public ymfm_device_base<ymfm::ym2413>
{
	using parent = ymfm_device_base<ymfm::ym2413>;

public:
	// constructor
	ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// internal state
	required_region_ptr<u8> m_internal; // internal memory region
};


// ======================> ym2423_device

DECLARE_DEVICE_TYPE(YM2423, ym2423_device);

class ym2423_device : public ymfm_device_base<ymfm::ym2423>
{
	using parent = ymfm_device_base<ymfm::ym2423>;

public:
	// constructor
	ym2423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// internal state
	required_region_ptr<u8> m_internal; // internal memory region
};


// ======================> ymf281_device

DECLARE_DEVICE_TYPE(YMF281, ymf281_device);

class ymf281_device : public ymfm_device_base<ymfm::ymf281>
{
	using parent = ymfm_device_base<ymfm::ymf281>;

public:
	// constructor
	ymf281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// internal state
	required_region_ptr<u8> m_internal; // internal memory region
};


// ======================> ds1001_device

DECLARE_DEVICE_TYPE(DS1001, ds1001_device);

class ds1001_device : public ymfm_device_base<ymfm::ds1001>
{
	using parent = ymfm_device_base<ymfm::ds1001>;

public:
	// constructor
	ds1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// internal state
	required_region_ptr<u8> m_internal; // internal memory region
};

#endif // MAME_SOUND_YM2413_H
