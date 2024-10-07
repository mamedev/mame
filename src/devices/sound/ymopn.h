// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPN_H
#define MAME_SOUND_YMOPN_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm/src/ymfm_opn.h"
#include "dirom.h"


// ======================> ym2203_device

DECLARE_DEVICE_TYPE(YM2203, ym2203_device);

class ym2203_device : public ymfm_ssg_device_base<ymfm::ym2203>
{
	using parent = ymfm_ssg_device_base<ymfm::ym2203>;

public:
	// constructor
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_a_read_callback() { return io_read_handler(0); }
	auto port_b_read_callback() { return io_read_handler(1); }
	auto port_a_write_callback() { return io_write_handler(0); }
	auto port_b_write_callback() { return io_write_handler(1); }

	// data register read
	u8 data_r() { return update_streams().read_data(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> ym2608_device

DECLARE_DEVICE_TYPE(YM2608, ym2608_device);

class ym2608_device : public ymfm_ssg_device_base<ymfm::ym2608>, public device_rom_interface<21>
{
	using parent = ymfm_ssg_device_base<ymfm::ym2608>;

public:
	// constructor
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers, handled by the interface
	auto port_a_read_callback() { return io_read_handler(0); }
	auto port_b_read_callback() { return io_read_handler(1); }
	auto port_a_write_callback() { return io_write_handler(0); }
	auto port_b_write_callback() { return io_write_handler(1); }

	// additional register reads
	u8 data_r() { return update_streams().read_data(); }
	u8 status_hi_r() { return update_streams().read_status_hi(); }
	u8 data_hi_r() { return update_streams().read_data_hi(); }

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// ROM device overrides
	virtual void rom_bank_pre_change() override;

private:
	// ADPCM read/write callbacks
	virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;
	virtual void ymfm_external_write(ymfm::access_class type, uint32_t address, u8 data) override;

	// internal state
	required_memory_region m_internal; // internal memory region
};


// ======================> ym2610_device_base

template<typename ChipClass>
class ym2610_device_base : public ymfm_ssg_device_base<ChipClass>, public device_memory_interface
{
	using parent = ymfm_ssg_device_base<ChipClass>;

public:
	// constructor
	ym2610_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

	// additional register reads
	u8 data_r() { return update_streams().read_data(); }
	u8 status_hi_r() { return update_streams().read_status_hi(); }
	u8 data_hi_r() { return update_streams().read_data_hi(); }

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }

protected:
	using parent::update_streams;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// memory space configuration
	virtual space_config_vector memory_space_config() const override;

private:
	// ADPCM read/write callbacks
	virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override;

	// internal state
	address_space_config const m_adpcm_a_config; // address space 0 config (ADPCM-A)
	address_space_config const m_adpcm_b_config; // address space 1 config (ADPCM-B)
	optional_memory_region m_adpcm_a_region; // ADPCM-A memory region
	optional_memory_region m_adpcm_b_region; // ADPCM-B memory region
};


// ======================> ym2610_device

DECLARE_DEVICE_TYPE(YM2610, ym2610_device);

class ym2610_device : public ym2610_device_base<ymfm::ym2610>
{
public:
	ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ym2610b_device

DECLARE_DEVICE_TYPE(YM2610B, ym2610b_device);

class ym2610b_device : public ym2610_device_base<ymfm::ym2610b>
{
public:
	ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> ym2612_device

DECLARE_DEVICE_TYPE(YM2612, ym2612_device);

class ym2612_device : public ymfm_device_base<ymfm::ym2612>
{
public:
	// constructor
	ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }
};


// ======================> ym3438_device

DECLARE_DEVICE_TYPE(YM3438, ym3438_device);

class ym3438_device : public ymfm_device_base<ymfm::ym3438>
{
public:
	// constructor
	ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }
};


// ======================> ymf276_device

DECLARE_DEVICE_TYPE(YMF276, ymf276_device);

class ymf276_device : public ymfm_device_base<ymfm::ymf276>
{
public:
	// constructor
	ymf276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// additional register writes
	void address_hi_w(u8 data) { update_streams().write_address_hi(data); }
	void data_hi_w(u8 data) { update_streams().write_data_hi(data); }
};

#endif // MAME_SOUND_YMOPN_H
