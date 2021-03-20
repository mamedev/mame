// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMF262_H
#define MAME_SOUND_YMF262_H

#pragma once

#include "ymfm.h"


// ======================> ymf262_device

DECLARE_DEVICE_TYPE(YMF262, ymf262_device);

class ymf262_device : public device_t, public device_sound_interface
{
public:
	// YMF262 is OPL3
	using fm_engine = ymopl3_engine;

	// constructor
	ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YMF262);

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

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal state
	u16 m_address;                   // address register
	sound_stream *m_stream;          // sound stream
	fm_engine m_fm;                  // core FM engine
};


#endif // MAME_SOUND_YMF262_H
