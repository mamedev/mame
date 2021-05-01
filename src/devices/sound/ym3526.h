// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM3526_H
#define MAME_SOUND_YM3526_H

#pragma once

#include "ymfm.h"


// ======================> ym3526_device

DECLARE_DEVICE_TYPE(YM3526, ym3526_device);

class ym3526_device : public device_t, public device_sound_interface
{
public:
	// YM3526 is OPL
	using fm_engine = ymopl_engine;

	// constructor
	ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type = YM3526);

	// configuration helpers
	auto irq_handler() { return m_fm.irq_handler(); }

	// read access
	u8 status_r();  // A0=0
	u8 read(offs_t offset);

	// write access
	void address_w(u8 data);    // A0=0
	void data_w(u8 data);       // A0=1
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// internal state
	u8 m_address;                    // address register
	sound_stream *m_stream;          // sound stream
	fm_engine m_fm;                  // core FM engine
};


#endif // MAME_SOUND_YM3526_H
