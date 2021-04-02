// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert, hap
#ifndef MAME_SOUND_YMF278B_H
#define MAME_SOUND_YMF278B_H

#pragma once

#include "dirom.h"
#include "sound/ymfm.h"

class ymf278b_device : public device_t, public device_sound_interface, public device_rom_interface<22>
{
public:
	static constexpr u8 STATUS_BUSY = 0x01;
	static constexpr u8 STATUS_LD = 0x02;

	// YMF278B is OPL4
	using fm_engine = ymopl4_engine;

	// constructor
	ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_fm.irq_handler(); }

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	struct YMF278BSlot
	{
		int16_t wave;     /* wavetable number */
		int16_t F_NUMBER; /* frequency */
		int8_t octave;    /* octave */
		int8_t preverb;   /* pseudo-reverb */
		int8_t DAMP;      /* damping */
		int8_t CH;        /* output channel */
		int8_t LD;        /* level direct */
		int8_t TL;        /* total level */
		int8_t pan;       /* panpot */
		int8_t LFO;       /* LFO */
		int8_t VIB;       /* vibrato */
		int8_t AM;        /* tremolo */

		int8_t AR;        /* attack rate */
		int8_t D1R;       /* decay 1 rate */
		int8_t DL;        /* decay level */
		int8_t D2R;       /* decay 2 rate */
		int8_t RC;        /* rate correction */
		int8_t RR;        /* release rate */

		uint32_t step;    /* fixed-point frequency step */
		uint64_t stepptr; /* fixed-point pointer into the sample */

		int8_t active;    /* channel is playing */
		int8_t KEY_ON;    /* slot keyed on */
		int8_t bits;      /* width of the samples */
		uint32_t startaddr;
		uint32_t loopaddr;
		uint32_t endaddr;

		int env_step;
		uint32_t env_vol;
		uint32_t env_vol_step;
		uint32_t env_vol_lim;
		int8_t env_preverb;

		int num;        /* slot number (for debug only) */
	};

	int compute_rate(YMF278BSlot *slot, int val);
	uint32_t compute_decay_env_vol_step(YMF278BSlot *slot, int val);
	void compute_freq_step(YMF278BSlot *slot);
	void compute_envelope(YMF278BSlot *slot);
	void irq_check();
	void retrigger_sample(YMF278BSlot *slot);
	void C_w(uint8_t reg, uint8_t data);
	void timer_busy_start(int is_pcm);
	void precompute_rate_tables();
	void register_save_state();

	// internal state
	uint8_t m_pcmregs[256];
	YMF278BSlot m_slots[24];
	int8_t m_wavetblhdr;
	int8_t m_memmode;
	int32_t m_memadr;

	emu_timer *m_timer_busy;
	emu_timer *m_timer_ld;

	int32_t m_fm_l, m_fm_r;
	int32_t m_pcm_l, m_pcm_r;

	uint32_t m_fm_pos;

	uint8_t m_port_C, m_port_AB, m_lastport;
	bool m_next_status_id;

	// precomputed tables
	uint32_t m_lut_ar[64];              // attack rate
	uint32_t m_lut_dr[64];              // decay rate
	int32_t m_volume[256*4];            // precalculated attenuation values with some margin for envelope and pan levels
	int m_pan_left[16],m_pan_right[16]; // pan volume offsets
	int32_t m_mix_level[8];

	int m_clock;
	int m_rate;

	sound_stream * m_stream;
	std::vector<int32_t> m_mix_buffer;

	// ymf262
	fm_engine m_fm;
};

DECLARE_DEVICE_TYPE(YMF278B, ymf278b_device)

#endif // MAME_SOUND_YMF278B_H
