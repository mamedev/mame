// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/***************************************************************************

    mc14529.cpp

    See mc14529.h for device details.

    Timing and Transition Model:
    - Address changes are handled by queue_switch_update(), scheduling a
      switch event after a configurable switching delay (m_tsw). When the
      timer expires, switch_selector() updates the digital or analog outputs
      immediately for any non-inhibited selectors.
    - Input value changes are handled by queue_value_update(), scheduling
      a value update after a propagation delay (m_tplh for rising/high,
      m_tphl for falling/low). When the timer expires, the channel value
      is updated, and if that channel is currently selected and not
      inhibited, the output is updated instantly.
    - A pool of up to MAX_PENDING timers handles these events. If the pool
      becomes completely exhausted, the oldest pending transition is
      forced to commit early.

    MODE_SOUND:
    - Audio streams bypass the propagation/switching delay timers. Instead,
      the stream is updated to flush outstanding samples immediately before
      an address or inhibit change occurs.
    - Supports a configurable linear crossfade ramp (set_sound_crossfade)
      applied during channel switches and inhibit transitions to eliminate
      audible switching clicks.

    Inhibit Behavior:
    - Changing the inhibit state (inhibit_x_w / inhibit_y_w) instantly
      updates the selector's output via switch_selector() and updates the
      sound target. While inhibited, digital, analog, and sound outputs
      latch at their last value and do not track input updates.

***************************************************************************/

#include "emu.h"
#include "mc14529.h"

#define LOG_GENERAL     (1U << 0)
#define LOG_SWITCH      (1U << 1)
#define LOG_TIMER       (1U << 2)
#define LOG_ANALOGWRITE (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_SWITCH|LOG_TIMER|LOG_ANALOGWRITE)

#include "logmacro.h"
#define LOGSWITCH(...)      LOGMASKED(LOG_SWITCH, __VA_ARGS__)
#define LOGTIMER(...)       LOGMASKED(LOG_TIMER, __VA_ARGS__)
#define LOGANALOGWRITE(...) LOGMASKED(LOG_ANALOGWRITE, __VA_ARGS__)

DEFINE_DEVICE_TYPE(MC14529, mc14529_device, "mc14529", "MC14529 Dual 4-Channel Analog Data Selector")

ALLOW_SAVE_TYPE(mc14529_device::mode_t);

mc14529_device::mc14529_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MC14529, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_write_z{ { *this }, { *this } }
	, m_write_z_analog{ { *this }, { *this } }
	, m_write_address_changed{ *this }
	, m_stream(nullptr)
	, m_tplh(attotime::from_nsec(150))
	, m_tphl(attotime::from_nsec(150))
	, m_tsw(attotime::from_nsec(0))
	, m_address(0)
	, m_last_queued_address(0)
{
	std::fill(std::begin(m_mode), std::end(m_mode), MODE_DIGITAL);
	std::fill(std::begin(m_width_mask), std::end(m_width_mask), 0xff);
	std::fill(std::begin(m_inhibit), std::end(m_inhibit), 0);
	std::fill(std::begin(m_current_output), std::end(m_current_output), 0);
	m_pending_next = 0;

	std::fill(std::begin(m_sound_crossfade), std::end(m_sound_crossfade), attotime::zero);
	std::fill(std::begin(m_sound_current_source), std::end(m_sound_current_source), SOUND_SOURCE_SILENCE);
	std::fill(std::begin(m_sound_fade_active), std::end(m_sound_fade_active), false);
	std::fill(std::begin(m_sound_fade_from_sample), std::end(m_sound_fade_from_sample), 0.0);
	std::fill(std::begin(m_sound_fade_target), std::end(m_sound_fade_target), SOUND_SOURCE_SILENCE);
	std::fill(std::begin(m_sound_fade_samples_total), std::end(m_sound_fade_samples_total), 0);
	std::fill(std::begin(m_sound_fade_samples_done), std::end(m_sound_fade_samples_done), 0);

	for (auto &sel : m_channel)
		std::fill(std::begin(sel), std::end(sel), 0);
	for (auto &sel : m_channel_value)
		std::fill(std::begin(sel), std::end(sel), 0);

	for (unsigned s = 0; s < NUM_SELECTORS; s++)
	{
		std::fill(std::begin(m_pending_value[s]), std::end(m_pending_value[s]), 0);
		std::fill(std::begin(m_last_scheduled[s]), std::end(m_last_scheduled[s]), 0);
		m_last_stream_sampel[s] = sound_stream::sample_t(0.0);
	}

	std::fill(std::begin(m_pending_active), std::end(m_pending_active), false);
	std::fill(std::begin(m_pending_timer), std::end(m_pending_timer), nullptr);
}

mc14529_device &mc14529_device::set_propagation_delay(const attotime &tplh, const attotime &tphl)
{
	m_tplh = tplh;
	m_tphl = tphl;
	return *this;
}

mc14529_device &mc14529_device::set_switching_delay(const attotime &tsw)
{
	m_tsw = tsw;
	return *this;
}

mc14529_device &mc14529_device::set_sound_crossfade(unsigned selector, const attotime &time)
{
	m_sound_crossfade[selector] = time;
	return *this;
}

mc14529_device &mc14529_device::set_mode(unsigned selector, mode_t mode)
{
	m_mode[selector] = mode;

	return *this;
}

mc14529_device &mc14529_device::set_analog_width(unsigned selector, unsigned bits)
{
	m_width_mask[selector] = u8((1u << bits) - 1);
	return *this;
}

void mc14529_device::device_start()
{
	// Always allocate the sound stream (4 inputs per selector, 1 output
	// per selector). Harmless if neither selector uses MODE_SOUND. The
	// outputs are simply filled with silence and never routed anywhere.
	m_stream = stream_alloc(NUM_SELECTORS * NUM_CHANNELS, NUM_SELECTORS, SAMPLE_RATE_INPUT_ADAPTIVE);

	for (unsigned slot = 0; slot < MAX_PENDING; slot++)
		m_pending_timer[slot] = timer_alloc(FUNC(mc14529_device::delay_expired), this);

	save_item(NAME(m_channel));
	save_item(NAME(m_channel_value));
	save_item(NAME(m_address));
	save_item(NAME(m_inhibit));
	save_item(NAME(m_current_output));
	save_item(NAME(m_last_scheduled));
	save_item(NAME(m_pending_value));
	save_item(NAME(m_pending_active));
	save_item(NAME(m_pending_next));

	// Configuration state that can be altered at runtime via public setters
	save_item(NAME(m_mode));
	save_item(NAME(m_width_mask));
	save_item(NAME(m_sound_crossfade));

	save_item(NAME(m_sound_current_source));
	save_item(NAME(m_sound_fade_active));
	save_item(NAME(m_sound_fade_from_sample));
	save_item(NAME(m_sound_fade_target));
	save_item(NAME(m_sound_fade_samples_total));
	save_item(NAME(m_sound_fade_samples_done));
	save_item(NAME(m_last_stream_sampel));
	save_item(NAME(m_last_queued_address));

	// GLOBALS / MEMBERS NOT MANUALLY SAVED BELOW:
	// - m_stream: Sound streams are automatically registered and saved by the MAME core.
	// - m_pending_timer: Timers allocated via timer_alloc() are tracked and restored internally by the core.
	// - m_tplh, m_tphl, m_tsw: Core timing characteristics set at device initialization; they do not change dynamically.
}

void mc14529_device::device_reset()
{
	m_address = 0;
	std::fill(std::begin(m_inhibit), std::end(m_inhibit), 0);

	for (auto &sel : m_channel)
		std::fill(std::begin(sel), std::end(sel), 0);
	for (auto &sel : m_channel_value)
		std::fill(std::begin(sel), std::end(sel), 0);

	for (unsigned s = 0; s < NUM_SELECTORS; s++)
	{
		for (unsigned chan = 0; chan < NUM_CHANNELS; chan++)
		{
			m_pending_value[s][chan] = 0;
			m_last_scheduled[s][chan] = 0;
		}

		m_current_output[s] = 0;

		if (m_mode[s] == MODE_ANALOG)
			m_write_z_analog[s](0);
		else if (m_mode[s] == MODE_DIGITAL)
			m_write_z[s](0);

		m_sound_fade_active[s] = false;
		m_sound_fade_samples_total[s] = 0;
		m_sound_fade_samples_done[s] = 0;
		m_sound_current_source[s] = m_inhibit[s] ? SOUND_SOURCE_SILENCE : m_address;
	}

	for (unsigned slot = 0; slot < MAX_PENDING; slot++)
	{
		m_pending_timer[slot]->adjust(attotime::never);
		m_pending_active[slot] = false;
	}

	m_pending_next = 0;
	m_last_queued_address = 0;
}

void mc14529_device::sound_stream_update(sound_stream &stream)
{
	for (unsigned selector = 0; selector < NUM_SELECTORS; selector++)
		update_selector_stream(stream, selector);
}

inline void mc14529_device::update_selector_stream(sound_stream &stream, unsigned selector)
{
	if (m_mode[selector] != MODE_SOUND)
	{
		stream.fill(selector, 0);
		finalize_sample_capture(stream, selector);
		return;
	}

	if (!m_sound_fade_active[selector])
	{
		u8 const src = m_sound_current_source[selector];
		if (src == SOUND_SOURCE_SILENCE)
			stream.fill(selector, m_last_stream_sampel[selector]);
		else
			stream.copy(selector, selector * NUM_CHANNELS + src);

		finalize_sample_capture(stream, selector);
		return;
	}

	s32 const n = stream.samples();
	u8 const target = m_sound_fade_target[selector];
	unsigned const target_input = selector * NUM_CHANNELS + target;
	s32 i = 0;

	for (; i < n; i++)
	{
		sound_stream::sample_t const to = (target == SOUND_SOURCE_SILENCE)
			? m_last_stream_sampel[selector] : stream.get(target_input, i);
		sound_stream::sample_t const frac = sound_stream::sample_t(m_sound_fade_samples_done[selector])
			/ sound_stream::sample_t(m_sound_fade_samples_total[selector]);

		stream.put(selector, i, m_sound_fade_from_sample[selector] + (to - m_sound_fade_from_sample[selector]) * frac);

		if (++m_sound_fade_samples_done[selector] >= m_sound_fade_samples_total[selector])
		{
			m_sound_fade_active[selector] = false;
			m_sound_current_source[selector] = target;
			i++;
			break;
		}
	}

	if (!m_sound_fade_active[selector] && i < n)
	{
		if (target == SOUND_SOURCE_SILENCE)
			stream.fill(selector, m_last_stream_sampel[selector], i);
		else
			stream.copy(selector, target_input, i);
	}

	finalize_sample_capture(stream, selector);
}

inline void mc14529_device::finalize_sample_capture(sound_stream &stream, unsigned selector)
{
	s32 const n = stream.samples();
	sound_stream::sample_t last_sample = stream.get_output(selector, n - 1);
	m_last_stream_sampel[selector] = last_sample;
}

void mc14529_device::switch_selector(unsigned address)
{
	m_address = address;
	m_write_address_changed(m_address);

	if (m_stream)
		m_stream->update(); // flush MODE_SOUND output(s) at the old address before switching

	for (unsigned selector = 0; selector < NUM_SELECTORS; selector++)
	{
		if (m_inhibit[selector])
			continue; // frozen: output holds its last value until un-inhibited

		if (m_mode[selector] == MODE_ANALOG)
		{
			u8 new_value;
			m_current_output[selector] = m_channel_value[selector][m_address];
			new_value = m_current_output[selector];
			m_write_z_analog[selector](new_value);
		}
		else if (m_mode[selector] == MODE_DIGITAL)
		{
			int new_value;
			m_current_output[selector] = m_channel[selector][m_address];
			new_value = m_current_output[selector];
			m_write_z[selector](new_value);
		}
	}
}

void mc14529_device::queue_switch_update(unsigned new_address)
{
	if (m_last_queued_address == new_address)
		return;

	m_last_queued_address = new_address;

	update_sound_target(new_address);

	unsigned slot = m_pending_next;
	if (m_pending_active[slot])
	{
		// pool exhausted
#ifdef MAME_DEBUG
		fatalerror("mc14529: transition pool exhausted: slot %u\n", slot);
#else
 		osd_printf_error("mc14529: transition pool exhausted, forcing early commit of slot %u\n", slot);
#endif
		m_pending_timer[slot]->adjust(attotime::never);
		delay_expired(m_pending_timer[slot]->param());
	}

	s32 param = pack(slot, 0, 0, true /* switch */, new_address);
	m_pending_active[slot] = true;
	m_pending_timer[slot]->adjust(m_tsw, param);
	m_pending_next = (slot + 1) % MAX_PENDING;
	LOGSWITCH("queued switch change: slot %d\n", slot);
}

void mc14529_device::update_sound_target(unsigned address)
{
	for (unsigned selector = 0; selector < NUM_SELECTORS; selector++)
	{
		if (m_mode[selector] != MODE_SOUND)
			continue;

		u8 const target = m_inhibit[selector] ? SOUND_SOURCE_SILENCE : address;

		if (target == m_sound_current_source[selector] && !m_sound_fade_active[selector])
			continue;

		if (target == m_sound_fade_target[selector] && m_sound_fade_active[selector])
			continue;

		if (m_stream)
			m_stream->update();

		if (m_sound_crossfade[selector] == attotime::zero)
		{
			m_sound_current_source[selector] = target;
			m_sound_fade_active[selector] = false;
			continue;
		}

		m_sound_fade_from_sample[selector] = m_last_stream_sampel[selector];
		m_sound_fade_target[selector] = target;
		m_sound_fade_samples_total[selector] = std::max<u32>(1,
			u32(m_sound_crossfade[selector].as_double() * m_stream->sample_rate()));
		m_sound_fade_samples_done[selector] = 0;
		m_sound_fade_active[selector] = true;
	}
}

void mc14529_device::queue_value_update(unsigned selector, unsigned channel, u8 value)
{
	if (m_mode[selector] == MODE_SOUND)
		return;

	if (m_last_scheduled[selector][channel] == value)
		return;

	unsigned const slot = m_pending_next;

	if (m_pending_active[slot])
	{
		// pool exhausted
#ifdef MAME_DEBUG
		fatalerror("mc14529: transition pool exhausted: slot %u\n", slot);
#else
 		osd_printf_error("mc14529: transition pool exhausted, forcing early commit of slot %u\n", slot);
#endif
		m_pending_timer[slot]->adjust(attotime::never);
		delay_expired(m_pending_timer[slot]->param());
	}

	bool const rising = (m_mode[selector] == MODE_ANALOG) ? (value > m_last_scheduled[selector][channel]) : (value != 0);
	attotime const delay = rising ? m_tplh : m_tphl;

	m_pending_active[slot] = true;
	m_pending_timer[slot]->adjust(delay, pack(slot, selector, channel, false /* not switch */, value));
	m_last_scheduled[selector][channel] = value;
	m_pending_next = (slot + 1) % MAX_PENDING;
}

TIMER_CALLBACK_MEMBER(mc14529_device::delay_expired)
{
	const PackedData data{ .raw = param };
	const unsigned slot     = data.bits.slot;
	const unsigned selector = data.bits.selector;
	const unsigned channel  = data.bits.channel;
	const bool sw           = data.bits.sw;
	const uint8_t value     = (uint8_t)data.bits.value;

	m_pending_active[slot] = false;

	LOGTIMER("delay_expired: slot: %d, switch: %d, selector: %d, channel: %d, value: %d (%11.6f)\n",
		slot, sw, selector, channel, value, machine().time().as_double());

	if (sw)
	{
		switch_selector(value);
	}
	else
	{
		if (m_mode[selector] == MODE_ANALOG)
			m_channel_value[selector][channel] = value;
		else if (m_mode[selector] == MODE_DIGITAL)
			m_channel[selector][channel] = value;
		else if (m_mode[selector] == MODE_SOUND)
			return;

		if (m_inhibit[selector] == false)
		{
			if (m_address == channel)
			{
				m_current_output[selector] = value;

				if (m_mode[selector] == MODE_ANALOG)
					m_write_z_analog[selector](value);
				else if (m_mode[selector] == MODE_DIGITAL)
					m_write_z[selector](value);
			}
		}
	}
}

u8 mc14529_device::zx_value()
{
	return m_current_output[SEL_X];
}

u8 mc14529_device::zy_value()
{
	return m_current_output[SEL_Y];
}

void mc14529_device::address_w(int bit, int state)
{
	state = !!state;
	u8 const mask = u8(1) << bit;
	u8 const new_address = state ? (m_last_queued_address | mask) : (m_last_queued_address & ~mask);

	LOGSWITCH("address_w: bit: %d, state: %d, new_address: %d (%11.6f)\n", bit, state, new_address, machine().time().as_double());

	queue_switch_update(new_address);
}

void mc14529_device::inhibit_x_w(int state)
{
	state = !!state;

	LOGSWITCH("inhibit_x_w: state: %d (%11.6f)\n", state, machine().time().as_double());

	if (m_inhibit[SEL_X] == state)
		return;

	if (m_stream)
		m_stream->update();

	m_inhibit[SEL_X] = state;

	switch_selector(m_address);
	update_sound_target(m_address);
}

void mc14529_device::inhibit_y_w(int state)
{
	state = !!state;

	LOGSWITCH("inhibit_y_w: state: %d (%11.6f)\n", state, machine().time().as_double());

	if (m_inhibit[SEL_Y] == state)
		return;

	if (m_stream)
		m_stream->update();

	m_inhibit[SEL_Y] = state;
	switch_selector(m_address);
	update_sound_target(m_address);
}

void mc14529_device::x_w(int channel, int state)
{
	state = !!state;
	queue_value_update(SEL_X, channel, state);
}

void mc14529_device::y_w(int channel, int state)
{
	state = !!state;
	queue_value_update(SEL_Y, channel, state);
}

void mc14529_device::x_analog_w(int channel, u8 value)
{
	value &= m_width_mask[SEL_X];
	queue_value_update(SEL_X, channel, value);
	LOGANALOGWRITE("x_analog_w: channel: %d, value: %d (%11.6f)\n", channel, value, machine().time().as_double());
}

void mc14529_device::y_analog_w(int channel, u8 value)
{
	value &= m_width_mask[SEL_Y];
	queue_value_update(SEL_Y, channel, value);
	LOGANALOGWRITE("x_analog_y: channel: %d, value: %d (%11.6f)\n", channel, value, machine().time().as_double());
}

int32_t mc14529_device::pack(unsigned slot, unsigned selector, unsigned channel, bool sw, uint8_t value)
{
	assert(slot < MAX_PENDING);
	assert(selector < NUM_SELECTORS);
	assert(channel < NUM_CHANNELS);

	PackedData data{};
	data.bits.value    = value;
	data.bits.channel  = channel;
	data.bits.sw       = sw;
	data.bits.selector = selector;
	data.bits.slot     = slot;

	return data.raw;
}
