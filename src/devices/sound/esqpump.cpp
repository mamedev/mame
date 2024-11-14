// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  esqpump.cpp - Ensoniq 5505/5506 to 5510 interface.

  By Christian Brunschen

***************************************************************************/

#include "emu.h"
#include "esqpump.h"

DEFINE_DEVICE_TYPE(ESQ_5505_5510_PUMP, esq_5505_5510_pump_device, "esq_5505_5510_pump", "Ensoniq 5505/5506 to 5510 interface")

esq_5505_5510_pump_device::esq_5505_5510_pump_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ESQ_5505_5510_PUMP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_esp(*this, finder_base::DUMMY_TAG)
	, m_esp_halted(true)
	, ticks_spent_processing(0)
	, samples_processed(0)
{
}

void esq_5505_5510_pump_device::device_start()
{
	m_stream = stream_alloc(8, 2, clock(), STREAM_SYNCHRONOUS);

#if PUMP_DETECT_SILENCE
	silent_for = 500;
	was_silence = 1;
#endif
#if !PUMP_FAKE_ESP_PROCESSING
	ticks_spent_processing = 0;
	samples_processed = 0;
#endif
#if PUMP_TRACK_SAMPLES
	last_samples = 0;
	last_ticks = osd_ticks();
	next_report_ticks = last_ticks + osd_ticks_per_second();
#endif

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	e.resize(0x4000);
	ei = 0;
#endif
}

void esq_5505_5510_pump_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
}

void esq_5505_5510_pump_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	sound_assert(outputs[0].samples() == 1);

	auto &left = outputs[0];
	auto &right = outputs[1];
#define SAMPLE_SHIFT 4
	constexpr stream_buffer::sample_t input_scale = 32768.0 / (1 << SAMPLE_SHIFT);

	// anything for the 'aux' output?
	stream_buffer::sample_t l = inputs[0].get(0) * (1.0 / (1 << SAMPLE_SHIFT));
	stream_buffer::sample_t r = inputs[1].get(0) * (1.0 / (1 << SAMPLE_SHIFT));

	// push the samples into the ESP
	m_esp->ser_w(0, s32(inputs[2].get(0) * input_scale));
	m_esp->ser_w(1, s32(inputs[3].get(0) * input_scale));
	m_esp->ser_w(2, s32(inputs[4].get(0) * input_scale));
	m_esp->ser_w(3, s32(inputs[5].get(0) * input_scale));
	m_esp->ser_w(4, s32(inputs[6].get(0) * input_scale));
	m_esp->ser_w(5, s32(inputs[7].get(0) * input_scale));

#if PUMP_FAKE_ESP_PROCESSING
	m_esp->ser_w(6, m_esp->ser_r(0) + m_esp->ser_r(2) + m_esp->ser_r(4));
	m_esp->ser_w(7, m_esp->ser_r(1) + m_esp->ser_r(3) + m_esp->ser_r(5));
#else
	if (!m_esp_halted) {
		osd_ticks_t a = osd_ticks();
		m_esp->run_once();
		osd_ticks_t b = osd_ticks();
		ticks_spent_processing += (b - a);
		samples_processed++;
	}
#endif

	// read the processed result from the ESP and add to the saved AUX data
	stream_buffer::sample_t ll = stream_buffer::sample_t(m_esp->ser_r(6)) * (1.0 / 32768.0);
	stream_buffer::sample_t rr = stream_buffer::sample_t(m_esp->ser_r(7)) * (1.0 / 32768.0);
	l += ll;
	r += rr;

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	// if we're processing the fake program through the ESP, the result should just be that of adding the inputs
	stream_buffer::sample_t el = (inputs[2].get(0)) + (inputs[4].get(0)) + (inputs[6].get(0));
	stream_buffer::sample_t er = (inputs[3].get(0)) + (inputs[5].get(0)) + (inputs[7].get(0));
	stream_buffer::sample_t e_next = el + er;
	e[(ei + 0x1d0f) % 0x4000] = e_next;

	if (fabs(l - e[ei]) > 1e-5) {
		util::stream_format(std::cerr, "expected (%d) but have (%d)\n", e[ei], l);
	}
	ei = (ei + 1) % 0x4000;
#endif

	// write the combined data to the output
	left.put(0, l);
	right.put(0, r);

#if PUMP_DETECT_SILENCE
	if (left.get(0) == 0 && right.get(0) == 0) {
		silent_for++;
	} else {
		silent_for = 0;
	}
	bool silence = silent_for >= 500;
	if (was_silence != silence) {
		if (!silence) {
			util::stream_format(std::cerr, ".-*\n");
		} else {
			util::stream_format(std::cerr, "*-.\n");
		}
		was_silence = silence;
	}
#endif

#if PUMP_TRACK_SAMPLES
	last_samples += samples;
	osd_ticks_t now = osd_ticks();
	if (now >= next_report_ticks)
	{
		osd_ticks_t elapsed = now - last_ticks;
		osd_ticks_t tps = osd_ticks_per_second();
		util::stream_format(std::cerr, "Pump: %d samples in %d ticks for %f Hz\n", last_samples, elapsed, last_samples * (double)tps / (double)elapsed);
		last_ticks = now;
		while (next_report_ticks <= now) {
			next_report_ticks += tps;
		}
		last_samples = 0;

#if !PUMP_FAKE_ESP_PROCESSING
		util::stream_format(std::cerr, "  ESP spent %d ticks on %d samples, %f ticks per sample\n", ticks_spent_processing, samples_processed, (double)ticks_spent_processing / (double)samples_processed);
		ticks_spent_processing = 0;
		samples_processed = 0;
#endif
	}
#endif
}
