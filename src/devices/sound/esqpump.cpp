// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  esqpump.cpp - Ensoniq 5505/5506 to 5510 interface.

  By Christian Brunschen

***************************************************************************/

#include "emu.h"
#include "sound/esqpump.h"

DEFINE_DEVICE_TYPE(ESQ_5505_5510_PUMP, esq_5505_5510_pump_device, "esq_5505_5510_pump", "Ensoniq 5505/5506 to 5510 interface")

esq_5505_5510_pump_device::esq_5505_5510_pump_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ESQ_5505_5510_PUMP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer(nullptr)
	, m_esp(*this, finder_base::DUMMY_TAG)
	, m_esp_halted(true)
	, ticks_spent_processing(0)
	, samples_processed(0)
{
#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	e = nullptr;
#endif
}

void esq_5505_5510_pump_device::device_start()
{
	logerror("Clock = %d\n", clock());

	m_stream = machine().sound().stream_alloc(*this, 8, 2, clock());
	m_timer = timer_alloc(0);
	m_timer->enable(false);

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
	e = make_unique_clear<int16_t[]>(0x4000);
	ei = 0;
#endif
}

void esq_5505_5510_pump_device::device_stop()
{
	m_timer->enable(false);
}

void esq_5505_5510_pump_device::device_reset()
{
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
	m_timer->enable(true);
}

void esq_5505_5510_pump_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));
}

void esq_5505_5510_pump_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (samples != 1) {
		logerror("Pump: request for %d samples\n", samples);
	}

	stream_sample_t *left = outputs[0], *right = outputs[1];
	for (int i = 0; i < samples; i++)
	{
#define SAMPLE_SHIFT 4
		// anything for the 'aux' output?
		int16_t l = inputs[0][i] >> SAMPLE_SHIFT;
		int16_t r = inputs[1][i] >> SAMPLE_SHIFT;

		// push the samples into the ESP
		m_esp->ser_w(0, inputs[2][i] >> SAMPLE_SHIFT);
		m_esp->ser_w(1, inputs[3][i] >> SAMPLE_SHIFT);
		m_esp->ser_w(2, inputs[4][i] >> SAMPLE_SHIFT);
		m_esp->ser_w(3, inputs[5][i] >> SAMPLE_SHIFT);
		m_esp->ser_w(4, inputs[6][i] >> SAMPLE_SHIFT);
		m_esp->ser_w(5, inputs[7][i] >> SAMPLE_SHIFT);

#if PUMP_FAKE_ESP_PROCESSING
		m_esp->ser_w(6, m_esp->ser_r(0) + m_esp->ser_r(2) + m_esp->ser_r(4));
		m_esp->ser_w(7, m_esp->ser_r(1) + m_esp->ser_r(3) + m_esp->ser_r(5));
#else
		if (!m_esp_halted) {
			logerror("passing one sample through ESP\n");
			osd_ticks_t a = osd_ticks();
			m_esp->run_once();
			osd_ticks_t b = osd_ticks();
			ticks_spent_processing += (b - a);
			samples_processed++;
		}
#endif

		// read the processed result from the ESP and add to the saved AUX data
		int16_t ll = m_esp->ser_r(6);
		int16_t rr = m_esp->ser_r(7);
		l += ll;
		r += rr;

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
		// if we're processing the fake program through the ESP, the result should just be that of adding the inputs
		int32_t el = (inputs[2][i]) + (inputs[4][i]) + (inputs[6][i]);
		int32_t er = (inputs[3][i]) + (inputs[5][i]) + (inputs[7][i]);
		int32_t e_next = el + er;
		e[(ei + 0x1d0f) % 0x4000] = e_next;

		if (l != e[ei]) {
			util::stream_format(std::cerr, "expected (%d) but have (%d)\n", e[ei], l);
		}
		ei = (ei + 1) % 0x4000;
#endif

		// write the combined data to the output
		*left++  = l;
		*right++ = r;
	}

#if PUMP_DETECT_SILENCE
	for (int i = 0; i < samples; i++) {
		if (outputs[0][i] == 0 && outputs[1][i] == 0) {
			silent_for++;
		} else {
			silent_for = 0;
		}
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

void esq_5505_5510_pump_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) {
	// ecery time there's a new sample period, update the stream!
	m_stream->update();
}
