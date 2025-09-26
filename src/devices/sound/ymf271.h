// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
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

protected:
	// device-level overrides
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
	struct YMF271Slot
	{
		uint8_t ext_en;
		uint8_t ext_out;
		uint8_t lfoFreq;
		uint8_t lfowave;
		uint8_t pms, ams;
		uint8_t detune;
		uint8_t multiple;
		uint8_t tl;
		uint8_t keyscale;
		uint8_t ar;
		uint8_t decay1rate, decay2rate;
		uint8_t decay1lvl;
		uint8_t relrate;
		uint8_t block;
		uint8_t fns_hi;
		uint32_t fns;
		uint8_t feedback;
		uint8_t waveform;
		uint8_t accon;
		uint8_t algorithm;
		uint8_t ch0_level, ch1_level, ch2_level, ch3_level;

		uint32_t startaddr;
		uint32_t loopaddr;
		uint32_t endaddr;
		uint8_t altloop;
		uint8_t fs;
		uint8_t srcnote, srcb;

		uint32_t step;
		uint64_t stepptr;

		uint8_t active;
		uint8_t bits;

		// envelope generator
		int32_t volume;
		int32_t env_state;
		int32_t env_attack_step;      // volume increase step in attack state
		int32_t env_decay1_step;
		int32_t env_decay2_step;
		int32_t env_release_step;

		int64_t feedback_modulation0;
		int64_t feedback_modulation1;

		int lfo_phase, lfo_step;
		int lfo_amplitude;
		double lfo_phasemod;
	};

	struct YMF271Group
	{
		uint8_t sync, pfm;
	};

	void init_state();
	void init_tables();
	void calculate_clock_correction();
	void calculate_step(YMF271Slot *slot);
	void update_envelope(YMF271Slot *slot);
	void init_envelope(YMF271Slot *slot);
	void init_lfo(YMF271Slot *slot);
	void update_lfo(YMF271Slot *slot);
	int64_t calculate_slot_volume(YMF271Slot *slot);
	void update_pcm(int slotnum, int32_t *mixp, int length);
	int64_t calculate_op(int slotnum, int64_t inp);
	void set_feedback(int slotnum, int64_t inp);
	void write_register(int slotnum, int reg, uint8_t data);
	void ymf271_write_fm(int bank, uint8_t address, uint8_t data);
	void ymf271_write_pcm(uint8_t address, uint8_t data);
	void ymf271_write_timer(uint8_t address, uint8_t data);

	inline int get_keyscaled_rate(int rate, int keycode, int keyscale);
	inline int get_internal_keycode(int block, int fns);
	inline int get_external_keycode(int block, int fns);
	inline bool check_envelope_end(YMF271Slot *slot);
	inline void calculate_status_end(int slotnum, bool state);

	// lookup tables
	std::unique_ptr<int16_t[]> m_lut_waves[8];
	std::unique_ptr<double[]> m_lut_plfo[4][8];
	std::unique_ptr<int[]> m_lut_alfo[4];
	double m_lut_ar[64];
	double m_lut_dc[64];
	double m_lut_lfo[256];
	int m_lut_attenuation[16];
	int m_lut_total_level[128];
	int m_lut_env_volume[256];

	// internal state
	YMF271Slot m_slots[48];
	YMF271Group m_groups[12];

	uint8_t m_regs_main[0x10];

	uint32_t m_timerA;
	uint32_t m_timerB;
	uint8_t m_irqstate;
	uint8_t m_status;
	uint16_t m_end_status;
	uint8_t m_enable;

	uint32_t m_ext_address;
	uint8_t m_ext_rw;
	uint8_t m_ext_readlatch;

	uint32_t m_master_clock;

	emu_timer *m_timA;
	emu_timer *m_timB;
	sound_stream *m_stream;
	std::vector<int32_t> m_mix_buffer;

	devcb_write_line m_irq_handler;
};

DECLARE_DEVICE_TYPE(YMF271, ymf271_device)

#endif // MAME_SOUND_YMF271_H
