// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX analogue-to-digital converter. Implements input selection, clocked
    conversion sequencing, completion status and interrupts, accepting
    analogue input values or routed microphone audio.

    Converter sampling and the software-visible result use separate timing
    stages. Reset retains the converter's held sample and output while
    resetting the control registers and completion sequence.

    TODO:
    - Determine the analogue gain selected by control bits 2-3.

***************************************************************************/

#include "emu.h"
#include "xavix_adc.h"

#include <algorithm>
#include <limits>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_ADC, xavix_adc_device, "xavix_adc", "XaviX ADC")

xavix_adc_device::xavix_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_ADC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_input_cb(*this, 0xff)
	, m_converter_result(0)
	, m_converter_sample(0)
	, m_converter_state(0)
	, m_legacy_control(false)
	, m_audio_channel{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
	, m_audio_input_count(0)
{
}

void xavix_adc_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(xavix_adc_device::data_r)).nopw();
	map(0x1, 0x1).rw(FUNC(xavix_adc_device::ctrl_r), FUNC(xavix_adc_device::ctrl_w));
	map(0x2, 0x2).rw(FUNC(xavix_adc_device::rate_select_r), FUNC(xavix_adc_device::rate_select_w));
}

void xavix_adc_device::device_start()
{
	m_conversion_timer = timer_alloc(FUNC(xavix_adc_device::conversion_tick), this);
	if (m_audio_input_count)
		m_audio_stream = stream_alloc(m_audio_input_count, 0, SAMPLE_RATE_INPUT_ADAPTIVE);

	save_item(NAME(m_ctrl));
	save_item(NAME(m_result));
	save_item(NAME(m_converter_result));
	save_item(NAME(m_converter_sample));
	save_item(NAME(m_converter_state));
	save_item(NAME(m_rate_select));
	save_item(NAME(m_clock_phase));
	save_item(NAME(m_countdown));
	save_item(NAME(m_request_pending));
	save_item(NAME(m_conversion_pulse));
	save_item(NAME(m_result_pending));
	save_item(NAME(m_capture_edge));
	save_item(NAME(m_last_update_ticks));
	save_item(NAME(m_audio_sample));
}

void xavix_adc_device::device_reset()
{
	m_ctrl = 0;
	m_result = 0;
	m_rate_select = 0;
	m_clock_phase = 0;
	m_countdown = 0;
	m_request_pending = true;
	m_conversion_pulse = false;
	// Reset holds the converter clock low, retaining its sample and output.
	// The register copies that output on the next clock. Cold output is zero.
	m_result_pending = true;
	m_capture_edge = false;
	m_last_update_ticks = current_ticks();
	m_audio_sample.fill(0x80);
	update_irq();
	// An initial conversion supplies the result that epo_quiz polls at boot.
	schedule_conversion();
}

void xavix_adc_device::advance(u64 clocks)
{
	// The request and converter finish within a few divider periods. Once
	// idle, the independent completion timer can advance in one operation.
	while (clocks && (m_request_pending || m_conversion_pulse || m_result_pending || m_converter_state))
	{
		m_capture_edge = false;
		if (m_result_pending)
		{
			m_result = m_converter_result;
			m_result_pending = false;
		}
		bool const converting = m_conversion_pulse;
		if (m_countdown == 1)
			m_ctrl |= 0x80;
		if (m_clock_phase == 7)
		{
			if (converting)
				m_countdown = CONVERSION_PERIODS[m_rate_select];
			else if (m_countdown)
				m_countdown--;
		}
		if (m_clock_phase == 3)
		{
			if (converting)
				m_converter_state = 1;
			else if (m_converter_state)
			{
				// Track the selected input through the first two converter
				// periods, then hold it until the result becomes available.
				if (m_converter_state == 2)
				{
					m_converter_sample = sample_input();
					m_capture_edge = true;
				}
				if (m_converter_state == 10)
				{
					m_converter_result = m_converter_sample;
					m_result_pending = true;
				}
				m_converter_state = (m_converter_state + 1) & 15;
			}
		}
		if (m_clock_phase == 4)
			m_conversion_pulse = m_request_pending;
		if (converting)
			m_request_pending = false;
		m_clock_phase = (m_clock_phase + 1) & 7;
		clocks--;
	}
	if (clocks)
	{
		m_capture_edge = false;
		if (m_countdown && ((m_countdown == 1) || (clocks >= 9 - m_clock_phase + u64(m_countdown - 2) * 8)))
			m_ctrl |= 0x80;
		u64 const ticks = (clocks + m_clock_phase) / 8;
		m_countdown = ticks < m_countdown ? m_countdown - ticks : 0;
		m_clock_phase = (m_clock_phase + clocks) & 7;
	}
}

u64 xavix_adc_device::current_ticks() const
{
	attotime const now = machine().time();
	u32 const frequency = clock() * 2;
	return u64(now.seconds()) * frequency + now.attoseconds() / HZ_TO_ATTOSECONDS(frequency);
}

void xavix_adc_device::synchronize()
{
	u64 const now = current_ticks();
	// A CPU register access can precede a timer callback that already ran a
	// few clocks ahead. Keep the synchronized position monotonic.
	if (now > m_last_update_ticks)
	{
		advance(now - m_last_update_ticks);
		m_last_update_ticks = now;
	}
	update_irq();
}

void xavix_adc_device::schedule_conversion()
{
	u32 clocks = std::numeric_limits<u32>::max();
	if (m_request_pending || m_conversion_pulse)
		clocks = ((4 - m_clock_phase) & 7) + 1;
	if (m_converter_state || m_conversion_pulse)
		clocks = std::min<u32>(clocks, ((3 - m_clock_phase) & 7) + 1);
	if (m_result_pending)
		clocks = 1;
	if (m_countdown && !BIT(m_ctrl, 7))
	{
		u32 const completion = m_countdown == 1 ? 1 : 9 - m_clock_phase + (m_countdown - 2) * 8;
		clocks = std::min(clocks, completion);
	}
	if (clock() && clocks != std::numeric_limits<u32>::max())
		m_conversion_timer->adjust(attotime::from_ticks(m_last_update_ticks + clocks, clock() * 2) - machine().time());
	else
		m_conversion_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(xavix_adc_device::conversion_tick)
{
	synchronize();
	schedule_conversion();
}

u8 xavix_adc_device::data_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: data_r %02x\n", machine().describe_context(), m_result);
	return m_result;
}

u8 xavix_adc_device::status() const
{
	return m_legacy_control ? m_ctrl : m_ctrl | ((m_conversion_pulse || !BIT(m_ctrl, 7)) ? 0x40 : 0);
}

u8 xavix_adc_device::ctrl_r()
{
	if (!machine().side_effects_disabled())
		LOG("%s: ctrl_r %02x\n", machine().describe_context(), status());
	return status();
}

void xavix_adc_device::ctrl_w(u8 data)
{
	LOG("%s: ctrl_w %02x\n", machine().describe_context(), data);
	synchronize();
	// Start requests conversion without cancelling a pending request. Earlier
	// software also acknowledges completion on Start and retains Run readback.
	bool const acknowledge = BIT(data, 7) || (m_legacy_control && BIT(data, 6));
	m_ctrl = (acknowledge ? 0 : (m_ctrl & 0x80)) | (data & (m_legacy_control ? 0x7f : 0x3f));
	if (BIT(data, 6))
		m_request_pending = true;
	// TODO: Determine the analogue gain selected by bits 2-3.
	// A selection write on the final tracking edge participates in the sample.
	if (m_capture_edge)
		m_converter_sample = sample_input();
	update_irq();
	schedule_conversion();
}

void xavix_adc_device::sound_stream_update(sound_stream &stream)
{
	for (unsigned input = 0; input < m_audio_input_count; ++input)
	{
		sound_stream::sample_t const sample = stream.get(input, stream.samples() - 1);
		m_audio_sample[input] = std::clamp<int>(int(sample * 128.0F) + 128, 0, 255);
	}
}

u8 xavix_adc_device::rate_select_r()
{
	return m_rate_select;
}

void xavix_adc_device::rate_select_w(u8 data)
{
	LOG("%s: rate_select_w %02x\n", machine().describe_context(), data);
	synchronize();
	m_rate_select = data & 0x07;
	// A rate write can precede the queued conversion's countdown load, as
	// epo_quiz does at startup. It does not restart an existing countdown.
	schedule_conversion();
}

u8 xavix_adc_device::sample_input()
{
	u8 const input = (m_ctrl & 0x03) | (BIT(m_ctrl, 4) << 2);
	for (unsigned audio_input = 0; audio_input < m_audio_input_count; ++audio_input)
	{
		if (input == m_audio_channel[audio_input])
		{
			m_audio_stream->update();
			return m_audio_sample[audio_input];
		}
	}
	return m_input_cb[input]();
}

void xavix_adc_device::update_irq()
{
	m_irq_cb((m_ctrl & 0xa0) == 0xa0);
}
