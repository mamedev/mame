// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_YM2154_H
#define MAME_SOUND_YM2154_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(YM2154, ym2154_device)

// ======================> ym2154_device

class ym2154_device : public device_t, public device_sound_interface, public device_memory_interface
{
	static constexpr uint8_t ADDR_SHIFT = 6;

public:
	// internal constructor
	ym2154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_update_irq.bind(); }
	auto io_read_handler() { return m_io_read.bind(); }
	auto io_write_handler() { return m_io_write.bind(); }

	// register reads
	u8 read(offs_t offset);

	// register writes
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// memory space configuration
	virtual space_config_vector memory_space_config() const override;

	TIMER_CALLBACK_MEMBER(delayed_irq);

private:
	struct channel
	{
		channel() :
			m_pos(0xfffffff), m_start(0), m_end(0), m_panpot(0), m_output_level(0), m_rate(0) { }

		void reset()
		{
			m_pos = 0;
			m_start = m_end = 0;
			m_panpot = 0;
			m_output_level = 0;
			m_rate = 0;
		}

		void start()
		{
			m_pos = m_start << ADDR_SHIFT;
		}

		uint32_t m_pos;
		uint16_t m_start;
		uint16_t m_end;
		uint8_t m_panpot;
		uint8_t m_output_level;
		uint8_t m_rate;
	};

	// internal helpers
	u32 sample_rate() const { return device_t::clock() / 18 / 6; }
	void update_irq_state(u8 state) { if (m_irq_state != state) { m_irq_state = state; m_update_irq(state); } }

	// internal state
	sound_stream *m_stream;                     // sound stream
	emu_timer *m_timer;                         // two timers
	uint16_t m_timer_count;                     // current timer count
	uint8_t m_timer_enable;                     // timer enable
	uint8_t m_irq_state;                        // current IRQ state
	uint8_t m_irq_count;                        // current IRQ count
	uint8_t m_total_level;                      // master volume
	channel m_channel[12];                      // output channels
	devcb_write_line m_update_irq;              // IRQ update callback
	devcb_read8 m_io_read;                      // input port handler
	devcb_write8 m_io_write;                    // output port handler
	address_space_config const m_group0_config; // address space 0 config
	address_space_config const m_group1_config; // address space 1 config
	optional_memory_region m_group0_region;     // group 0 memory region
	optional_memory_region m_group1_region;     // group 1 memory region
};


#endif // MAME_SOUND_YM2154_H
