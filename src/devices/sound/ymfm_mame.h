// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_MAME_H
#define MAME_SOUND_YMFM_MAME_H

#pragma once

#include "ymfm.h"
#include "ymfm_ssg.h"
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

// ======================> ym_generic

// inner base class for a standalone FM device
class ym_generic : public device_t, public device_sound_interface, public ymfm::fm_interface, public ymfm::ssg_override
{
public:
	// constructor
	ym_generic(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int aystreams = 0, int ayioports = 0) :
		device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_timer{ nullptr, nullptr },
		m_update_irq(*this),
		m_io_read{ *this, *this },
		m_io_write{ *this, *this },
		m_ssg(*this, "ssg")
	{
	}

	// configuration helpers, handled by the interface
	auto irq_handler() { return m_update_irq.bind(); }
	auto io_read_handler(int index = 0) { return m_io_read[index & 1].bind(); }
	auto io_write_handler(int index = 0) { return m_io_write[index & 1].bind(); }

	// read access, handled by the chip implementation
	virtual u8 read(offs_t offset) = 0;

	// write access, handled by the chip implementation
	virtual void write(offs_t offset, u8 data) = 0;

protected:
	// perform a synchronized write
	virtual void ymfm_sync_mode_write(uint8_t data) override
	{
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(ym_generic::fm_mode_write), this), data);
	}

	// perform a synchronized interrupt check
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
	void fm_timer_handler(void *ptr, int param) { m_engine->engine_timer_expired(param); }
	void fm_mode_write(void *ptr, int param) { m_engine->engine_mode_write(param); }
	void fm_check_interrupts(void *ptr, int param) { m_engine->engine_check_interrupts(); }

	// internal state
	attotime m_busy_end;             // busy end time
	emu_timer *m_timer[2];           // two timers
	devcb_write_line m_update_irq;   // IRQ update callback
	devcb_read8 m_io_read[2];        // up to 2 input port handlers
	devcb_write8 m_io_write[2];      // up to 2 output port handlers
	optional_device<ay8910_device> m_ssg; // our embedded SSG device
};


// ======================> ymfm_device_base

// this template provides most of the basics used by device objects in MAME
// that wrap ymfm chips; it implements the fm_interface, offers binding to
// external callbacks, and provides basic read/write functions; however, this
// class is not intended to be used directly -- rather, devices should inherit
// from eitehr ymfm_device_base or ymfm_ssg_device_base, depending on whether
// they include an SSG or not
template<typename ChipClass, int SSGStreams = 0>
class ymfm_device_base : public ym_generic
{
public:
	// constructor
	ymfm_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int aystreams = 0, int ayioports = 0) :
		ym_generic(mconfig, tag, owner, clock, type, aystreams, ayioports),
		m_stream(nullptr),
		m_ssg_stream(nullptr),
		m_chip(*this)
	{
	}

	// read access, handled by the chip implementation
	virtual u8 read(offs_t offset) override
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
	virtual void write(offs_t offset, u8 data) override
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

	using ym_generic::machine;
	using ym_generic::stream_alloc;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override
	{
		// if there are SSG streams, create an AY8910 device
		if (SSGStreams > 0)
		{
			AY8910(config, m_ssg, device_t::clock());
			m_ssg->set_psg_type(ay8910_device::PSG_TYPE_YM);
			if (SSGStreams == 1)
				m_ssg->set_flags(AY8910_SINGLE_OUTPUT | AY8910_LEGACY_OUTPUT);

			// configure the callbacks to route through our callbacks
			m_ssg->port_a_read_callback().set([this] () { return m_io_read[0].isnull() ? 0 : m_io_read[0](0); });
			m_ssg->port_a_write_callback().set([this] (uint8_t data) { if (!m_io_write[0].isnull()) m_io_write[0](0, data); });
			m_ssg->port_b_read_callback().set([this] () { return m_io_read[1].isnull() ? 0 : m_io_read[1](1); });
			m_ssg->port_b_write_callback().set([this] (uint8_t data) { if (!m_io_write[1].isnull()) m_io_write[1](1, data); });

			// route outputs through us
			m_ssg->add_route(0, *this, 1.0, 0);
			if (SSGStreams > 1)
			{
				m_ssg->add_route(1, *this, 1.0, 1);
				m_ssg->add_route(2, *this, 1.0, 2);
			}
		}
	}

	// handle device start
	virtual void device_start() override
	{
		// let our parent do its startup
		ym_generic::device_start();

		// allocate an SSG stream if needed
		device_start_ssg();

		// allocate our stream
		m_stream = stream_alloc(0, ChipClass::OUTPUTS, m_chip.sample_rate(device_t::clock()));

		// register for save states
		m_chip.register_save(*this);
	}

	// if SSGStreams is non-zero, create them and request overrides; otherwise,
	// do nothing
	template<bool SSGPresent = (SSGStreams != 0)>
	std::enable_if_t<!SSGPresent, void> device_start_ssg()
	{
	}

	template<bool SSGPresent = (SSGStreams != 0)>
	std::enable_if_t<SSGPresent, void> device_start_ssg()
	{
		m_ssg_stream = stream_alloc(SSGStreams, SSGStreams, SAMPLE_RATE_INPUT_ADAPTIVE);
		m_chip.ssg_override(*this);
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
		device_clock_changed_ssg();
	}

	// if SSGStreams is non-zero, configure the SSG clock based on our
	// clock change; otherwise, do nothing
	template<bool SSGPresent = (SSGStreams != 0)>
	std::enable_if_t<!SSGPresent, void> device_clock_changed_ssg()
	{
	}

	template<bool SSGPresent = (SSGStreams != 0)>
	std::enable_if_t<SSGPresent, void> device_clock_changed_ssg()
	{
		m_ssg->set_unscaled_clock(ymfm::ssg_engine::CLOCK_DIVIDER * m_chip.sample_rate_ssg(device_t::clock()));
	}

	virtual void device_post_load() override
	{
		m_chip.invalidate_caches();
	}

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override
	{
		if (&stream == m_stream)
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
		else if (&stream == m_ssg_stream)
		{
			for (int index = 0; index < SSGStreams; index++)
				outputs[index] = inputs[index];
		}
	}

	// internal state
	sound_stream *m_stream;          // sound stream
	sound_stream *m_ssg_stream;      // SSG sound stream
	ChipClass m_chip;                // core chip implementation
};


#endif // MAME_SOUND_YMFM_H
