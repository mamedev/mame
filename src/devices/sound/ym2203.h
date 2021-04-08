// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2203_H
#define MAME_SOUND_YM2203_H

#pragma once

#include "ymfm.h"
#include "ay8910.h"


// ======================> ym2203_device

DECLARE_DEVICE_TYPE(YM2203, ym2203_device);

class ym2203_device : public ay8910_device
{
public:
	// YM2151 is OPN
	using fm_engine = ymopn_engine;

	// constructor
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_fm.irq_handler(); }

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// direct port access
	u8 status_port_r() { return read(0); }
	u8 read_port_r() { return read(1); }
	void control_port_w(u8 data) { write(0, data); }
	void write_port_w(u8 data) { write(1, data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// set a new prescale value and update clocks
	void update_prescale(u8 newval);

	// internal state
	fm_engine m_fm;                  // core FM engine
	sound_stream *m_stream;          // sound stream
	attotime m_busy_duration;        // precomputed busy signal duration
	u8 m_address;                    // address register
};

#endif // MAME_SOUND_YM2203_H
