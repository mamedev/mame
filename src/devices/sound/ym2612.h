// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2612_H
#define MAME_SOUND_YM2612_H

#pragma once

#include "ymfm.h"


// ======================> ym2612_device

DECLARE_DEVICE_TYPE(YM2612, ym2612_device);

class ym2612_device : public device_t, public device_sound_interface
{
public:
	// constructor
	ym2612_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YM2612);

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

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal helpers
	void sound_stream_update_common(write_stream_view &outl, write_stream_view &outr, bool discontinuity);

	// internal state
	ymopna_engine m_opn;             // core OPN engine
	sound_stream *m_stream;          // sound stream
	attotime m_busy_duration;        // precomputed busy signal duration
	u16 m_address;                   // address register
	u16 m_dac_data;                  // 9-bit DAC data
	u8 m_dac_enable;                 // DAC enabled?
	u8 m_channel;                    // current multiplexed channel
};


// ======================> ym3438_device

DECLARE_DEVICE_TYPE(YM3438, ym3438_device);

class ym3438_device : public ym2612_device
{
public:
	// constructor
	ym3438_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
};


// ======================> ymf276_device

DECLARE_DEVICE_TYPE(YMF276, ymf276_device);

class ymf276_device : public ym2612_device
{
public:
	// constructor
	ymf276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_clock_changed() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
};


#endif // MAME_SOUND_YM2612_H
