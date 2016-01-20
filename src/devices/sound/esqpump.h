// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#pragma once

#ifndef _ESQPUMP_H_
#define _ESQPUMP_H_

#include "emu.h"
#include "sound/es5506.h"
#include "cpu/es5510/es5510.h"

#define PUMP_DETECT_SILENCE 0
#define PUMP_TRACK_SAMPLES 0
#define PUMP_FAKE_ESP_PROCESSING 0
#define PUMP_REPLACE_ESP_PROGRAM 0

class esq_5505_5510_pump : public device_t,
	public device_sound_interface
{
public:
	esq_5505_5510_pump(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	void set_otis(es5505_device *otis) { m_otis = otis; }
	void set_esp(es5510_device *esp) { m_esp = esp; }
	void set_esp_halted(bool esp_halted) {
		m_esp_halted = esp_halted;
		logerror("ESP-halted -> %d\n", m_esp_halted);
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
	bool get_esp_halted() {
		return m_esp_halted;
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// timer callback overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal state:
	// sound stream
	sound_stream *m_stream;

	// per-sample timer
	emu_timer *m_timer;

	// OTIS sound generator
	es5505_device *m_otis;

	// ESP signal processor
	es5510_device *m_esp;

	// Is the ESP halted by the CPU?
	bool m_esp_halted;

#if !PUMP_FAKE_ESP_PROCESSING
	osd_ticks_t ticks_spent_processing;
	int samples_processed;
#endif

#if PUMP_DETECT_SILENCE
	int silent_for;
	bool was_silence;
#endif

#if PUMP_TRACK_SAMPLES
	int last_samples;
	osd_ticks_t last_ticks;
	osd_ticks_t next_report_ticks;
#endif

#if !PUMP_FAKE_ESP_PROCESSING && PUMP_REPLACE_ESP_PROGRAM
	INT16 e[0x4000];
	int ei;
#endif
};

extern const device_type ESQ_5505_5510_PUMP;

#endif
