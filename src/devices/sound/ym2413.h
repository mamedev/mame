// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2413_H
#define MAME_SOUND_YM2413_H

#pragma once

#include "ymfm.h"


// ======================> ym2413_device

DECLARE_DEVICE_TYPE(YM2413, ym2413_device);

class ym2413_device : public device_t, public device_sound_interface
{
public:
	// YM2151 is OPLL
	using fm_engine = ymopll_engine;

	// constructor
	ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YM2413, u8 const *instruments = nullptr);

	// no read access present

	// write access
	void address_w(u8 data);    // A0=0
	void data_w(u8 data);       // A0=1
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal state
	u8 m_address;                       // address register
	sound_stream *m_stream;             // sound stream
	required_region_ptr<u8> m_internal; // internal memory region
	fm_engine m_fm;                     // core FM engine
};


// ======================> ym2423_device

DECLARE_DEVICE_TYPE(YM2423, ym2423_device);

class ym2423_device : public ym2413_device
{
public:
	// constructor
	ym2423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// ======================> ymf281_device

DECLARE_DEVICE_TYPE(YMF281, ymf281_device);

class ymf281_device : public ym2413_device
{
public:
	// constructor
	ymf281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// ======================> ds1001_device

DECLARE_DEVICE_TYPE(DS1001, ds1001_device);

class ds1001_device : public ym2413_device
{
public:
	// constructor
	ds1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


#endif // MAME_SOUND_YM2413_H
