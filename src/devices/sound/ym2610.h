// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2610_H
#define MAME_SOUND_YM2610_H

#pragma once

#include "ymfm.h"
#include "ymadpcm.h"
#include "ay8910.h"


// ======================> ym2610_device

DECLARE_DEVICE_TYPE(YM2610, ym2610_device);

class ym2610_device : public ay8910_device, public device_memory_interface
{
public:
	// YM2610 is OPNA
	using fm_engine = ymopna_engine;

	// constructor
	ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YM2610, u8 fm_mask = 0x36);

	// configuration helpers
	auto irq_handler() { return m_fm.irq_handler(); }

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// memory space configuration
	virtual space_config_vector memory_space_config() const override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// ADPCM read/write callbacks
	u8 adpcm_a_read(offs_t address);
	u8 adpcm_b_read(offs_t address);

	// internal state
	address_space_config const m_adpcm_a_config; // address space 0 config (ADPCM-A)
	address_space_config const m_adpcm_b_config; // address space 1 config (ADPCM-B)
	optional_memory_region m_adpcm_a_region; // ADPCM-A memory region
	optional_memory_region m_adpcm_b_region; // ADPCM-B memory region
	fm_engine m_fm;                  // core FM engine
	ymadpcm_a_engine m_adpcm_a;      // ADPCM-A engine
	ymadpcm_b_engine m_adpcm_b;      // ADPCM-B engine
	sound_stream *m_stream;          // sound stream
	attotime m_busy_duration;        // precomputed busy signal duration
	u16 m_address;                   // address register
	u8 const m_fm_mask;              // FM channel mask
	u8 m_eos_status;                 // end-of-sample signals
	u8 m_flag_mask;                  // flag mask control
};


// ======================> ym2610b_device

DECLARE_DEVICE_TYPE(YM2610B, ym2610b_device);

class ym2610b_device : public ym2610_device
{
public:
	// constructor
	ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif // MAME_SOUND_YM2610_H
