// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_MAME_H
#define MAME_SOUND_YMFM_MAME_H

#pragma once

#include "ymfm/src/ymfm.h"
#include "ymfm/src/ymfm_ssg.h"
#include "ay8910.h"


// set this to 1 to use ymfm's built-in SSG implementation
// set it to 0 to use MAME's ay8910 as the SSG implementation
#define USE_BUILTIN_SSG (1)

// set this to control the output sample rate for SSG-based chips
#define SSG_FIDELITY (ymfm::OPN_FIDELITY_MED)



//*********************************************************
//  MAME INTERFACES
//*********************************************************

// ======================> ym_generic_device

// generic base class for a standalone FM device; this class contains the shared
// configuration helpers, timers, and ymfm interface implementation; it also
// specifies pure virtual functions for read/write access, which means it
// can be used as a generic proxy for systems that have multiple FM types that are
// swappable
class ym_generic_device : public device_t, public device_sound_interface, public ymfm::ymfm_interface
{
public:
	// constructor
	ym_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
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
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(ym_generic_device::fm_mode_write), this), data);
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
			scheduler.synchronize(timer_expired_delegate(FUNC(ym_generic_device::fm_check_interrupts), this));
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

	// the chip implementation calls this whenever data is read from outside
	// of the chip; our responsibility is to provide the data requested
	virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address) override
	{
		return (type != ymfm::ACCESS_IO || m_io_read[address & 1].isnull()) ? 0 : m_io_read[address & 1]();
	}

	// the chip implementation calls this whenever data is written outside
	// of the chip; our responsibility is to pass the written data on to any consumers
	virtual void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) override
	{
		if (type == ymfm::ACCESS_IO && !m_io_write[address & 1].isnull())
			m_io_write[address & 1](data);
	}

	// handle device start
	virtual void device_start() override
	{
		// allocate our timers
		for (int tnum = 0; tnum < 2; tnum++)
			m_timer[tnum] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ym_generic_device::fm_timer_handler), this));

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
// that wrap ymfm chips; it provides basic read/write functions; however, this
// class is not intended to be used directly -- rather, devices should inherit
// from either ymfm_device_base or ymfm_ssg_device_base, depending on whether
// they include an SSG or not
template<typename ChipClass, bool FMOnly = false>
class ymfm_device_base : public ym_generic_device
{
protected:
	// for SSG chips, we only create a subset of outputs here:
	// YM2203 is 4 outputs: 1 mono FM + 3 SSG
	// YM2608/2610 is 3 outputs: 2 stereo FM + 1 SSG
	static constexpr int OUTPUTS = FMOnly ? ((ChipClass::OUTPUTS == 4) ? 1 : 2) : ChipClass::OUTPUTS;

public:
	// constructor
	ymfm_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ym_generic_device(mconfig, tag, owner, clock, type),
		m_stream(nullptr),
		m_chip(*this)
	{
	}

	// read access: update the streams before performing the read
	virtual u8 read(offs_t offset) override { return update_streams().read(offset); }
	virtual u8 status_r() override { return update_streams().read_status(); }

	// write access: update the strams before performing the write
	virtual void write(offs_t offset, u8 data) override { update_streams().write(offset, data); }
	virtual void address_w(u8 data) override { update_streams().write_address(data); }
	virtual void data_w(u8 data) override { update_streams().write_data(data); }

protected:
	// handle device start
	virtual void device_start() override
	{
		// let our parent do its startup
		ym_generic_device::device_start();

		// allocate our stream
		m_stream = device_sound_interface::stream_alloc(0, OUTPUTS, m_chip.sample_rate(device_t::clock()));

		// compute the size of the save buffer by doing an initial save
		ymfm::ymfm_saved_state state(m_save_blob, true);
		m_chip.save_restore(state);

		// now register the blob for save, on the assumption the size won't change
		save_item(NAME(m_save_blob));
	}

	// device reset
	virtual void device_reset() override
	{
		m_chip.reset();
	}

	// handle clock changed
	virtual void device_clock_changed() override
	{
		if (m_stream != nullptr)
			m_stream->set_sample_rate(m_chip.sample_rate(device_t::clock()));
	}

	// handle pre-saving by filling the blob
	virtual void device_pre_save() override
	{
		// remember the original blob size
		auto orig_size = m_save_blob.size();

		// save the state
		ymfm::ymfm_saved_state state(m_save_blob, true);
		m_chip.save_restore(state);

		// ensure that the size didn't change since we first allocated
		if (m_save_blob.size() != orig_size)
			throw emu_fatalerror("State size changed for ymfm chip");
	}

	// handle post-loading by restoring from the blob
	virtual void device_post_load() override
	{
		// populate the state from the blob
		ymfm::ymfm_saved_state state(m_save_blob, false);
		m_chip.save_restore(state);
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		update_internal(outputs);
	}

	// update streams
	virtual ChipClass &update_streams()
	{
		m_stream->update();
		return m_chip;
	}

	// internal update helper
	void update_internal(std::vector<write_stream_view> &outputs, int output_shift = 0)
	{
		// local buffer to hold samples
		constexpr int MAX_SAMPLES = 256;
		typename ChipClass::output_data output[MAX_SAMPLES];

		// parameters
		int const outcount = std::min(outputs.size(), std::size(output[0].data));
		int const numsamples = outputs[0].samples();

		// generate the FM/ADPCM stream
		for (int sampindex = 0; sampindex < numsamples; sampindex += MAX_SAMPLES)
		{
			int cursamples = std::min(numsamples - sampindex, MAX_SAMPLES);
			m_chip.generate(output, cursamples);
			for (int outnum = 0; outnum < outcount; outnum++)
			{
				int eff_outnum = (outnum + output_shift) % OUTPUTS;
				for (int index = 0; index < cursamples; index++)
					outputs[eff_outnum].put_int(sampindex + index, output[index].data[outnum], 32768);
			}
		}
	}

	// internal state
	sound_stream *m_stream;           // sound stream
	ChipClass m_chip;                 // core chip implementation
	std::vector<uint8_t> m_save_blob; // state saving buffer
};


// ======================> ymfm_ssg_internal_device_base

// this template adds SSG support to the base template, using ymfm's internal
// SSG implementation
template<typename ChipClass>
class ymfm_ssg_internal_device_base : public ymfm_device_base<ChipClass>
{
	using parent = ymfm_device_base<ChipClass>;

public:
	// constructor
	ymfm_ssg_internal_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ymfm_device_base<ChipClass>(mconfig, tag, owner, clock, type)
	{
	}

	// configuration helpers
	void set_flags(int flags) { /* not supported when using internal SSG */ }

protected:
	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		// ymfm outputs FM first, then SSG, while MAME traditionally
		// wants SSG streams first; to do this, we rotate the outputs
		// by the number of SSG output channels
		parent::update_internal(outputs, ChipClass::SSG_OUTPUTS);

		// for the single-output case, also apply boost the gain to better match
		// previous version, which summed instead of averaged the outputs
		if (ChipClass::SSG_OUTPUTS == 1)
			outputs[0].apply_gain(3.0);
	}
};


// ======================> ymfm_ssg_external_device_base

// this template adds SSG support to the base template, using MAME's YM2149
// implementation in ay8910.cpp; this is the "classic" way to do it in MAME
// and is more flexible in terms of output handling
template<typename ChipClass>
class ymfm_ssg_external_device_base : public ymfm_device_base<ChipClass, true>, public ymfm::ssg_override
{
	using parent = ymfm_device_base<ChipClass, true>;

public:
	// constructor
	ymfm_ssg_external_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ymfm_device_base<ChipClass, true>(mconfig, tag, owner, clock, type),
		m_ssg_stream(nullptr),
		m_ssg(*this, "ssg"),
		m_ssg_flags(((ChipClass::SSG_OUTPUTS == 1) ? AY8910_SINGLE_OUTPUT : 0) | AY8910_LEGACY_OUTPUT)
	{
	}

	// configuration helpers
	void set_flags(int flags)
	{
		// don't allow some flags to be changed: there is no pin26 in the embedded chip,
		// and the number of outputs is configured by the chip itself
		flags &= ~(AY8910_SINGLE_OUTPUT | YM2149_PIN26_LOW);
		flags |= m_ssg_flags & AY8910_SINGLE_OUTPUT;
		m_ssg_flags = flags;
		if (m_ssg)
			m_ssg->set_flags(m_ssg_flags);
	}

protected:
	using parent::m_chip;
	using parent::m_io_read;
	using parent::m_io_write;

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

	virtual void ssg_prescale_changed() override
	{
		device_clock_changed();
	}

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override
	{
		YM2149(config, m_ssg, device_t::clock());
		m_ssg->set_flags(m_ssg_flags);

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
		m_ssg->set_unscaled_clock(m_chip.ssg_effective_clock(device_t::clock()));
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
	virtual ChipClass &update_streams() override
	{
		m_ssg_stream->update();
		return parent::update_streams();
	}

	// I/O reader trampoline
	template<int Index>
	uint8_t io_reader()
	{
		return m_io_read[Index].isnull() ? 0 : m_io_read[Index](0);
	}

	// I/O writer trampoline
	template<int Index>
	void io_writer(uint8_t data)
	{
		if (!m_io_write[Index].isnull())
			m_io_write[Index](0, data);
	}

	// internal state
	sound_stream *m_ssg_stream;           // SSG sound stream
	required_device<ay8910_device> m_ssg; // our embedded SSG device
	int m_ssg_flags;                      // SSG flags
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
