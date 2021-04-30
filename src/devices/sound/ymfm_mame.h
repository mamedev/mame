// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_MAME_H
#define MAME_SOUND_YMFM_MAME_H

#pragma once

#include "ymfm.h"
#include "ay8910.h"


//*********************************************************
//  MACROS
//*********************************************************

// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace (mostly necessary
// for chips which derive from AY-8910 classes and may have clashing
// names)
#define YMFM_NAME(x) x, "ymfm." #x



//*********************************************************
//  MAME INTERFACES
//*********************************************************

// ======================> ymfm_device_standalone_base

// inner base class for a standalone FM device
class ymfm_device_standalone_base : public device_t, public device_sound_interface
{
public:
	// constructor
	ymfm_device_standalone_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int aystreams = 0, int ayioports = 0) :
		device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this)
	{
	}

protected:
	// device overrides
	virtual void device_start() override
	{
	}

	// tell the chip we want to override the SSG (n/a in the standalone case)
	template<class ChipClass>
	void ssg_override(ChipClass &chip)
	{
	}
};


// ======================> ymfm_device_ay8910_base

// inner base class for an AY-8910-derived FM device
class ymfm_device_ay8910_base : public ay8910_device, public ymfm::ssg_interface
{
public:
	// constructor
	ymfm_device_ay8910_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int aystreams, int ayioports) :
		ay8910_device(mconfig, type, tag, owner, clock, PSG_TYPE_YM, aystreams, ayioports)
	{
	}

	// SSG overrides
	virtual void ssg_reset() override
	{
		ay8910_reset_ym();
	}

	virtual void ssg_set_clock_prescale(uint8_t clock_divider) override
	{
		ay_set_clock(clock() * 2 / clock_divider);
	}

	virtual uint8_t ssg_read(uint8_t offset) override
	{
		ay8910_write_ym(0, offset);
		return ay8910_read_ym();
	}

	virtual void ssg_write(uint8_t offset, uint8_t data) override
	{
		ay8910_write_ym(0, offset);
		ay8910_write_ym(1, data);
	}

protected:
	// tell the chip we want to override
	template<class ChipClass>
	void ssg_override(ChipClass &chip)
	{
		chip.ssg_override(*this);
	}
};


// ======================> ymfm_device_base_common

// this template provides most of the basics used by device objects in MAME
// that wrap ymfm chips; it implements the fm_interface, offers binding to
// external callbacks, and provides basic read/write functions; however, this
// class is not intended to be used directly -- rather, devices should inherit
// from eitehr ymfm_device_base or ymfm_ssg_device_base, depending on whether
// they include an SSG or not
template<typename BaseClass, typename ChipClass>
class ymfm_device_base_common : public BaseClass, public ymfm::fm_interface
{
public:
	// constructor
	ymfm_device_base_common(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int aystreams = 0, int ayioports = 0) :
		BaseClass(mconfig, tag, owner, clock, type, aystreams, ayioports),
		m_stream(nullptr),
		m_chip(*this),
		m_timer{ nullptr, nullptr },
		m_update_irq(*this),
		m_io_read{ *this, *this },
		m_io_write{ *this, *this }
	{
	}

	// configuration helpers, handled by the interface
	auto update_irq_handler() { return m_update_irq.bind(); }
	auto io_read_handler(int index = 0) { return m_io_read[index & 1].bind(); }
	auto io_write_handler(int index = 0) { return m_io_write[index & 1].bind(); }

	// read access, handled by the chip implementation
	u8 read(offs_t offset)
	{
		m_stream->update();
		return m_chip.read(offset);
	}

	// (almost) all chips have a status register
	u8 status_r()
	{
		m_stream->update();
		return m_chip.read_status();
	}

	// write access, handled by the chip implementation
	void write(offs_t offset, u8 data)
	{
		m_stream->update();
		m_chip.write(offset, data);
	}

	// (almost) all chips have a register/address latch
	void address_w(u8 data)
	{
		m_stream->update();
		m_chip.write_address(data);
	}

	// (almost) all chips have a data latch
	void data_w(u8 data)
	{
		m_stream->update();
		m_chip.write_data(data);
	}

	using BaseClass::machine;
	using BaseClass::stream_alloc;

protected:
	// perform a synchronized write
	virtual void ymfm_sync_mode_write(uint8_t data) override
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(ymfm_device_base_common::fm_mode_write), this), data);
	}

	// perform a synchronized interrupt check
	virtual void ymfm_sync_check_interrupts() override
	{
		// if we're currently executing a CPU, schedule the interrupt check;
		// otherwise, do it directly
		auto &scheduler = machine().scheduler();
		if (scheduler.currently_executing())
			scheduler.synchronize(timer_expired_delegate(FUNC(ymfm_device_base_common::fm_check_interrupts), this));
		else
			m_engine->engine_check_interrupts();
	}

	// set a timer
	virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override
	{
		if (duration_in_clocks >= 0)
			m_timer[tnum]->adjust(attotime::from_ticks(duration_in_clocks, device_t::clock()), tnum);
		else
			m_timer[tnum]->enable(false);
	}

	// set the time when busy will be clear
	virtual void ymfm_set_busy_end(uint32_t clocks) override
	{
		m_busy_end = machine().time() + attotime::from_ticks(clocks, device_t::clock());
	}

	// are we past the busy clear time?
	virtual bool ymfm_is_busy() override
	{
		return (machine().time() < m_busy_end);
	}

	// handle IRQ signaling
	virtual void ymfm_update_irq(bool asserted) override
	{
		if (!m_update_irq.isnull())
			m_update_irq(asserted ? ASSERT_LINE : CLEAR_LINE);
	}

	// handle prescale changing
	virtual void ymfm_prescale_changed() override
	{
		device_clock_changed();
	}

	// the engine calls this when a write to an output port is issued
	virtual void ymfm_io_write(uint8_t port, uint8_t data) override
	{
		if (!m_io_write[port & 1].isnull())
			m_io_write[port & 1](data);
	}

	// the engine calls this when a read from an input port is issued
	virtual uint8_t ymfm_io_read(uint8_t port) override
	{
		return m_io_read[port & 1].isnull() ? 0 : m_io_read[port & 1]();
	}

	// device-level overrides
	virtual void device_start() override
	{
		// call the parent
		BaseClass::device_start();

		// override the SSG, if there is one
		BaseClass::ssg_override(m_chip);

		// allocate our stream
		m_stream = stream_alloc(0, ChipClass::OUTPUTS, m_chip.sample_rate(device_t::clock()));

		// allocate our timers
		for (int tnum = 0; tnum < 2; tnum++)
			m_timer[tnum] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ymfm_device_base_common::fm_timer_handler), this));

		// resolve the handlers
		m_update_irq.resolve();
		m_io_read[0].resolve();
		m_io_read[1].resolve();
		m_io_write[0].resolve();
		m_io_write[1].resolve();

		// register for save states
		m_chip.register_save(*this);
	}

	virtual void device_reset() override
	{
		BaseClass::device_reset();
		m_chip.reset();
	}

	virtual void device_clock_changed() override
	{
		BaseClass::device_clock_changed();
		m_stream->set_sample_rate(m_chip.sample_rate(device_t::clock()));
	}

	virtual void device_post_load() override
	{
		BaseClass::device_post_load();
		m_chip.invalidate_caches();
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		// for AY8910-derived classes, pass on their stream updates
		if (&stream != m_stream)
			return BaseClass::sound_stream_update(stream, inputs, outputs);

		int32_t output[ChipClass::OUTPUTS];
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			m_chip.generate(output);
			for (int index = 0; index < ChipClass::OUTPUTS; index++)
				outputs[index].put_int(sampindex, output[index], 32768);
		}
	}

	// timer callbacks
	void fm_timer_handler(void *ptr, int param) { m_engine->engine_timer_expired(param); }
	void fm_mode_write(void *ptr, int param) { m_engine->engine_mode_write(param); }
	void fm_check_interrupts(void *ptr, int param) { m_engine->engine_check_interrupts(); }

	// internal state
	sound_stream *m_stream;          // sound stream
	ChipClass m_chip;                // core chip implementation
	attotime m_busy_end;             // busy end time
	emu_timer *m_timer[2];           // two timers
	devcb_write_line m_update_irq;   // IRQ update callback
	devcb_read8 m_io_read[2];        // up to 2 input port handlers
	devcb_write8 m_io_write[2];      // up to 2 output port handlers
};


// ======================> ymfm_device_base

template<typename ChipClass>
using ymfm_device_base = ymfm_device_base_common<ymfm_device_standalone_base, ChipClass>;


// ======================> ymfm_device_ssg_base

template<typename ChipClass>
using ymfm_device_ssg_base = ymfm_device_base_common<ymfm_device_ay8910_base, ChipClass>;


#endif // MAME_SOUND_YMFM_H
