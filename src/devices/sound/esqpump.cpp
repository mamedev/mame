// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  esqpump.c - Ensoniq 5505/5506 to 5510 interface.

  By Christian Brunschen

***************************************************************************/

#include "sound/esqpump.h"

const device_type ESQ_5505_5510_PUMP = &device_creator<esq_5505_5510_pump>;

esq_5505_5510_pump::esq_5505_5510_pump(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ESQ_5505_5510_PUMP, "ESQ_5505_5510_PUMP", tag, owner, clock, "esq_5505_5510_pump", __FILE__),
		device_sound_interface(mconfig, *this), m_stream(nullptr), m_timer(nullptr), m_otis(nullptr), m_esp(nullptr),
		m_esp_halted(true), ticks_spent_processing(0), samples_processed(0)
{
}

void esq_5505_5510_pump::device_start()
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
	memset(e, 0, 0x4000 * sizeof(e[0]));
	ei = 0;
#endif
}

void esq_5505_5510_pump::device_stop()
{
	m_timer->enable(false);
}

void esq_5505_5510_pump::device_reset()
{
	INT64 nsec_per_sample = 100 * 16 * 21;
	attotime sample_time(0, 1000000000 * nsec_per_sample);
	attotime initial_delay(0, 0);

	m_timer->adjust(initial_delay, 0, sample_time);
	m_timer->enable(true);
}

void esq_5505_5510_pump::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (samples != 1) {
		logerror("Pump: request for %d samples\n", samples);
	}

	stream_sample_t *left = outputs[0], *right = outputs[1];
	for (int i = 0; i < samples; i++)
	{
#define SAMPLE_SHIFT 4
		// anything for the 'aux' output?
		INT16 l = inputs[0][i] >> SAMPLE_SHIFT;
		INT16 r = inputs[1][i] >> SAMPLE_SHIFT;

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
		INT16 ll = m_esp->ser_r(6);
		INT16 rr = m_esp->ser_r(7);
		l += ll;
		r += rr;

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
		// if we're processing the fake program through the ESP, the result should just be that of adding the inputs
		INT32 el = (inputs[2][i]) + (inputs[4][i]) + (inputs[6][i]);
		INT32 er = (inputs[3][i]) + (inputs[5][i]) + (inputs[7][i]);
		INT32 e_next = el + er;
		e[(ei + 0x1d0f) % 0x4000] = e_next;

		if (l != e[ei]) {
			fprintf(stderr, "expected (%d) but have (%d)\n", e[ei], l);
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
			fprintf(stderr, ".-*\n");
		} else {
			fprintf(stderr, "*-.\n");
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
		fprintf(stderr, "Pump: %d samples in %" I64FMT "d ticks for %f Hz\n", last_samples, elapsed, last_samples * (double)tps / (double)elapsed);
		last_ticks = now;
		while (next_report_ticks <= now) {
			next_report_ticks += tps;
		}
		last_samples = 0;

#if !PUMP_FAKE_ESP_PROCESSING
		fprintf(stderr, "  ESP spent %" I64FMT "d ticks on %d samples, %f ticks per sample\n", ticks_spent_processing, samples_processed, (double)ticks_spent_processing / (double)samples_processed);
		ticks_spent_processing = 0;
		samples_processed = 0;
#endif
	}
#endif
}

void esq_5505_5510_pump::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) {
	// ecery time there's a new sample period, update the stream!
	m_stream->update();
}
