// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  Ensoniq 5505/5506 to 5510 interface.

  Modeled specifically after the routing of and for use with
  the VFX family of keyboards.

  By Christian Brunschen

***************************************************************************/

#include "emu.h"
#include "esqpump.h"

#include <iostream>


#define PUMP_TRACK_ESP_HALTED 0
#define PUMP_DETECT_SILENCE 0
#define PUMP_TRACK_SAMPLES 0
#define PUMP_TRACK_PROCESSING 0
#define PUMP_FAKE_ESP_PROCESSING 0
#define PUMP_REPLACE_ESP_PROGRAM 0


#define LOG_ESP_HALTED (1U << 1)

#if PUMP_TRACK_ESP_HALTED
#define VERBOSE LOG_ESP_HALTED
#endif

#include "logmacro.h"

namespace sound::esqpump {

struct stats {
#if PUMP_TRACK_PROCESSING && !PUMP_FAKE_ESP_PROCESSING
	osd_ticks_t ticks_spent_processing = 0;
	int samples_processed = 0;
#endif

#if PUMP_DETECT_SILENCE
	int silent_for = 0;
	bool was_silence = true;
#endif

#if PUMP_TRACK_SAMPLES
	int last_samples;
	osd_ticks_t last_ticks;
	osd_ticks_t next_report_ticks;
#endif

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	std::vector<sound_stream::sample_t> e;
	int ei;
#endif
};

}

DEFINE_DEVICE_TYPE(ESQ_5505_5510_PUMP, esq_5505_5510_pump_device, "esq_5505_5510_pump", "Ensoniq 5505/5506 to 5510 interface")

esq_5505_5510_pump_device::esq_5505_5510_pump_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ESQ_5505_5510_PUMP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_esp(*this, finder_base::DUMMY_TAG)
	, m_esp_halted(true)
	, m_stats(std::make_unique<sound::esqpump::stats>())
{
}

void esq_5505_5510_pump_device::set_esp_halted(bool esp_halted) {
	m_esp_halted = esp_halted;
	LOGMASKED(LOG_ESP_HALTED, "ESP-halted -> %d\n", static_cast<int>(m_esp_halted));
	if (!esp_halted) {
#if PUMP_REPLACE_ESP_PROGRAM
		m_esp->write_reg(245, 0x1d0f << 8); // dlength = 0x3fff, 16-sample delay

		int pc = 0;
		for (pc = 0; pc < 0xc0; pc++) {
			m_esp->write_reg(pc, 0);
		}
		pc = 0;
		// replace the ESP program with a simple summing & single-sample delay
		m_esp->_instr(pc++) = 0xffffeaa09000; // MOV SER0R > grp_a0
		m_esp->_instr(pc++) = 0xffffeba00000; // ADD SER0L, gpr_a0 > gpr_a0
		m_esp->_instr(pc++) = 0xffffeca00000; // ADD SER1R, gpr_a0 > gpr_a0
		m_esp->_instr(pc++) = 0xffffeda00000; // ADD SER1L, gpr_a0 > gpr_a0
		m_esp->_instr(pc++) = 0xffffeea00000; // ADD SER2R, gpr_a0 > gpr_a0

		m_esp->_instr(pc  ) = 0xffffefa00000; // ADD SER2L, gpr_a0 > gpr_a0; prepare to read from delay 2 instructions from now, offset = 0
		m_esp->write_reg(pc++, 0); //offset into delay

		m_esp->_instr(pc  ) = 0xffffa0a09508; // MOV gpr_a0 > delay + offset
		m_esp->write_reg(pc++, 1 << 8); // offset into delay - -1 samples

		m_esp->_instr(pc++) = 0xffff00a19928; // MOV DIL > gpr_a1; read Delay and dump FIFO (so that the value gets written)

		m_esp->_instr(pc++) = 0xffffa1f09000; // MOV gpr_a1 > SER3R
		m_esp->_instr(pc++) = 0xffffa1f19000; // MOV gpr_a1 > SER3L

		m_esp->_instr(pc++) = 0xffffffff0000; // NO-OP
		m_esp->_instr(pc++) = 0xffffffff0000; // NO-OP
		m_esp->_instr(pc++) = 0xfffffffff000; // END

		while (pc < 160) {
			m_esp->_instr(pc++) = 0xffffffffffff; // no-op
		}
#endif

		// m_esp->list_program(print_to_stderr);
	}
}

void esq_5505_5510_pump_device::device_start()
{
	// The VFX only has a single pair of stereo outputs, 'Main'; these will be channels 0 and 1,
	// and will be routed to the 'speaker' output device.
	// VFX-SD and later have a separate 'Aux' stereo output that bypasses ESP effect processing;
	// these will be channels 2 and 3 and can be routed to a separate 'aux' output device.
	// On the VFX, those will simply remain silent.
	m_stream = stream_alloc(8, 4, clock(), STREAM_SYNCHRONOUS);

#if PUMP_DETECT_SILENCE
	m_stats->silent_for = 500;
	m_stats->was_silence = 1;
#endif

#if PUMP_TRACK_PROCESSING && !PUMP_FAKE_ESP_PROCESSING
	m_stats->ticks_spent_processing = 0;
	m_stats->samples_processed = 0;
#endif

#if PUMP_TRACK_SAMPLES
	m_stats->last_samples = 0;
	m_stats->last_ticks = osd_ticks();
	m_stats->next_report_ticks = m_stats->last_ticks + osd_ticks_per_second();
#endif

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	m_stats->e.resize(0x4000);
	m_stats->ei = 0;
#endif
}

void esq_5505_5510_pump_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
}

void esq_5505_5510_pump_device::sound_stream_update(sound_stream &stream)
{
	constexpr sound_stream::sample_t input_scale = 32768.0;
	constexpr sound_stream::sample_t output_scale = 1.0 / input_scale;

	if (stream.samples() != 1)
	{
		logerror("esq_5505_5510_pump processes one sample at a time!\n");
	}

	// Push the 'Aux' output samples directly into the output stream
	stream.put(2, 0, stream.get(0, 0));
	stream.put(3, 0, stream.get(1, 0));

	// Push the 'FX1', 'FX2' and 'DRY' samples into the ESP
	m_esp->ser_w(0, s32(stream.get(2, 0) * input_scale));
	m_esp->ser_w(1, s32(stream.get(3, 0) * input_scale));
	m_esp->ser_w(2, s32(stream.get(4, 0) * input_scale));
	m_esp->ser_w(3, s32(stream.get(5, 0) * input_scale));
	m_esp->ser_w(4, s32(stream.get(6, 0) * input_scale));
	m_esp->ser_w(5, s32(stream.get(7, 0) * input_scale));

#if PUMP_FAKE_ESP_PROCESSING
	m_esp->ser_w(6, m_esp->ser_r(0) + m_esp->ser_r(2) + m_esp->ser_r(4));
	m_esp->ser_w(7, m_esp->ser_r(1) + m_esp->ser_r(3) + m_esp->ser_r(5));
#else

if (!m_esp_halted) {
#if PUMP_TRACK_PROCESSING
		osd_ticks_t a = osd_ticks();
#endif

m_esp->run_once();

#if PUMP_TRACK_PROCESSING
		osd_ticks_t b = osd_ticks();
		m_stats->ticks_spent_processing += (b - a);
		m_stats->samples_processed++;
#endif
	}
#endif

	// Read the processed result from the ESP.
	sound_stream::sample_t l = sound_stream::sample_t(m_esp->ser_r(6)) * output_scale;
	sound_stream::sample_t r = sound_stream::sample_t(m_esp->ser_r(7)) * output_scale;

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	// if we're processing the fake program through the ESP, the result should just be that of adding the inputs
	sound_stream::sample_t el = (stream.get(2, 0)) + (stream.get(4, 0)) + (stream.get(6, 0));
	sound_stream::sample_t er = (stream.get(3, 0)) + (stream.get(5, 0)) + (stream.get(7, 0));
	sound_stream::sample_t e_next = el + er;
	m_stats->e[(m_stats->ei + 0x1d0f) % 0x4000] = e_next;

	if (fabs(l - m_stats->e[m_stats->ei]) > 1e-5) {
		util::stream_format(std::cerr, "expected (%d) but have (%d)\n", m_stats->e[m_stats->ei], l);
	}
	m_stats->ei = (m_stats->ei + 1) % 0x4000;
#endif

	// Write the Processed samples to the output
	stream.put(0, 0, l);
	stream.put(1, 0, r);

#if PUMP_DETECT_SILENCE
	if (l == 0 && r == 0) {
		m_stats->silent_for++;
	} else {
		m_stats->silent_for = 0;
	}
	bool silence = m_stats->silent_for >= 500;
	if (m_stats->was_silence != silence) {
		if (!silence) {
			util::stream_format(std::cerr, ".-*\n");
		} else {
			util::stream_format(std::cerr, "*-.\n");
		}
		m_stats->was_silence = silence;
	}
#endif

#if PUMP_TRACK_SAMPLES
	m_stats->last_samples++;
	osd_ticks_t now = osd_ticks();
	if (now >= m_stats->next_report_ticks)
	{
		osd_ticks_t elapsed = now - m_stats->last_ticks;
		osd_ticks_t tps = osd_ticks_per_second();
		util::stream_format(std::cerr, "Pump: %d samples in %d ticks for %f Hz\n", m_stats->last_samples, elapsed, m_stats->last_samples * (double)tps / (double)elapsed);
		m_stats->last_ticks = now;
		while (m_stats->next_report_ticks <= now) {
			m_stats->next_report_ticks += tps;
		}
		m_stats->last_samples = 0;

#if !PUMP_FAKE_ESP_PROCESSING
		util::stream_format(std::cerr, "  ESP spent %d ticks on %d samples, %f ticks per sample\n", m_stats->ticks_spent_processing, m_stats->samples_processed, (double)m_stats->ticks_spent_processing / (double)m_stats->samples_processed);
		m_stats->ticks_spent_processing = 0;
		m_stats->samples_processed = 0;
#endif
	}
#endif
}
