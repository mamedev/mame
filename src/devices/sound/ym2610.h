// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2610_H
#define MAME_SOUND_YM2610_H

#pragma once

#include "ymfm_mame.h"
#include "ymfm_opn.h"


// ======================> ym2610_device_base

template<typename ChipClass>
class ym2610_device_base : public ymfm_device_base<ChipClass, ChipClass::SSG_OUTPUTS>, public device_memory_interface
{
	using parent = ymfm_device_base<ChipClass, 1>;

public:
	// constructor
	ym2610_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

protected:
	// device-level overrides
	virtual void device_start() override;

	// memory space configuration
	virtual space_config_vector memory_space_config() const override;

private:
	// ADPCM read/write callbacks
	virtual uint8_t adpcm_a_read(offs_t address) override;
	virtual uint8_t adpcm_b_read(offs_t address) override;

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


#endif // MAME_SOUND_YM2610_H
