// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2151_H
#define MAME_SOUND_YM2151_H

#pragma once

#include "ymfm.h"


// ======================> ym2151_device

DECLARE_DEVICE_TYPE(YM2151, ym2151_device);

class ym2151_device : public device_t, public device_sound_interface
{
public:
	// constructor
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YM2151);

	// configuration helpers
	auto irq_handler() { return m_opm.irq_handler(); }
	auto port_write_handler() { return m_port_w.bind(); }

	// read/write access
	u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	u8 status_r() { return read(1); }
	void register_w(u8 data) { write(0, data); }
	void data_w(u8 data) { write(1, data); }

	DECLARE_WRITE_LINE_MEMBER(reset_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal state
	ymopm_engine m_opm;              // core OPM engine
	sound_stream *m_stream;          // sound stream
	devcb_write8 m_port_w;           // port write handler
	attotime m_busy_duration;        // precomputed busy signal duration
	u8 m_address;                    // address register
	u8 m_reset_state;                // reset state
};


// ======================> ym2164_device

DECLARE_DEVICE_TYPE(YM2164, ym2164_device);

class ym2164_device : public ym2151_device
{
public:
	// constructor
	ym2164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write access
	virtual void write(offs_t offset, u8 data) override;
};


// ======================> ym2414_device

DECLARE_DEVICE_TYPE(YM2414, ym2414_device);

class ym2414_device : public ym2151_device
{
public:
	// constructor
	ym2414_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif // MAME_SOUND_YM2151_H
