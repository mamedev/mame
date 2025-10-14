// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPL_H
#define MAME_SOUND_YMOPL_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opl.h"
#include "dirom.h"


// ======================> ym3526_device

DECLARE_DEVICE_TYPE(YM3526, ym3526_device);

class ym3526_device : public ymfm_device_base<ymfm::ym3526>
{
public:
	// constructor
	ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> y8950_device

DECLARE_DEVICE_TYPE(Y8950, y8950_device);

class y8950_device : public ymfm_device_base<ymfm::y8950>, public device_rom_interface<21>
{
	using parent = ymfm_device_base<ymfm::y8950>;

public:
	// constructor
	y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto io_read() { return io_read_handler(0); }
	auto io_write() { return io_write_handler(0); }
	auto keyboard_read() { return io_read_handler(1); }
	auto keyboard_write() { return io_write_handler(1); }

	// additional register reads
	u8 data_r() { return update_streams().read_data(); }

protected:
	// ROM device overrides
	virtual void rom_bank_pre_change() override;

private:
	// ADPCM read/write callbacks
	uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;
	void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) override;
};


// ======================> ym3812_device

DECLARE_DEVICE_TYPE(YM3812, ym3812_device);

class ym3812_device : public ymfm_device_base<ymfm::ym3812>
{
public:
	// constructor
	ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ymf262_device

DECLARE_DEVICE_TYPE(YMF262, ymf262_device);

class ymf262_device : public ymfm_device_base<ymfm::ymf262>
{
public:
	// constructor
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data(data); }
};


// ======================> ymf278b_device

DECLARE_DEVICE_TYPE(YMF278B, ymf278b_device);

class ymf278b_device : public ymfm_device_base<ymfm::ymf278b>, public device_rom_interface<22>
{
	using parent = ymfm_device_base<ymfm::ymf278b>;

public:
	// constructor
	ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register reads
	uint8_t data_pcm_r() { return update_streams().read_data_pcm(); }

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data(data); }
	void address_pcm_w(u8 data) { update_streams().write_address_pcm(data); }
	void data_pcm_w(u8 data) { update_streams().write_data_pcm(data); }

protected:
	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// ADPCM read/write callbacks
	uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;
	void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) override;
};


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
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

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
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// internal state
	required_region_ptr<u8> m_internal; // internal memory region
};

#endif // MAME_SOUND_YMOPL_H
