// license:GPL-2.0+
// copyright-holders:Matthew Conte
/***************************************************************************

  MAME/MESS NES APU CORE

***************************************************************************/

#ifndef MAME_SOUND_NES_APU_H
#define MAME_SOUND_NES_APU_H

#pragma once

#include "nes_defs.h"

class cpu_device;
class m6502_device;
//class nes_exrom_device;
class ppu2c0x_device;

class nesapu_device : public device_t, public device_sound_interface
{
public:
	nesapu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq()      { return m_irq_handler.bind(); }
	auto mem_read() { return m_mem_read_cb.bind(); }

	virtual void device_reset() override;
	virtual void device_clock_changed() override;
	void calculate_rates();

	void do_oam_dma(address_space &space, u8 page);

	u8 read(offs_t offset);
	u8 status_r();
	void write(offs_t offset, u8 data);

	void tick_apu();

	void update_irq_output();
	void set_frame_irq_flag_only();

	void presave();
	void postload();

	emu_timer *m_apu_timer;

protected:
	nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_stop() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:

	/* GLOBAL CONSTANTS */
	//static constexpr unsigned  SYNCS_MAX1     = 0x20;
	//static constexpr unsigned  SYNCS_MAX2     = 0x80;
	static constexpr u32       NTSC_APU_CLOCK = 21477272 / 12;
	static constexpr u32       PAL_APU_CLOCK  = 26601712 / 16;
	//============================================================
	//  Types
	//============================================================

	int pal_cpu_ppu = 0;

	enum Frame_counter_mode : u8
	{
		FOUR_STEP = 0,
		FIVE_STEP = 1
	};

	enum class dma_cycle_kind : u8
	{
		get,
		put
	};

	enum class dmc_dma_mode : u8
	{
		none,

		// Normal DMC sample-byte load caused by $4015 enabling DMC with an
		// empty sample buffer.
		normal_load,

		// Normal DMC sample-byte reload caused by the output unit emptying the
		// sample buffer during playback.
		normal_reload,

		// Explicit stop bug:
		// $4015 disables DMC exactly as reload is scheduled. These are
		// aborted-DMA timing buckets, not normal sample fetches.
		explicit_abort_0,

		// Explicit stop bug:
		// $4015 disables DMC one CPU cycle before reload.
		explicit_abort_minus_1,

		// Explicit stop bug:
		// $4015 disables DMC two or three CPU cycles before reload.
		// This is cleanup-only in dmc_read(); it must not perform a real DMC get.
		explicit_abort_minus_2_or_3,

		// Implicit stop bug, 8/9 case.
		//
		// Two-stage behavior:
		//   1. First stage performs a normal DMC halt/dummy/read.
		//   2. After that read, dma_engine_finish_dmc_read() rearms a second
		//      halt window with implicit_abort_halt_reads_required = 2.
		//
		// If that second halt is delayed by a CPU write cycle, the aborted DMA
		// does not occur at all.
		implicit_abort_8_9,

		// Implicit stop bug, 6/7 case.
		//
		// Unexpected replay behavior:
		// after the first read, the engine backs up the DMC address/count so
		// the same byte can be fetched again through a second halt/dummy/read
		// sequence.
		implicit_unexpected_6_7
	};

	enum class dmc_dma_phase : u8
	{
		idle,
		waiting_for_halt,

		// DMC dummy/no-op cycle. The halted CPU repeats the last read unless
		// OAM uses the bus on this same get/put cycle.
		dummy,

		// DMC get/read phase. The actual sample fetch can only happen on a GET
		// cycle. If this phase is reached on PUT, the runner treats that tick
		// as the DMC alignment no-op without needing a separate enum state.
		read,

		done
	};

	enum class oam_dma_phase : u8
	{
		idle,
		waiting_for_halt,

		align,
		read,
		write,

		done
	};

	struct dmc_dma_transaction
	{
		dmc_dma_mode  mode  = dmc_dma_mode::none;
		dmc_dma_phase phase = dmc_dma_phase::idle;

		bool active = false;

		// True only for a $4015 enable/load DMA. Reload DMAs leave this false.
		bool load_request = false;

		// Set after the first implicit 6/7 read. The next DMC halt consumes
		// the replay path by backing address/count up and doing one more read.
		bool replay_pending = false;

		// Set when the next completed DMC read must clear implicit bookkeeping.
		bool clear_implicit_after_read = false;

		// True for the late/short implicit 6/7 loop path.
		bool late_implicit_short = false;

		// Implicit stop-bug halt-read requirement.
		//
		// For implicit_abort_8_9:
		//   0 = first stage: perform normal DMC halt/dummy/read
		//   2 = second stage: one-cycle aborted DMA halt window
		//
		// If a CPU write delays the second-stage halt, the aborted DMA is
		// cancelled completely.
		//
		// For implicit_unexpected_6_7 this remains 0; replay state is tracked
		// by replay_pending / clear_implicit_after_read.
		int implicit_abort_halt_reads_required = 0;

		// Countdown before a DMC halt request becomes visible to the CPU.
		int pending_delay = 0;

		// Number of CPU cycles observed while the halt request is waiting.
		int halt_read_cycles = 0;
		int halt_write_cycles = 0;
	};

	struct oam_dma_transaction
	{
		oam_dma_phase phase = oam_dma_phase::idle;

		bool active = false;

		u16 base_addr = 0;
		u16 index = 0;
	};

	struct nes_dma_engine
	{
		// Shared DMA/open-bus data latch used by OAM and DMC DMA.
		u8 data_bus = 0;

		dmc_dma_transaction dmc;
		oam_dma_transaction oam;
	};

	//============================================================
	//  Devices
	//============================================================

	cpu_device     *m_maincpu_dev;
	m6502_device   *m_maincpu6502;
	ppu2c0x_device *m_ppu_dev;
	//nes_exrom_device *m_mmc5;

	//============================================================
	//  Stream / mixer
	//============================================================

	sound_stream *m_stream;

	devcb_write_line m_irq_handler;
	devcb_read8      m_mem_read_cb;

	static constexpr u32 OUT_FIFO_SIZE = 2048;

	sound_stream::sample_t m_hp90_prev_in;
	sound_stream::sample_t m_hp90_prev_out;

	sound_stream::sample_t m_hp440_prev_in;
	sound_stream::sample_t m_hp440_prev_out;

	sound_stream::sample_t m_lp14k_prev_out;

	sound_stream::sample_t m_output_accum;
	sound_stream::sample_t m_out_fifo[OUT_FIFO_SIZE];
	u32 m_out_fifo_r;
	u32 m_out_fifo_w;
	sound_stream::sample_t m_last_out_sample;

	u64 m_resample_phase;
	u64 m_resample_step;

	//bool m_output_dirty;
	uint64_t m_audio_fifo_overflows = 0;
	uint64_t m_audio_fifo_underflows = 0;
	sound_stream::sample_t m_cached_output;

	sound_stream::sample_t m_square_lut[31];
	sound_stream::sample_t m_tnd_lut[16][16][128];

	//============================================================
	//  Core APU state
	//============================================================

	apu_t m_APU;
	u8    m_is_pal;

	// CPU/APU phase and PPU clock gate.
	u64  cpu_cycle;
	bool apu_clk1_is_high;
	bool cpu_reading;
	bool run_ppu;

	// Latched CPU read address used by DMC/APU bus conflict handling.
	u16 m_dmc_cpu_bus_latch;

	//============================================================
	//  Delayed register effects used by timing tests
	//============================================================

	// 10.len_halt_timing
	bool temp_halt_len_loop_env_0;
	bool temp_halt_len_loop_env_1;
	bool temp_tri_halt_flag;
	bool temp_noise_halt_len_loop_env;

	s32 delay_halt_len_loop_0;
	s32 delay_halt_len_loop_1;
	s32 delay_tri_halt_flag;
	s32 delay_noise_halt_len_loop_env;

	// 11.len_reload_timing
	u8 temp_len_cnt_0;
	u8 temp_len_cnt_0_reload_value;
	u8 temp_len_cnt_1;
	u8 temp_len_cnt_1_reload_value;
	u8 temp_tri_len_cnt;
	u8 temp_tri_len_cnt_reload_value;
	u8 temp_noise_len_cnt;
	u8 temp_noise_len_cnt_reload_value;

	s32 delay_len_cnt_0;
	s32 delay_len_cnt_1;
	s32 delay_tri_len_cnt;
	s32 delay_noise_len_cnt;

	//============================================================
	//  DMC special-case timing / bug handling
	//============================================================

	s32 dmc_cycles_until_buffer_empty;
	s32 dmc_cycles_since_buffer_empty;

	bool dmc_4015_load_defer_pending;
	s32  dmc_4015_load_defer_delay;

	//bool dmc_4011_write_pending;
	//u8   dmc_4011_old_counter;

	//============================================================
	//  Debug/stat counters
	//============================================================

	s32 detect_abort_1;
	s32 detect_abort_2_3;
	s32 detect_abort_0;
	s32 detect_abort_8_9;
	s32 detect_test_l;
	s32 detect_test_m;

	//============================================================
	//  IRQ / frame counter state
	//============================================================

	bool frame_irq;
	bool frame_irq_output;
	bool dmc_irq;
	bool inhibit_frame_irq;

	Frame_counter_mode frame_counter_mode;
	Frame_counter_mode new_frame_counter_mode;

	u32 delayed_frame_timer_reset;
	u32 frame_counter_clock;
	uint64_t last_frame_unit_pulse_apu_cycle;

	s32 frame_unit_clock_block_until;

	s32  delayed_frame_irq;
	bool delayed_frame_irq_after_dmc;
	s32  delayed_frame_irq_clear;
	s32  delayed_dmc_irq;

	//s32 frame_irq_no_clear_before;
	s32 frame_irq_suppress_clear_cycle;

	// Save-state mirrors for enum class values.
	int m_save_frame_counter_mode;
	int m_save_new_frame_counter_mode;

	int m_save_dmc_dma_mode;
	int m_save_dmc_dma_phase;
	int m_save_oam_dma_phase;

	//============================================================
	//  DMC channel
	//============================================================

	u32 dmc_counter;

	bool dmc_loading_sample_byte;
	bool dmc_sample_buffer_has_data;
	bool dpcm_active;

	bool dmc_irq_enabled;
	bool dmc_loop_sample;

	u32 dmc_period;
	s32 dmc_period_cnt;

	u32 dmc_sample_start_addr;
	u32 dmc_sample_len;

	u8  dmc_sample_buffer;
	u8  dmc_shift_reg;
	u16 dmc_sample_cur_addr;
	u32 dmc_bytes_remaining;
	u32 dmc_bits_remaining;

	u16 const *dmc_periods;

	//============================================================
	//  Triangle channel
	//============================================================

	u32  tri_output_level;
	bool tri_enabled;

	u32 tri_period;
	s32 tri_period_cnt;

	u32 tri_waveform_pos;
	s32 tri_len_cnt;

	bool tri_halt_flag;
	u32  tri_lin_cnt_load;
	s32  tri_lin_cnt;
	bool tri_lin_cnt_reload_flag;

	//============================================================
	//  Noise channel
	//============================================================

	u32  noise_output_level;
	bool noise_enabled;
	bool noise_halt_len_loop_env;
	bool noise_const_vol;

	u32 noise_vol;
	u32 noise_feedback_bit;
	u32 noise_period;
	s32 noise_period_cnt;
	s32 noise_len_cnt;

	u32  noise_shift_reg;
	bool noise_env_start_flag;
	u32  noise_env_vol;
	u32  noise_env_div_cnt;

	u16 const *noise_periods;

	//============================================================
	//  NES DMA engine
	//============================================================

	nes_dma_engine m_dma_engine;

	//============================================================
	//  Helpers - mixer / output
	//============================================================

	sound_stream::sample_t calc_current_output();
	sound_stream::sample_t apply_analog_filter(sound_stream::sample_t in);

	void push_out_sample(sound_stream::sample_t sample);
	bool pop_out_sample(sound_stream::sample_t &sample);
	void accumulate_output_sample(sound_stream::sample_t level);

	//============================================================
	//  Helpers - APU units
	//============================================================

	bool frame_unit_clock_allowed();
	void arm_frame_unit_clock_block();

	void update_sweep_target_period(unsigned n);
	void update_pulse_output_level(unsigned n);
	void update_noise_output_level();

	void clock_pulse_generator(unsigned n);
	void clock_frame_counter();
	void clock_triangle_generator();
	void clock_noise_generator();
	void clock_dmc();
	void clock_env_and_tri_lin();
	void clock_len_and_sweep();

	void set_frame_irq(bool s);
	void set_dmc_irq(bool s);
	void check_frame_irq();

	void load_dmc_sample_byte();
	void dmc_read();

	void tick();

	TIMER_CALLBACK_MEMBER(apu_tick);

	//============================================================
	//  Helpers - DMA engine
	//============================================================

	void dma_engine_reset();
	void dma_engine_request_oam(u16 base_addr);
	void dma_engine_request_dmc(dmc_dma_mode mode);

	dma_cycle_kind dma_engine_cycle_kind() const;
	bool dma_engine_is_get_cycle() const;
	bool dma_engine_is_put_cycle() const;

	bool dma_engine_dmc_waiting_for_halt() const;
	bool dma_engine_oam_waiting_for_halt() const;

	void dma_engine_clear_dmc_replay_state();
	void dma_engine_mark_dmc_idle();
	void dma_engine_resume_cpu_if_done();

	void dma_engine_oam_read();
	void dma_engine_oam_write();
	void dma_engine_finish_dmc_read();

	bool dma_engine_service_oam_access_this_cycle();
	bool dma_engine_run();
};

class apu2a03_device : public nesapu_device
{
public:
	apu2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(NES_APU,  nesapu_device)
DECLARE_DEVICE_TYPE(APU_2A03, apu2a03_device)

#endif // MAME_SOUND_NES_APU_H
