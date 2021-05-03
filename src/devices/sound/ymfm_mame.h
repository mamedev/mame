// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_MAME_H
#define MAME_SOUND_YMFM_MAME_H

#pragma once

#include "ymfm.h"
#include "ymfm_ssg.h"
#include "ay8910.h"


// set this to 1 to use ymfm's built-in SSG implementation
// set it to 0 to use MAME's ay8910 as the SSG implementation
#define USE_BUILTIN_SSG (0)


//*********************************************************
//  MACROS
//*********************************************************

// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace
#define YMFM_NAME(x) x, "ymfm." #x



//*********************************************************
//  MAME INTERFACES
//*********************************************************

// ======================> ym_generic

// generic base class for a standalone FM device; this class contains the shared
// configuration helpers, timers, and ymfm interface implementation; it also
// specifies pure virtual functions for read/write access, which means it
// can be used as a generic proxy for systems that have multiple FM types that are
// swappable
class ym_generic : public device_t, public device_sound_interface, public ymfm::ymfm_interface
{
public:
	// constructor
	ym_generic(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_timer{ nullptr, nullptr },
		m_update_irq(*this),
		m_io_read{ *this, *this },
		m_io_write{ *this, *this }
	{
	}

	// configuration helpers
	auto irq_handler() { return m_update_irq.bind(); }
	auto io_read_handler(int index = 0) { return m_io_read[index & 1].bind(); }
	auto io_write_handler(int index = 0) { return m_io_write[index & 1].bind(); }

	// read access interface, implemented by the derived chip-specific class
	virtual u8 read(offs_t offset) = 0;
	virtual u8 status_r() = 0;

	// write access interface, implemented by the derived chip-specific class
	virtual void write(offs_t offset, u8 data) = 0;
	virtual void address_w(u8 data) = 0;
	virtual void data_w(u8 data) = 0;

protected:
	// the chip implementation calls this when a write happens to the mode
	// register, which could affect timers and interrupts; our responsibility
	// is to ensure the system is up to date before calling the engine's
	// engine_mode_write() method
	virtual void ymfm_sync_mode_write(uint8_t data) override
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(ym_generic::fm_mode_write), this), data);
	}

	// the chip implementation calls this when the chip's status has changed,
	// which may affect the interrupt state; our responsibility is to ensure
	// the system is up to date before calling the engine's
	// engine_check_interrupts() method
	virtual void ymfm_sync_check_interrupts() override
	{
		// if we're currently executing a CPU, schedule the interrupt check;
		// otherwise, do it directly
		auto &scheduler = machine().scheduler();
		if (scheduler.currently_executing())
			scheduler.synchronize(timer_expired_delegate(FUNC(ym_generic::fm_check_interrupts), this));
		else
			m_engine->engine_check_interrupts();
	}

	// the chip implementation calls this when one of the two internal timers
	// has changed state; our responsibility is to arrange to call the engine's
	// engine_timer_expired() method after the provided number of clocks; if
	// duration_in_clocks is negative, we should cancel any outstanding timers
	virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) override
	{
		if (duration_in_clocks >= 0)
			m_timer[tnum]->adjust(attotime::from_ticks(duration_in_clocks, device_t::clock()), tnum);
		else
			m_timer[tnum]->enable(false);
	}

	// the chip implementation calls this when the state of the IRQ signal has
	// changed due to a status change; our responsibility is to respons as
	// needed to the change in IRQ state, signaling any consumers
	virtual void ymfm_update_irq(bool asserted) override
	{
		if (!m_update_irq.isnull())
			m_update_irq(asserted ? ASSERT_LINE : CLEAR_LINE);
	}

	// the chip implementation calls this to indicate that the chip should be
	// considered in a busy state until the given number of clocks has passed;
	// our responsibility is to compute and remember the ending time based on
	// the chip's clock for later checking
	virtual void ymfm_set_busy_end(uint32_t clocks) override
	{
		m_busy_end = machine().time() + attotime::from_ticks(clocks, device_t::clock());
	}

	// the chip implementation calls this to see if the chip is still currently
	// is a busy state, as specified by a previous call to ymfm_set_busy_end();
	// our responsibility is to compare the current time against the previously
	// noted busy end time and return true if we haven't yet passed it
	virtual bool ymfm_is_busy() override
	{
		return (machine().time() < m_busy_end);
	}

	// the chip implementation calls this whenever the internal clock prescaler
	// changes; our responsibility is to adjust our clocking of the chip in
	// response to produce the correct output rate
	virtual void ymfm_prescale_changed() override
	{
		device_clock_changed();
	}

	// the chip implementation calls this whenever a new value is written to
	// one of the chip's output ports (only applies to some chip types); our
	// responsibility is to pass the written data on to any consumers
	virtual void ymfm_io_write(uint8_t port, uint8_t data) override
	{
		if (!m_io_write[port & 1].isnull())
			m_io_write[port & 1](data);
	}

	// the chip implementation calls this whenever an on-chip register is read
	// which returns data from one of the chip's input ports; our responsibility
	// is to produce the current input value so that it can be reflected by the
	// read operation
	virtual uint8_t ymfm_io_read(uint8_t port) override
	{
		return m_io_read[port & 1].isnull() ? 0 : m_io_read[port & 1]();
	}

	// handle device start
	virtual void device_start() override
	{
		// allocate our timers
		for (int tnum = 0; tnum < 2; tnum++)
			m_timer[tnum] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ym_generic::fm_timer_handler), this));

		// resolve the handlers
		m_update_irq.resolve();
		m_io_read[0].resolve();
		m_io_read[1].resolve();
		m_io_write[0].resolve();
		m_io_write[1].resolve();
	}

	// timer callbacks
	void fm_mode_write(void *ptr, int param) { m_engine->engine_mode_write(param); }
	void fm_check_interrupts(void *ptr, int param) { m_engine->engine_check_interrupts(); }
	void fm_timer_handler(void *ptr, int param) { m_engine->engine_timer_expired(param); }

	// internal state
	attotime m_busy_end;             // busy end time
	emu_timer *m_timer[2];           // two timers
	devcb_write_line m_update_irq;   // IRQ update callback
	devcb_read8 m_io_read[2];        // up to 2 input port handlers
	devcb_write8 m_io_write[2];      // up to 2 output port handlers
};


// ======================> ymfm_device_base

// this template provides most of the basics used by device objects in MAME
// that wrap ymfm chips; it implements the ymfm_interface, offers binding to
// external callbacks, and provides basic read/write functions; however, this
// class is not intended to be used directly -- rather, devices should inherit
// from eitehr ymfm_device_base or ymfm_ssg_device_base, depending on whether
// they include an SSG or not
template<typename ChipClass>
class ymfm_device_base : public ym_generic
{
public:
	// constructor
	ymfm_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ym_generic(mconfig, tag, owner, clock, type),
		m_stream(nullptr),
		m_chip(*this)
	{
	}

	// read access, handled by the chip implementation
	virtual u8 read(offs_t offset) override
	{
		update_streams();
		return m_chip.read(offset);
	}

	// (almost) all chips have a status register
	virtual u8 status_r() override
	{
		update_streams();
		return m_chip.read_status();
	}

	// write access, handled by the chip implementation
	virtual void write(offs_t offset, u8 data) override
	{
		update_streams();
		m_chip.write(offset, data);
	}

	// (almost) all chips have a register/address latch
	virtual void address_w(u8 data) override
	{
		update_streams();
		m_chip.write_address(data);
	}

	// (almost) all chips have a data latch
	virtual void data_w(u8 data) override
	{
		update_streams();
		m_chip.write_data(data);
	}

protected:
	// handle device start
	virtual void device_start() override
	{
		// let our parent do its startup
		ym_generic::device_start();

		// allocate our stream
		m_stream = device_sound_interface::stream_alloc(0, ChipClass::OUTPUTS, m_chip.sample_rate(device_t::clock()));

		// register for save states
		m_chip.register_save(*this);
	}

	// device reset
	virtual void device_reset() override
	{
		m_chip.reset();
	}

	// handle clock changed
	virtual void device_clock_changed() override
	{
		m_stream->set_sample_rate(m_chip.sample_rate(device_t::clock()));
	}

	virtual void device_post_load() override
	{
		m_chip.invalidate_caches();
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		// generate the FM/ADPCM stream
		int32_t output[ChipClass::OUTPUTS];
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			m_chip.generate(output);
			for (int index = 0; index < ChipClass::OUTPUTS; index++)
				outputs[index].put_int(sampindex, output[index], 32768);
		}
	}

	// update streams
	virtual void update_streams()
	{
		m_stream->update();
	}

	// internal state
	sound_stream *m_stream;          // sound stream
	ChipClass m_chip;                // core chip implementation
};


// ======================> ymfm_ssg_internal_device_base

// this template adds SSG support to the base template, using ymfm's internal
// SSG implementation
template<typename ChipClass>
class ymfm_ssg_internal_device_base : public ymfm_device_base<ChipClass>
{
	using parent = ymfm_device_base<ChipClass>;
	using parent::m_chip;

public:
	// constructor
	ymfm_ssg_internal_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ymfm_device_base<ChipClass>(mconfig, tag, owner, clock, type),
		m_ssg_stream(nullptr)
	{
	}

protected:
	// handle device start
	virtual void device_start() override
	{
		// SSG streams are expected to be first, so allocate our stream then
		// call the parent afterwards
		m_ssg_stream = device_sound_interface::stream_alloc(0, ChipClass::SSG_OUTPUTS, m_chip.sample_rate_ssg(device_t::clock()));\
		parent::device_start();
	}

	// handle clock changed
	virtual void device_clock_changed() override
	{
		parent::device_clock_changed();

		// update our sample rate
		m_ssg_stream->set_sample_rate(m_chip.sample_rate_ssg(device_t::clock()));
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		// if not the SSG stream, pass it along to our parent
		if (&stream != m_ssg_stream)
			return parent::sound_stream_update(stream, inputs, outputs);

		// generate samples until the buffer is filled
		int32_t output[ChipClass::SSG_OUTPUTS];
		for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
		{
			m_chip.generate_ssg(output);
			for (int index = 0; index < ChipClass::SSG_OUTPUTS; index++)
				outputs[index].put_int(sampindex, output[index], 32768);
		}
	}

	// internal helper to update all our streams
	virtual void update_streams() override
	{
		parent::update_streams();
		m_ssg_stream->update();
	}

	// internal state
	sound_stream *m_ssg_stream;      // SSG sound stream
};


// ======================> ymfm_ssg_external_device_base

// this template adds SSG support to the base template, using MAME's YM2149
// implementation in ay8910.cpp; this is the "classic" way to do it in MAME
// and is more flexible in terms of output handling
template<typename ChipClass>
class ymfm_ssg_external_device_base : public ymfm_device_base<ChipClass>, public ymfm::ssg_override
{
	using parent = ymfm_device_base<ChipClass>;
	using parent::m_chip;
	using parent::m_io_read;
	using parent::m_io_write;

public:
	// constructor
	ymfm_ssg_external_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ymfm_device_base<ChipClass>(mconfig, tag, owner, clock, type),
		m_ssg_stream(nullptr),
		m_ssg(*this, "ssg")
	{
	}

protected:
	// SSG overrides
	virtual void ssg_reset() override
	{
		m_ssg->reset();
	}

	virtual uint8_t ssg_read(uint32_t offset) override
	{
		m_ssg->address_w(offset);
		return m_ssg->data_r();
	}

	virtual void ssg_write(uint32_t offset, uint8_t data) override
	{
		m_ssg->address_w(offset);
		m_ssg->data_w(data);
	}

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override
	{
		AY8910(config, m_ssg, device_t::clock());
		m_ssg->set_psg_type(ay8910_device::PSG_TYPE_YM);
		m_ssg->set_flags(((ChipClass::SSG_OUTPUTS == 1) ? AY8910_SINGLE_OUTPUT : 0) | AY8910_LEGACY_OUTPUT);

		// configure the callbacks to route through our callbacks
		m_ssg->port_a_read_callback().set(FUNC(ymfm_ssg_external_device_base::io_reader<0>));
		m_ssg->port_a_write_callback().set(FUNC(ymfm_ssg_external_device_base::io_writer<0>));
		m_ssg->port_b_read_callback().set(FUNC(ymfm_ssg_external_device_base::io_reader<1>));
		m_ssg->port_b_write_callback().set(FUNC(ymfm_ssg_external_device_base::io_writer<1>));

		// route outputs through us
		m_ssg->add_route(0, *this, 1.0, 0);
		if (ChipClass::SSG_OUTPUTS > 1)
		{
			m_ssg->add_route(1, *this, 1.0, 1);
			m_ssg->add_route(2, *this, 1.0, 2);
		}
	}

	// handle device start
	virtual void device_start() override
	{
		// to use the YM2149 in MAME, we allocate our stream with the same number of inputs
		// and outputs; in our update handler we'll just forward each output from the
		// embedded YM2149 device through our stream to make it look like it used to when
		// we were inheriting from ay8910_device
		m_ssg_stream = device_sound_interface::stream_alloc(ChipClass::SSG_OUTPUTS, ChipClass::SSG_OUTPUTS, SAMPLE_RATE_INPUT_ADAPTIVE);

		// also tell the chip we want to override reads & writes
		m_chip.ssg_override(*this);

		// SSG streams are expected to be first, so call the parent afterwards
		parent::device_start();
	}

	// handle clock changed
	virtual void device_clock_changed() override
	{
		parent::device_clock_changed();

		// derive the effective clock from the computed sample rate
		m_ssg->set_unscaled_clock(ymfm::ssg_engine::CLOCK_DIVIDER * m_chip.sample_rate_ssg(device_t::clock()));
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		// if not the SSG stream, pass it along to our parent
		if (&stream != m_ssg_stream)
			return parent::sound_stream_update(stream, inputs, outputs);

		// just copy the streams from the SSG
		for (int index = 0; index < ChipClass::SSG_OUTPUTS; index++)
			outputs[index] = inputs[index];
	}

	// internal helper to update all our streams
	virtual void update_streams() override
	{
		parent::update_streams();
		m_ssg_stream->update();
	}

	template<int Index>
	uint8_t io_reader()
	{
		return m_io_read[Index].isnull() ? 0 : m_io_read[Index](0);
	}

	template<int Index>
	void io_writer(uint8_t data)
	{
		if (!m_io_write[Index].isnull())
			m_io_write[Index](0, data);
	}

	// internal state
	sound_stream *m_ssg_stream;           // SSG sound stream
	required_device<ay8910_device> m_ssg; // our embedded SSG device
};


// now pick the right one
#if USE_BUILTIN_SSG
template<typename ChipClass>
using ymfm_ssg_device_base = ymfm_ssg_internal_device_base<ChipClass>;
#else
template<typename ChipClass>
using ymfm_ssg_device_base = ymfm_ssg_external_device_base<ChipClass>;
#endif


#endif // MAME_SOUND_YMFM_H
