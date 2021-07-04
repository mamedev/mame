// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_MAME_H
#define MAME_SOUND_YMFM_MAME_H

#pragma once

#include "ymfm/src/ymfm.h"
#include "ymfm/src/ymfm_ssg.h"


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

		// remember the busy end time
		save_item(NAME(m_busy_end));
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
// that wrap ymfm chips; it provides basic read/write functions
template<typename ChipClass>
class ymfm_device_base : public ym_generic_device
{
protected:
	static constexpr int OUTPUTS = ChipClass::OUTPUTS;

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


// ======================> ymfm_ssg_device_base

// this template adds SSG support to the base template, using ymfm's internal
// SSG implementation
template<typename ChipClass>
class ymfm_ssg_device_base : public ymfm_device_base<ChipClass>
{
	using parent = ymfm_device_base<ChipClass>;

public:
	// constructor
	ymfm_ssg_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type) :
		ymfm_device_base<ChipClass>(mconfig, tag, owner, clock, type)
	{
	}

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

#endif // MAME_SOUND_YMFM_H
