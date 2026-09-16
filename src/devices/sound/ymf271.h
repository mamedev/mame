// license:BSD-3-Clause
// copyright-holders:superctr
#ifndef MAME_SOUND_YMF271_H
#define MAME_SOUND_YMF271_H

#pragma once

#include "dirom.h"

class ymf271_device : public device_t, public device_sound_interface, public device_rom_interface<23>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	ymf271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	struct opx_alg
	{
		uint8_t mods[4];
		uint8_t car;
		uint8_t fbsrc;
	};

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	TIMER_CALLBACK_MEMBER(timer_a_expired);
	TIMER_CALLBACK_MEMBER(timer_b_expired);

private:
	static constexpr int NUM_SLOTS = 48;
	static constexpr int NUM_GROUPS = 12;

	struct opx_slot
	{
		// function registers (decoded)
		uint8_t kon;
		uint8_t ext_en;
		uint8_t ext_out;
		uint8_t lfo_freq;
		uint8_t ams, pms, lfo_wave;
		uint8_t dt, mul;
		uint8_t tl;
		uint8_t ks, ar;
		uint8_t d1r;
		uint8_t d2r;
		uint8_t d1l, rr;
		uint16_t fnum;
		uint8_t block;
		uint8_t fnum_latch;
		uint8_t accon, fb, wave;
		uint8_t alg;
		uint8_t ch_level[4];

		// PCM attribute registers (only meaningful if slot % 4 == 0)
		uint32_t pcm_start;
		uint32_t pcm_end;
		uint32_t pcm_loop;
		uint8_t pcm_altloop;
		uint8_t pcm_fs;
		uint8_t pcm_12bit;
		uint8_t pcm_srcnote, pcm_srcb;

		// runtime state
		int8_t block_s;
		uint8_t keycode;
		uint8_t eg_state;
		int32_t eg_att;
		uint32_t phase;
		int32_t out;
		int32_t acc;
		int32_t fb_hist[2];
		uint32_t lfo_cnt;
		uint8_t lfo_pos;
		uint32_t pcm_pos;
		uint32_t pcm_frac;
		uint8_t pcm_ended;   // End flag already raised since the last key-on

		// connection cache (rebuilt when sync/algorithm changes)
		uint8_t c_nmod;
		uint8_t c_mod[3];
		uint8_t c_fbhead;
		int8_t c_fbtarget;
		uint8_t c_carrier;
	};

	struct opx_group
	{
		uint8_t sync;
		uint8_t pfm;
		uint8_t dirty;
	};

	void init_tables();
	void init_state();

	bool is_keyon_slot(int bank, int group) const;
	int voice_slots(int bank, int group, int *slots) const;
	static void update_keycode(opx_slot &s, int slotnum);
	void slot_keyon(int slotnum);
	void slot_keyoff(int slotnum);
	void write_slot_reg(int slotnum, int reg, uint8_t data);
	void write_fm(int bank, uint8_t address, uint8_t data);
	void write_pcm(uint8_t address, uint8_t data);
	void write_util(uint8_t address, uint8_t data);
	void update_irq();

	void connect(const opx_alg &alg, const int *slots, int n);
	void rebuild_group(int g);
	void eg_tick(opx_slot &s);
	static int eg_rate(int rate2, int rks);
	static uint32_t lfo_period(uint8_t n);
	static void lfo_tick(opx_slot &s);
	static int32_t lfo_pm(const opx_slot &s);
	static int32_t lfo_am(const opx_slot &s);
	static uint32_t phase_inc(const opx_slot &s, int32_t lfo_pm);
	static uint32_t pcm_step(const opx_slot &s, int32_t lfo_pm);
	int32_t pcm_word(const opx_slot &s, uint32_t pos);
	int32_t pcm_sample(opx_slot &s, int slotnum, int32_t lfo_pm);
	int32_t env_mul(int32_t v, uint32_t env) const;
	int32_t op(uint32_t phase, int wave, uint32_t env) const;
	static int32_t pan(int32_t v, uint8_t level);

	// lookup tables (generated at start)
	uint16_t m_logsin[256];
	uint16_t m_exp[256];

	// internal state
	opx_slot m_slots[NUM_SLOTS];
	opx_group m_groups[NUM_GROUPS];

	uint8_t m_regs_main[0x10];

	uint16_t m_timerA;
	uint8_t m_timerB;
	uint8_t m_timer_ctrl;
	uint8_t m_status;
	uint16_t m_end_status;
	uint8_t m_irqstate;

	uint32_t m_ext_address;
	uint8_t m_ext_rw;
	uint8_t m_ext_readlatch;

	uint32_t m_eg_cnt;
	uint8_t m_eg_phase;

	uint32_t m_master_clock;

	emu_timer *m_timA;
	emu_timer *m_timB;
	sound_stream *m_stream;

	devcb_write_line m_irq_handler;
};

DECLARE_DEVICE_TYPE(YMF271, ymf271_device)

#endif // MAME_SOUND_YMF271_H
