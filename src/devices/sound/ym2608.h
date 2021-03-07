// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2608_H
#define MAME_SOUND_YM2608_H

#pragma once

#include "ymfm.h"
#include "ymadpcm.h"
#include "ay8910.h"


// ======================> ym2608_device

DECLARE_DEVICE_TYPE(YM2608, ym2608_device);

class ym2608_device : public ay8910_device, public device_rom_interface<21>
{
public:
	// constructor
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_opn.irq_handler(); }

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// ROM device overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void rom_bank_updated() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// set a new prescale value and update clocks
	void update_prescale(u8 newval);

	// combine ADPCM and OPN statuses
	u8 combine_status();

	// ADPCM read/write callbacks
	u8 adpcm_a_read(offs_t address);
	u8 adpcm_b_read(offs_t address);
	void adpcm_b_write(offs_t address, u8 data);

	// internal state
	required_memory_region m_internal; // internal memory region
	ymopna_engine m_opn;             // core OPNA engine
	ymadpcm_a_engine m_adpcm_a;      // ADPCM-A engine
	ymadpcm_b_engine m_adpcm_b;      // ADPCM-B engine
	sound_stream *m_stream;          // sound stream
	attotime m_busy_duration;        // precomputed busy signal duration
	u16 m_address;                   // address register
	u8 m_irq_enable;                 // IRQ enable register
	u8 m_flag_control;               // flag control register
};

#endif // MAME_SOUND_YM2608_H
