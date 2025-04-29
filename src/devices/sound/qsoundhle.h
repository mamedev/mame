// license:BSD-3-Clause
// copyright-holders:superctr, Valley Bell
/*********************************************************

  Capcom QSound DL-1425 (HLE)

*********************************************************/
#ifndef MAME_SOUND_QSOUNDHLE_H
#define MAME_SOUND_QSOUNDHLE_H

#pragma once

#include "cpu/dsp16/dsp16.h"
#include "dirom.h"


class qsound_hle_device : public device_t, public device_sound_interface, public device_rom_interface<24>
{
public:
	// default 60MHz clock (divided by 2 for DSP core clock, and then by 1248 for sample rate)
	qsound_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 60'000'000);

	void qsound_w(offs_t offset, uint8_t data);
	uint8_t qsound_r();

protected:
	// device_t implementation
	tiny_rom_entry const *device_rom_region() const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:

	// DSP ROM sample map
	enum {
		DATA_PAN_TAB        = 0x110,
		DATA_ADPCM_TAB      = 0x9dc,
		DATA_FILTER_TAB     = 0xd53,    // dual filter mode, 5 tables * 95 taps each
		DATA_FILTER_TAB2    = 0xf2e,    // overlapping data (95+15+95)

		STATE_BOOT          = 0x000,
		STATE_INIT1         = 0x288,
		STATE_INIT2         = 0x61a,
		STATE_REFRESH1      = 0x039,
		STATE_REFRESH2      = 0x04f,
		STATE_NORMAL1       = 0x314,
		STATE_NORMAL2       = 0x6b2
	};

	const uint16_t PAN_TABLE_DRY = 0;
	const uint16_t PAN_TABLE_WET = 98;
	const uint16_t PAN_TABLE_CH_OFFSET = 196;
	const uint16_t FILTER_ENTRY_SIZE = 95;
	const uint16_t DELAY_BASE_OFFSET = 0x554;
	const uint16_t DELAY_BASE_OFFSET2 = 0x53c;

	struct qsound_voice {
		uint16_t m_bank = 0;
		int16_t m_addr = 0; // top word is the sample address
		uint16_t m_phase = 0;
		uint16_t m_rate = 0;
		int16_t m_loop_len = 0;
		int16_t m_end_addr = 0;
		int16_t m_volume = 0;
		int16_t m_echo = 0;

		int16_t update(qsound_hle_device &dsp, int32_t *echo_out);
	};

	struct qsound_adpcm {
		uint16_t m_start_addr = 0;
		uint16_t m_end_addr = 0;
		uint16_t m_bank = 0;
		int16_t m_volume = 0;
		uint16_t m_flag = 0;
		int16_t m_cur_vol = 0;
		int16_t m_step_size = 0;
		uint16_t m_cur_addr = 0;

		int16_t update(qsound_hle_device &dsp, int16_t curr_sample, int nibble);
	};

	// Q1 Filter
	struct qsound_fir {
		int m_tap_count = 0;    // usually 95
		int m_delay_pos = 0;
		uint16_t m_table_pos = 0;
		int16_t m_taps[95] = { 0 };
		int16_t m_delay_line[95] = { 0 };

		int32_t apply(int16_t input);
	};

	// Delay line
	struct qsound_delay {
		int16_t m_delay = 0;
		int16_t m_volume = 0;
		int16_t m_write_pos = 0;
		int16_t m_read_pos = 0;
		int16_t m_delay_line[51] = { 0 };

		int32_t apply(const int32_t input);
		void update();
	};

	struct qsound_echo {
		uint16_t m_end_pos = 0;

		int16_t m_feedback = 0;
		int16_t m_length = 0;
		int16_t m_last_sample = 0;
		int16_t m_delay_line[1024] = { 0 };
		int16_t m_delay_pos = 0;

		int16_t apply(int32_t input);
	};

	// MAME resources
	sound_stream *m_stream;
	required_region_ptr<uint16_t> m_dsp_rom;

	uint16_t m_data_latch;
	int16_t m_out[2];

	qsound_voice m_voice[16];
	qsound_adpcm m_adpcm[3];

	uint16_t m_voice_pan[16+3];
	int16_t m_voice_output[16+3];

	qsound_echo m_echo;

	qsound_fir m_filter[2];
	qsound_fir m_alt_filter[2];

	qsound_delay m_wet[2];
	qsound_delay m_dry[2];

	uint16_t m_state;
	uint16_t m_next_state;

	uint16_t m_delay_update;

	int m_state_counter;
	int m_ready_flag;

	uint16_t *m_register_map[256];

	inline uint16_t read_dsp_rom(uint16_t addr) { return m_dsp_rom[addr&0xfff]; }

	void write_data(uint8_t addr, uint16_t data);
	uint16_t read_data(uint8_t addr);

	void init_register_map();
	void update_sample();

	// DSP states
	void state_init();
	void state_refresh_filter_1();
	void state_refresh_filter_2();
	void state_normal_update();

	// sub functions
	int16_t read_sample(uint16_t bank, uint16_t address);
};

DECLARE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device)

#endif // MAME_SOUND_QSOUNDHLE_H
