// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2151_H
#define MAME_SOUND_YM2151_H

#pragma once

#include "ymfm_opm.h"


// ======================> opm_device_base

template<typename ChipClass>
class opm_device_base : public device_t, public device_sound_interface
{
public:
	// constructor
	opm_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		device_t(mconfig, type, tag, owner, clock),
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
	u8 read(offs_t offset)
	{
		m_stream->update();
		return m_fm.read(offset);
	}
	u8 status_r()
	{
		m_stream->update();
		return m_fm.read_status();
	}

	// write access, handled by the chip implementation
	void write(offs_t offset, u8 data)
	{
		if (m_reset_state == 0)
			return;
		m_stream->update();
		m_fm.write(offset, data);
	}
	void register_w(u8 data)
	{
		if (m_reset_state == 0)
			return;
		m_stream->update();
		m_fm.write_address(data);
	}
	void data_w(u8 data)
	{
		if (m_reset_state == 0)
			return;
		m_stream->update();
		m_fm.write_data(data);
	}

	// reset line, active LOW
	DECLARE_WRITE_LINE_MEMBER(reset_w)
	{
		if (state != m_reset_state)
		{
			m_stream->update();
			m_reset_state = state;
			if (state != 0)
				m_fm.reset();
		}
	}

protected:
	// device-level overrides
	virtual void device_start() override
	{
		m_stream = stream_alloc(0, ChipClass::OUTPUTS, m_fm.sample_rate(clock()));
		m_fm_intf.start();
		m_fm.register_save(*this);
	}
	virtual void device_reset() override { m_fm.reset(); }
	virtual void device_clock_changed() override { m_stream->set_sample_rate(m_fm.sample_rate(clock())); }
	virtual void device_post_load() override { m_fm.invalidate_caches(); }

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		int32_t output[ChipClass::OUTPUTS];
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			m_fm.generate(output);
			for (int index = 0; index < ChipClass::OUTPUTS; index++)
				outputs[index].put_int(sampindex, output[index], 32768);
		}
	}

	// internal state
	mame_fm_interface m_fm_intf;     // FM interface
	sound_stream *m_stream;          // sound stream
	uint8_t m_reset_state;           // reset line state
	ChipClass m_fm;                  // core FM implementation
};


// ======================> ym2151_device

DECLARE_DEVICE_TYPE(YM2151, ym2151_device);
class ym2151_device : public opm_device_base<ymfm::ym2151>
{
public:
	ym2151_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		opm_device_base(mconfig, tag, owner, clock, YM2151)
	{
	}
};


// ======================> ym2164_device

DECLARE_DEVICE_TYPE(YM2164, ym2164_device);
class ym2164_device : public opm_device_base<ymfm::ym2164>
{
public:
	ym2164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		opm_device_base(mconfig, tag, owner, clock, YM2164)
	{
	}
};

#endif // MAME_SOUND_YM2151_H
