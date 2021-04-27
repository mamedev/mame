// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2414_H
#define MAME_SOUND_YM2414_H

#pragma once

#include "ymfm_opz.h"


// ======================> opm_device_base

DECLARE_DEVICE_TYPE(YM2414, ym2414_device);
class ym2414_device : public device_t, public device_sound_interface
{
public:
	// constructor
	ym2414_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, YM2414, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_fm_intf(*this),
		m_stream(nullptr),
		m_fm(m_fm_intf)
	{
	}

	// configuration helpers, handled by the interface
	auto irq_handler() { return m_fm_intf.update_irq_handler(); }
	auto port_write_handler() { return m_fm_intf.output_port_handler(); }

	// read access, handled by the chip implementation
	u8 read(offs_t offset) { m_stream->update(); return m_fm.read(offset); }
	u8 status_r() { m_stream->update(); return m_fm.read_status(); }

	// write access, handled by the chip implementation
	void write(offs_t offset, u8 data) { m_stream->update(); m_fm.write(offset, data); }
	void register_w(u8 data) { m_stream->update(); m_fm.write_address(data); }
	void data_w(u8 data) { m_stream->update(); m_fm.write_data(data); }

protected:
	// device-level overrides
	virtual void device_start() override
	{
		m_stream = stream_alloc(0, m_fm.OUTPUTS, m_fm.sample_rate(clock()));
		m_fm_intf.start();
		m_fm.register_save(*this);
	}
	virtual void device_reset() override { m_fm.reset(); }
	virtual void device_clock_changed() override { m_stream->set_sample_rate(m_fm.sample_rate(clock())); }
	virtual void device_post_load() override { m_fm.invalidate_caches(); }

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		int32_t output[m_fm.OUTPUTS];
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			m_fm.generate(output);
			for (int index = 0; index < m_fm.OUTPUTS; index++)
				outputs[index].put_int(sampindex, output[index], 32768);
		}
	}

	// internal state
	mame_fm_interface m_fm_intf;     // FM interface
	sound_stream *m_stream;          // sound stream
	ymfm::ym2414 m_fm;               // core FM implementation
};

#endif // MAME_SOUND_YM2414_H
