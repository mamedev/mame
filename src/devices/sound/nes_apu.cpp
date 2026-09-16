// license:GPL-2.0+
// copyright-holders:Matthew Conte
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES RP2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.

  timing notes:
  master = 21477270
  2A03 clock = master/12
  sequencer = master/89490 or CPU/7457

 *****************************************************************************

   NES_APU.CPP

   Actual NES APU interface.

   LAST MODIFIED 02/29/2004

 *****************************************************************************/

#include "emu.h"
#include "nes_apu.h"
#include "video/ppu2c0x.h"
#include "cpu/m6502/m6502.h"
//#include "bus/nes/mmc5.h"

DEFINE_DEVICE_TYPE(NES_APU,  nesapu_device,  "nesapu",  "RP2A0X APU")
DEFINE_DEVICE_TYPE(APU_2A03, apu2a03_device, "apu2a03", "RP2A03 APU")

nesapu_device::nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_apu_timer(nullptr)
	, m_maincpu_dev(nullptr)
	, m_maincpu6502(nullptr)
	, m_ppu_dev(nullptr)
	//, m_mmc5(nullptr)
	, m_stream(nullptr)
	, m_irq_handler(*this)
	, m_mem_read_cb(*this, 0x00)
	, m_is_pal(0)
{
}

nesapu_device::nesapu_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock)
	: nesapu_device(mconfig, NES_APU, tag, owner, clock)
{
}

apu2a03_device::apu2a03_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock)
	: nesapu_device(mconfig, APU_2A03, tag, owner, clock)
{
}

void nesapu_device::device_stop()
{
	/*osd_printf_info("\n[NES APU STATS]\n");
	osd_printf_info("\tTest L [APU Delta Modulation Channel] Detected = %d\n", detect_test_l);
	osd_printf_info("\tTest M [APU Delta Modulation Channel] Detected = %d\n", detect_test_m);
	osd_printf_info("\tExplicit DMA Abort Detected: Case 0 = %d\n", detect_abort_0);
	osd_printf_info("\tExplicit DMA Abort Detected: Case -1 = %d\n", detect_abort_1);
	osd_printf_info("\tExplicit DMA Abort Detected: Case -2 or -3 = %d\n", detect_abort_2_3);
	osd_printf_info("\tImplicit DMA Abort Detected: Case -8 or -9 = %d\n", detect_abort_8_9);
	osd_printf_info("\tAudio FIFO overflows = %llu\n", (unsigned long long)m_audio_fifo_overflows);
	osd_printf_info("\tAudio FIFO underflows = %llu\n", (unsigned long long)m_audio_fifo_underflows);
	*/
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void nesapu_device::device_clock_changed()
{
	calculate_rates();

	if (m_apu_timer != nullptr)
		m_apu_timer->adjust(clocks_to_attotime(1));
}

void nesapu_device::calculate_rates()
{
	m_is_pal = clock() == PAL_APU_CLOCK;

	if (m_is_pal)
	{
		dmc_periods = pal_dmc_periods;
		noise_periods = pal_noise_periods;
		m_maincpu6502->set_is_pal(true);
	}
	else
	{
		dmc_periods = ntsc_dmc_periods;
		noise_periods = ntsc_noise_periods;
		m_maincpu6502->set_is_pal(false);
	}

	if (m_stream != nullptr)
	{
		m_resample_step = uint64_t(double(clock()) * 4294967296.0 / double(m_stream->sample_rate()));
		assert(m_resample_step > 0);
	}
}

void nesapu_device::device_start()
{
	m_maincpu_dev = machine().root_device().subdevice<cpu_device>("maincpu");
	m_maincpu6502 = machine().root_device().subdevice<m6502_device>("maincpu");
	m_ppu_dev = machine().root_device().subdevice<ppu2c0x_device>("ppu");
	//m_mmc5 = machine().root_device().subdevice<nes_exrom_device>("nes_slot:exrom");
	pal_cpu_ppu = 0;
	// --------------------------------------------------
	// Deterministic startup initialization.
	//
	// Keep these here instead of initializing members in the .h.
	// device_reset() owns reset behavior.  This block gives every member a
	// known power-up/startup value before save-state registration and reset.
	// --------------------------------------------------

	m_hp90_prev_in = 0.0;
	m_hp90_prev_out = 0.0;

	m_hp440_prev_in = 0.0;
	m_hp440_prev_out = 0.0;

	m_lp14k_prev_out = 0.0;

	m_output_accum = 0.0;

	for (u32 i = 0; i < OUT_FIFO_SIZE; i++)
		m_out_fifo[i] = 0.0;

	m_out_fifo_r = 0;
	m_out_fifo_w = 0;
	m_last_out_sample = 0.0;

	m_resample_phase = 0;
	m_resample_step = 0;

	m_cached_output = 0.0;

	cpu_cycle = 0;
	apu_clk1_is_high = false;
	cpu_reading = false;
	run_ppu = false;

	m_dmc_cpu_bus_latch = 0;

	temp_halt_len_loop_env_0 = false;
	temp_halt_len_loop_env_1 = false;
	temp_tri_halt_flag = false;
	temp_noise_halt_len_loop_env = false;

	delay_halt_len_loop_0 = 0;
	delay_halt_len_loop_1 = 0;
	delay_tri_halt_flag = 0;
	delay_noise_halt_len_loop_env = 0;

	temp_len_cnt_0 = 0;
	temp_len_cnt_0_reload_value = 0;
	temp_len_cnt_1 = 0;
	temp_len_cnt_1_reload_value = 0;
	temp_tri_len_cnt = 0;
	temp_tri_len_cnt_reload_value = 0;
	temp_noise_len_cnt = 0;
	temp_noise_len_cnt_reload_value = 0;

	delay_len_cnt_0 = 0;
	delay_len_cnt_1 = 0;
	delay_tri_len_cnt = 0;
	delay_noise_len_cnt = 0;

	m_dma_engine.dmc.load_request = false;

	m_dma_engine.dmc.late_implicit_short = false;

	dmc_cycles_until_buffer_empty = 0;
	dmc_cycles_since_buffer_empty = -1;

	dmc_4015_load_defer_pending = false;
	dmc_4015_load_defer_delay = 0;
	
	//dmc_4011_write_pending = false;
	//dmc_4011_old_counter = 0;

	detect_abort_1 = 0;
	detect_abort_2_3 = 0;
	detect_abort_0 = 0;
	detect_abort_8_9 = 0;
	detect_test_l = 0;
	detect_test_m = 0;

	frame_irq = false;
	frame_irq_output = false;
	dmc_irq = false;
	inhibit_frame_irq = false;

	frame_counter_mode = FOUR_STEP;
	new_frame_counter_mode = FOUR_STEP;

	delayed_frame_timer_reset = 0;
	frame_counter_clock = 0;
	frame_unit_clock_block_until = 0;
	last_frame_unit_pulse_apu_cycle = ~uint64_t(0);

	delayed_frame_irq = 0;
	delayed_frame_irq_after_dmc = false;
	delayed_frame_irq_clear = 0;
	delayed_dmc_irq = 0;

	//frame_irq_no_clear_before = 0;
	frame_irq_suppress_clear_cycle = 0;

	m_save_frame_counter_mode = 0;
	m_save_new_frame_counter_mode = 0;
	
	m_save_dmc_dma_mode = 0;
	m_save_dmc_dma_phase = 0;
	m_save_oam_dma_phase = 0;

	dmc_counter = 0;

	dmc_loading_sample_byte = false;
	dmc_sample_buffer_has_data = false;
	dpcm_active = false;

	dmc_irq_enabled = false;
	dmc_loop_sample = false;

	dmc_sample_start_addr = 0xC000;
	dmc_sample_len = 1;

	dmc_sample_buffer = 0;
	dmc_shift_reg = 0xFF;
	dmc_sample_cur_addr = 0x0000; //0xC000;
	dmc_bytes_remaining = 0;
	dmc_bits_remaining = 8;

	dmc_periods = nullptr;
	noise_periods = nullptr;

	calculate_rates();
	
	tri_output_level = 0;
	tri_enabled = false;

	tri_period = 0;
	tri_period_cnt = 1;

	tri_waveform_pos = 0;
	tri_len_cnt = 0;

	tri_halt_flag = false;
	tri_lin_cnt_load = 0;
	tri_lin_cnt = 0;
	tri_lin_cnt_reload_flag = false;

	noise_output_level = 0;
	noise_enabled = false;
	noise_halt_len_loop_env = false;
	noise_const_vol = false;

	noise_vol = 0;
	noise_feedback_bit = 1;
	noise_period = 0;
	noise_period_cnt = 0;
	noise_len_cnt = 0;

	noise_shift_reg = 1;
	noise_env_start_flag = false;
	noise_env_vol = 0;
	noise_env_div_cnt = 0;

	dma_engine_reset();

	// --------------------------------------------------
	// Power-up/default register latch values that reset preserves.
	// --------------------------------------------------

	dmc_period = dmc_periods[0];
	dmc_period_cnt = dmc_period;

	noise_period = noise_periods[0];
	noise_period_cnt = noise_period + 1;

	// --------------------------------------------------
	// Mixer lookup tables.
	// --------------------------------------------------

	for (int i = 0; i < 31; i++)
	{
		sound_stream::sample_t pulse_out = (i == 0) ? 0.0 : 95.88 / ((8128.0 / i) + 100.0);
		m_square_lut[i] = pulse_out;
	}

	for (int t = 0; t < 16; t++)
	{
		for (int n = 0; n < 16; n++)
		{
			for (int d = 0; d < 128; d++)
			{
				sound_stream::sample_t tnd_out = (t / 8227.0) + (n / 12241.0) + (d / 22638.0);
				tnd_out = (tnd_out == 0.0) ? 0.0 : 159.79 / ((1.0 / tnd_out) + 100.0);
				m_tnd_lut[t][n][d] = tnd_out;
			}
		}
	}

	// --------------------------------------------------
	// Power-up register/latch defaults.
	//
	// These are one-time power-up defaults.  device_reset() intentionally
	// does not wipe many of these because NES reset leaves several APU
	// register latches unchanged.
	// --------------------------------------------------

	for (int n = 0; n < 2; ++n)
	{
		m_APU.pulse[n].enabled = false;
		m_APU.pulse[n].waveform_pos = 0;
		m_APU.pulse[n].len_cnt = 0;
		m_APU.pulse[n].period_cnt = 1;
		m_APU.pulse[n].sweep_period_cnt = 1;
		m_APU.pulse[n].env_div_cnt = 0;
		m_APU.pulse[n].env_vol = 0;

		m_APU.pulse[n].const_vol = false;
		m_APU.pulse[n].duty = 0;
		m_APU.pulse[n].period = 0;
		m_APU.pulse[n].sweep_enabled = false;
		m_APU.pulse[n].sweep_negate = false;
		m_APU.pulse[n].sweep_period = 0;
		m_APU.pulse[n].sweep_shift = 0;
		m_APU.pulse[n].sweep_reload_flag = false;
		m_APU.pulse[n].vol = 0;

		m_APU.pulse[n].halt_len_loop_env = false;
		m_APU.pulse[n].env_start_flag = false;

		m_APU.pulse[n].sweep_target_period = 0;
		m_APU.pulse[n].output_level = 0;
	}

	tri_enabled = false;
	tri_period_cnt = 1;
	tri_waveform_pos = 0;
	tri_len_cnt = 0;
	tri_lin_cnt = 0;
	tri_period = 0;
	tri_halt_flag = false;
	tri_lin_cnt_load = 0;
	tri_lin_cnt_reload_flag = false;
	tri_output_level = tri_waveform_steps[tri_waveform_pos];

	temp_tri_halt_flag = false;
	delay_tri_halt_flag = 0;

	temp_tri_len_cnt = 0;
	temp_tri_len_cnt_reload_value = 0;
	delay_tri_len_cnt = 0;

	noise_enabled = false;
	noise_halt_len_loop_env = false;
	noise_const_vol = false;
	noise_vol = 0;
	noise_feedback_bit = 1;
	noise_len_cnt = 0;
	noise_shift_reg = 1;
	noise_env_start_flag = false;
	noise_env_vol = 0;
	noise_env_div_cnt = 0;
	noise_output_level = 0;

	dmc_counter = 0;
	dmc_irq_enabled = false;
	dmc_loop_sample = false;
	dmc_sample_start_addr = 0xC000;
	dmc_sample_len = 1;
	dmc_sample_buffer = 0;
	dmc_loading_sample_byte = false;

	run_ppu = false;

	inhibit_frame_irq = false;

	frame_irq = false;
	frame_irq_output = false;
	//frame_irq_no_clear_before = 0;

	delayed_frame_irq = 0;
	delayed_frame_irq_after_dmc = false;

	delayed_frame_irq_clear = 0;
	delayed_dmc_irq = 0;

	frame_irq_suppress_clear_cycle = 0;
	frame_unit_clock_block_until = 0;
	last_frame_unit_pulse_apu_cycle = ~uint64_t(0);

	m_dmc_cpu_bus_latch = 0;

	// --------------------------------------------------
	// Audio stream.
	// --------------------------------------------------

	int rate = 48000;

	logerror("Audio Rate: %d\n", rate);

	if (m_stream != nullptr)
	{
		m_stream->set_sample_rate(rate);
	}
	else
	{
		m_stream = stream_alloc(0, 1, rate);
	}
	calculate_rates();
	m_apu_timer = timer_alloc(FUNC(nesapu_device::apu_tick), this);
	m_apu_timer->adjust(attotime::zero);

	// --------------------------------------------------
	// Save-state registration.
	// --------------------------------------------------

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_APU.pulse[i].enabled), i);
		save_item(NAME(m_APU.pulse[i].waveform_pos), i);
		save_item(NAME(m_APU.pulse[i].len_cnt), i);
		save_item(NAME(m_APU.pulse[i].period_cnt), i);
		save_item(NAME(m_APU.pulse[i].sweep_period_cnt), i);
		save_item(NAME(m_APU.pulse[i].env_div_cnt), i);
		save_item(NAME(m_APU.pulse[i].env_vol), i);

		save_item(NAME(m_APU.pulse[i].const_vol), i);
		save_item(NAME(m_APU.pulse[i].duty), i);
		save_item(NAME(m_APU.pulse[i].period), i);
		save_item(NAME(m_APU.pulse[i].sweep_enabled), i);
		save_item(NAME(m_APU.pulse[i].sweep_negate), i);
		save_item(NAME(m_APU.pulse[i].sweep_period), i);
		save_item(NAME(m_APU.pulse[i].sweep_shift), i);
		save_item(NAME(m_APU.pulse[i].sweep_reload_flag), i);
		save_item(NAME(m_APU.pulse[i].vol), i);
		save_item(NAME(m_APU.pulse[i].halt_len_loop_env), i);
		save_item(NAME(m_APU.pulse[i].env_start_flag), i);

		save_item(NAME(m_APU.pulse[i].sweep_target_period), i);
		save_item(NAME(m_APU.pulse[i].output_level), i);
	}

	save_item(NAME(m_hp90_prev_in));
	save_item(NAME(m_hp90_prev_out));

	save_item(NAME(m_hp440_prev_in));
	save_item(NAME(m_hp440_prev_out));

	save_item(NAME(m_lp14k_prev_out));

	save_item(NAME(m_output_accum));

	save_item(NAME(m_out_fifo));
	save_item(NAME(m_out_fifo_r));
	save_item(NAME(m_out_fifo_w));
	save_item(NAME(m_last_out_sample));

	save_item(NAME(m_resample_phase));
	save_item(NAME(m_resample_step));
	
	save_item(NAME(run_ppu));
	save_item(NAME(m_cached_output));

	save_item(NAME(tri_enabled));
	save_item(NAME(tri_period_cnt));
	save_item(NAME(tri_waveform_pos));
	save_item(NAME(tri_len_cnt));
	save_item(NAME(tri_lin_cnt));
	save_item(NAME(tri_period));
	save_item(NAME(tri_halt_flag));
	save_item(NAME(tri_lin_cnt_load));
	save_item(NAME(tri_lin_cnt_reload_flag));
	save_item(NAME(temp_tri_halt_flag));
	save_item(NAME(delay_tri_halt_flag));
	save_item(NAME(temp_tri_len_cnt));
	save_item(NAME(temp_tri_len_cnt_reload_value));
	save_item(NAME(delay_tri_len_cnt));
	save_item(NAME(tri_output_level));

	save_item(NAME(noise_enabled));
	save_item(NAME(noise_period));
	save_item(NAME(noise_period_cnt));
	save_item(NAME(noise_len_cnt));
	save_item(NAME(noise_shift_reg));
	save_item(NAME(noise_env_vol));
	save_item(NAME(noise_env_div_cnt));
	save_item(NAME(noise_halt_len_loop_env));
	save_item(NAME(noise_const_vol));
	save_item(NAME(noise_vol));
	save_item(NAME(noise_feedback_bit));
	save_item(NAME(noise_env_start_flag));
	save_item(NAME(temp_noise_halt_len_loop_env));
	save_item(NAME(delay_noise_halt_len_loop_env));
	save_item(NAME(temp_noise_len_cnt));
	save_item(NAME(temp_noise_len_cnt_reload_value));
	save_item(NAME(delay_noise_len_cnt));
	save_item(NAME(noise_output_level));

	save_item(NAME(dmc_period_cnt));
	save_item(NAME(dmc_period));
	save_item(NAME(dmc_sample_cur_addr));
	save_item(NAME(dmc_bytes_remaining));
	save_item(NAME(dmc_sample_buffer_has_data));
	save_item(NAME(dmc_bits_remaining));
	save_item(NAME(dmc_shift_reg));
	save_item(NAME(dpcm_active));
	save_item(NAME(dmc_counter));
	save_item(NAME(dmc_irq_enabled));
	save_item(NAME(dmc_loop_sample));
	save_item(NAME(dmc_sample_start_addr));
	save_item(NAME(dmc_sample_len));
	save_item(NAME(dmc_sample_buffer));
	save_item(NAME(dmc_loading_sample_byte));
	save_item(NAME(dmc_cycles_until_buffer_empty));
	save_item(NAME(dmc_cycles_since_buffer_empty));
	save_item(NAME(dmc_4015_load_defer_pending));
	save_item(NAME(dmc_4015_load_defer_delay));
	//save_item(NAME(dmc_4011_write_pending));
	//save_item(NAME(dmc_4011_old_counter));
	save_item(NAME(m_dmc_cpu_bus_latch));

	save_item(NAME(delayed_frame_timer_reset));
	save_item(NAME(frame_counter_clock));
	save_item(NAME(frame_unit_clock_block_until));
	save_item(NAME(last_frame_unit_pulse_apu_cycle));
	save_item(NAME(frame_irq));
	save_item(NAME(dmc_irq));
	save_item(NAME(frame_irq_output));
	save_item(NAME(delayed_frame_irq));
	save_item(NAME(delayed_frame_irq_after_dmc));
	save_item(NAME(delayed_frame_irq_clear));
	save_item(NAME(delayed_dmc_irq));
	save_item(NAME(frame_irq_suppress_clear_cycle));
	save_item(NAME(m_save_frame_counter_mode));
	save_item(NAME(m_save_new_frame_counter_mode));
	save_item(NAME(inhibit_frame_irq));

	save_item(NAME(cpu_cycle));
	save_item(NAME(apu_clk1_is_high));
	save_item(NAME(cpu_reading));
	
	// --------------------------------------------------
	// DMA engine save-state registration.
	// --------------------------------------------------

	save_item(NAME(m_dma_engine.data_bus));

	save_item(NAME(m_dma_engine.dmc.active));
	save_item(NAME(m_save_dmc_dma_mode));
	save_item(NAME(m_save_dmc_dma_phase));
	save_item(NAME(m_dma_engine.dmc.load_request));
	save_item(NAME(m_dma_engine.dmc.replay_pending));
	save_item(NAME(m_dma_engine.dmc.clear_implicit_after_read));
	save_item(NAME(m_dma_engine.dmc.late_implicit_short));
	save_item(NAME(m_dma_engine.dmc.implicit_abort_halt_reads_required));
	save_item(NAME(m_dma_engine.dmc.pending_delay));
	save_item(NAME(m_dma_engine.dmc.halt_read_cycles));
	save_item(NAME(m_dma_engine.dmc.halt_write_cycles));

	save_item(NAME(m_dma_engine.oam.active));
	save_item(NAME(m_save_oam_dma_phase));
	save_item(NAME(m_dma_engine.oam.base_addr));
	save_item(NAME(m_dma_engine.oam.index));

	save_item(NAME(temp_halt_len_loop_env_0));
	save_item(NAME(temp_halt_len_loop_env_1));
	save_item(NAME(delay_halt_len_loop_0));
	save_item(NAME(delay_halt_len_loop_1));
	save_item(NAME(temp_len_cnt_0_reload_value));
	save_item(NAME(temp_len_cnt_0));
	save_item(NAME(delay_len_cnt_0));
	save_item(NAME(temp_len_cnt_1_reload_value));
	save_item(NAME(temp_len_cnt_1));
	save_item(NAME(delay_len_cnt_1));

	save_item(NAME(detect_abort_1));
	save_item(NAME(detect_abort_2_3));
	save_item(NAME(detect_abort_0));
	save_item(NAME(detect_test_l));
	save_item(NAME(detect_test_m));
	save_item(NAME(detect_abort_8_9));

	// --------------------------------------------------
	// Final startup values.
	// --------------------------------------------------

	frame_counter_mode = FOUR_STEP;
	new_frame_counter_mode = FOUR_STEP;

	tri_waveform_pos = 0;
	tri_output_level = tri_waveform_steps[tri_waveform_pos];

	machine().save().register_presave(save_prepost_delegate(FUNC(nesapu_device::presave), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(nesapu_device::postload), this));

	m_output_accum = 0.0;
	m_resample_phase = 0;
	calculate_rates();
	m_output_accum = 0.0;

	m_cached_output = 0.0;
}

void nesapu_device::device_reset()
{
	// --------------------------------------------------
	// Reset any CPU/APU DMA handshake and restart the APU tick timer.
	//
	// Cold boot arms m_apu_timer in device_start().  A soft reset does not
	// run device_start(), so the APU timer can keep its old scheduled phase.
	// DMC DMA + OAM DMA and explicit abort timing are phase-sensitive, so
	// restart the APU timer here just like power-up.
	// --------------------------------------------------
	if (m_maincpu6502)
	{
		m_maincpu6502->dmc_clear_halt();
		m_maincpu6502->oam_clear_halt();
	}

	if (m_maincpu_dev && m_maincpu_dev->suspended())
		m_maincpu_dev->resume(SUSPEND_REASON_HALT);

	if (m_apu_timer)
		m_apu_timer->adjust(attotime::zero);
	// --------------------------------------------------
	// CPU/APU phase and DMA runtime state
	// --------------------------------------------------
	//
	// RESET does not preserve an in-flight emulator DMA/timing transaction.
	// These are emulator/runtime bookkeeping fields, not APU register latches.
	pal_cpu_ppu = 0;
	cpu_cycle = 0;
	apu_clk1_is_high = false;
	cpu_reading = false;
	m_dmc_cpu_bus_latch = 0;

	run_ppu = false;

	// --------------------------------------------------
	// Audio/output runtime state
	// --------------------------------------------------

	m_hp90_prev_in = 0.0;
	m_hp90_prev_out = 0.0;

	m_hp440_prev_in = 0.0;
	m_hp440_prev_out = 0.0;

	m_lp14k_prev_out = 0.0;

	m_output_accum = 0.0;

	m_out_fifo_r = 0;
	m_out_fifo_w = 0;
	m_last_out_sample = 0.0;

	m_resample_phase = 0;

	m_cached_output = 0.0;

	// --------------------------------------------------
	// Debug/stat counters
	// --------------------------------------------------

	detect_abort_1 = 0;
	detect_abort_2_3 = 0;
	detect_abort_0 = 0;
	detect_test_l = 0;
	detect_test_m = 0;
	detect_abort_8_9 = 0;

	// --------------------------------------------------
	// Pulse channels / $4015 disable behavior.
	//
	// Keep pulse register latches alone:
	//   duty / period / sweep / volume / halt_len_loop_env
	//
	// Clear runtime enable/counter/output state so the channel cannot keep
	// playing across reset.
	// --------------------------------------------------

	for (unsigned n = 0; n < 2; ++n)
	{
		m_APU.pulse[n].enabled = false;
		m_APU.pulse[n].waveform_pos = 0;
		m_APU.pulse[n].len_cnt = 0;
		m_APU.pulse[n].period_cnt = 1;
		m_APU.pulse[n].sweep_period_cnt = 1;
		m_APU.pulse[n].env_div_cnt = 0;
		m_APU.pulse[n].env_vol = 0;

		// Pending internal actions must not survive reset.
		m_APU.pulse[n].sweep_reload_flag = false;
		m_APU.pulse[n].env_start_flag = false;
	}

	// --------------------------------------------------
	// Triangle channel runtime reset.
	//
	// Keep register latches alone:
	//   tri_period / tri_halt_flag / tri_lin_cnt_load
	// --------------------------------------------------

	tri_enabled = false;
	tri_period_cnt = 1;
	tri_waveform_pos = 0;
	tri_len_cnt = 0;
	tri_lin_cnt = 0;
	tri_lin_cnt_reload_flag = false;

	tri_output_level = tri_waveform_steps[tri_waveform_pos];

	temp_tri_halt_flag = false;
	delay_tri_halt_flag = 0;

	temp_tri_len_cnt = 0;
	temp_tri_len_cnt_reload_value = 0;
	delay_tri_len_cnt = 0;

	// --------------------------------------------------
	// Noise channel runtime reset.
	//
	// Keep register latches alone:
	//   noise_halt_len_loop_env / noise_const_vol / noise_vol
	//   noise_feedback_bit / noise_period
	//
	// Restart the period counter from the current selected latch value.
	// --------------------------------------------------

	noise_enabled = false;
	noise_period_cnt = noise_period + 1;
	noise_len_cnt = 0;

	noise_env_start_flag = false;
	noise_env_vol = 0;
	noise_env_div_cnt = 0;

	temp_noise_halt_len_loop_env = false;
	delay_noise_halt_len_loop_env = 0;

	temp_noise_len_cnt = 0;
	temp_noise_len_cnt_reload_value = 0;
	delay_noise_len_cnt = 0;

	// --------------------------------------------------
	// DMC reset behavior.
	//
	// Reset clears active DMC playback/DMA state, but it does not wipe the
	// $4010/$4012/$4013 register latches. device_start() gives those latches
	// deterministic power-up defaults; reset only stops the active reader and
	// output unit so no stale DMA transaction survives into the next test.
	// --------------------------------------------------

	dmc_period_cnt = dmc_period;

	dmc_sample_cur_addr = 0x0000;//dmc_sample_start_addr;
	dmc_bytes_remaining = 0;
	dmc_sample_buffer_has_data = false;
	dmc_bits_remaining = 8;

	dmc_shift_reg = 0xFF;
	dpcm_active = false;
	dmc_sample_buffer = 0;
	dmc_loading_sample_byte = false;

	m_dma_engine.dmc.load_request = false;

	m_dma_engine.dmc.late_implicit_short = false;

	dmc_cycles_until_buffer_empty = 0;
	dmc_cycles_since_buffer_empty = -1;

	dmc_4015_load_defer_pending = false;
	dmc_4015_load_defer_delay = 0;
	
	//dmc_4011_write_pending = false;
	//dmc_4011_old_counter = 0;

	// --------------------------------------------------
	// Frame counter / IRQ reset behavior.
	//
	// Keep this deterministic for the test ROM reset flow.  Do not let a
	// previous $4017 mode/inhibit write carry into the next DMA test run.
	// --------------------------------------------------

	delayed_frame_timer_reset = 0;
	frame_counter_clock = 0;
	frame_unit_clock_block_until = 0;
	last_frame_unit_pulse_apu_cycle = ~uint64_t(0);

	//frame_counter_mode = FOUR_STEP;
	//new_frame_counter_mode = FOUR_STEP;
	//inhibit_frame_irq = false;

	frame_irq = false;
	frame_irq_output = false;
	dmc_irq = false;

	delayed_frame_irq = 0;
	delayed_frame_irq_after_dmc = false;
	delayed_frame_irq_clear = 0;
	delayed_dmc_irq = 0;

	//frame_irq_no_clear_before = 0;
	frame_irq_suppress_clear_cycle = 0;

	// --------------------------------------------------
	// Delayed register effects:
	// 10.len_halt_timing
	// --------------------------------------------------

	temp_halt_len_loop_env_0 = false;
	temp_halt_len_loop_env_1 = false;

	delay_halt_len_loop_0 = 0;
	delay_halt_len_loop_1 = 0;

	temp_noise_halt_len_loop_env = false;
	delay_noise_halt_len_loop_env = 0;

	// --------------------------------------------------
	// Delayed register effects:
	// 11.len_reload_timing
	// --------------------------------------------------

	temp_len_cnt_0_reload_value = 0;
	temp_len_cnt_0 = 0;
	delay_len_cnt_0 = 0;

	temp_len_cnt_1_reload_value = 0;
	temp_len_cnt_1 = 0;
	delay_len_cnt_1 = 0;

	temp_noise_len_cnt = 0;
	temp_noise_len_cnt_reload_value = 0;
	delay_noise_len_cnt = 0;

	temp_tri_len_cnt = 0;
	temp_tri_len_cnt_reload_value = 0;
	delay_tri_len_cnt = 0;
	// --------------------------------------------------
	// Recompute derived output and IRQ state
	// --------------------------------------------------

	for (unsigned n = 0; n < 2; ++n)
	{
		update_sweep_target_period(n);
		update_pulse_output_level(n);
	}

	update_noise_output_level();

	tri_waveform_pos &= 0x1f;
	tri_output_level = tri_waveform_steps[tri_waveform_pos];

	update_irq_output();

	m_cached_output = 0.0;
	
	dma_engine_reset();
}

void nesapu_device::postload()
{
	frame_counter_mode = Frame_counter_mode(m_save_frame_counter_mode);
	new_frame_counter_mode = Frame_counter_mode(m_save_new_frame_counter_mode);

	m_dma_engine.dmc.mode = dmc_dma_mode(m_save_dmc_dma_mode);
	m_dma_engine.dmc.phase = dmc_dma_phase(m_save_dmc_dma_phase);
	m_dma_engine.oam.phase = oam_dma_phase(m_save_oam_dma_phase);

	tri_waveform_pos &= 0x1f;

	for (int n = 0; n < 2; n++) {
		update_sweep_target_period(n);
		update_pulse_output_level(n);
	}

	update_noise_output_level();
	tri_output_level = tri_waveform_steps[tri_waveform_pos];
	update_irq_output();
}

void nesapu_device::presave()
{
	m_save_frame_counter_mode = int(frame_counter_mode);
	m_save_new_frame_counter_mode = int(new_frame_counter_mode);

	m_save_dmc_dma_mode = int(m_dma_engine.dmc.mode);
	m_save_dmc_dma_phase = int(m_dma_engine.dmc.phase);
	m_save_oam_dma_phase = int(m_dma_engine.oam.phase);
}

void nesapu_device::tick() {	
	//Start 10.len_halt_timing is delayed by 1 cycle
	//     current tick: 2 -> 1, no commit
	//     next tick:    1 -> 0, commit
	if (delay_halt_len_loop_0 > 0 && --delay_halt_len_loop_0 == 0) {
		m_APU.pulse[0].halt_len_loop_env = temp_halt_len_loop_env_0;
	}

	if (delay_halt_len_loop_1 > 0 && --delay_halt_len_loop_1 == 0) {
		m_APU.pulse[1].halt_len_loop_env = temp_halt_len_loop_env_1;
	}
	
	if (delay_tri_halt_flag > 0 && --delay_tri_halt_flag == 0) {
		tri_halt_flag = temp_tri_halt_flag;
	}
	
	if (delay_noise_halt_len_loop_env > 0 && --delay_noise_halt_len_loop_env == 0) {
		noise_halt_len_loop_env = temp_noise_halt_len_loop_env;
	}
	//End 10.len_halt_timing is delayed by 1 cycle
	
	//Start 11.len_reload_timing is delayed by 1 cycle
	if (delay_len_cnt_0 > 0 && --delay_len_cnt_0 == 0) {
		if (temp_len_cnt_0_reload_value) {
			if (m_APU.pulse[0].len_cnt == temp_len_cnt_0) {
				m_APU.pulse[0].len_cnt = temp_len_cnt_0_reload_value;
			}
			temp_len_cnt_0_reload_value = 0;
		}

		update_sweep_target_period(0);
		update_pulse_output_level(0);
	}

	if (delay_len_cnt_1 > 0 && --delay_len_cnt_1 == 0) {
		if (temp_len_cnt_1_reload_value) {
			if (m_APU.pulse[1].len_cnt == temp_len_cnt_1) {
				m_APU.pulse[1].len_cnt = temp_len_cnt_1_reload_value;
			}
			temp_len_cnt_1_reload_value = 0;
		}

		update_sweep_target_period(1);
		update_pulse_output_level(1);
	}

	if (delay_tri_len_cnt > 0 && --delay_tri_len_cnt == 0) {
		if (temp_tri_len_cnt_reload_value) {
			if (tri_len_cnt == temp_tri_len_cnt) {
				tri_len_cnt = temp_tri_len_cnt_reload_value;
			}
			temp_tri_len_cnt_reload_value = 0;
		}
	}

	if (delay_noise_len_cnt > 0 && --delay_noise_len_cnt == 0) {
		if (temp_noise_len_cnt_reload_value) {
			if (noise_len_cnt == temp_noise_len_cnt) {
				noise_len_cnt = temp_noise_len_cnt_reload_value;
			}
			temp_noise_len_cnt_reload_value = 0;
		}

		update_noise_output_level();
	}
	//End 11.len_reload_timing is delayed by 1 cycle
	
	//clock here before irq delay or fails IRQ timing
	clock_frame_counter();
	
	// IRQ in APU is delayed before sending to CPU.
	//
	// When it expires, publish the separate CPU IRQ-output state.
	if (delayed_frame_irq > 0 && --delayed_frame_irq == 0) {
		frame_irq_output = true;
		update_irq_output();
	}

	// Frame IRQ clear is delayed to preserve the put/get vs get/put timing.
	//
	// Clear both the $4015-visible flag and the CPU IRQ output here, but
	// do not clear frame_irq immediately in set_frame_irq(false), because $4015
	// can still observe the old flag for one edge in the timing tests.
	if (delayed_frame_irq_clear > 0 && --delayed_frame_irq_clear == 0) {
		frame_irq = false;
		frame_irq_output = false;

		// Cancel any pending frame IRQ output publication.
		delayed_frame_irq = 0;
		delayed_frame_irq_after_dmc = false;

		update_irq_output();
	}

	// DMC IRQ output update delay.
	//
	// Important ordering:
	// The DMC read path intentionally services tick() before dmc_read():
	//
	//   tick();
	//   dma_engine_finish_dmc_read();
	//
	// This means a DMC IRQ raised inside dmc_read() arms delayed_dmc_irq after
	// this tick's delay-service point has already run. Keep set_dmc_irq(true)
	// at delayed_dmc_irq = 2; do not compensate here.
	if (delayed_dmc_irq > 0 && --delayed_dmc_irq == 0) {
		update_irq_output();
	}
	
	if (frame_irq_suppress_clear_cycle > 0 && --frame_irq_suppress_clear_cycle == 0) {
		frame_irq = false;
		frame_irq_output = false;
		// Cancel any pending frame IRQ output publication.
		delayed_frame_irq = 0;
		delayed_frame_irq_after_dmc = false;

		delayed_frame_irq_clear = 0;
		update_irq_output();
	}
	
	// Pulse timers tick on every other CPU/APU cycle.
	if (!apu_clk1_is_high)
	{
		if (--m_APU.pulse[0].period_cnt == 0)
		{
			m_APU.pulse[0].period_cnt = m_APU.pulse[0].period + 1;
			clock_pulse_generator(0);
		}

		if (--m_APU.pulse[1].period_cnt == 0)
		{
			m_APU.pulse[1].period_cnt = m_APU.pulse[1].period + 1;
			clock_pulse_generator(1);
		}
	}

	// Triangle timer ticks every CPU/APU cycle.
	if (--tri_period_cnt == 0)
	{
		tri_period_cnt = tri_period + 1;
		clock_triangle_generator();
	}

	// Noise timer.
	if (--noise_period_cnt == 0)
	{
		noise_period_cnt = noise_period + 1;
		clock_noise_generator();
	}

	// DMC timer.
	// DMC period table values are already the full countdown interval.
	if (--dmc_period_cnt == 0)
	{
		dmc_period_cnt = dmc_period;
		clock_dmc();
	}

	// Mixing
	// Latch exactly one mixed output sample for this APU tick, then let the
	// sound stream consume latched history instead of re-synthesizing "now".
	accumulate_output_sample(calc_current_output());
	
		
	// Clock the PPU from the APU/CPU tick stream.
	//
	// Boot edge case:
	// Avoid ticking the PPU
	// before the CPU/APU cycle relationship is valid at startup. Keep that
	// behavior as a one-time latch instead of rechecking cpu_cycle math forever.
	if (!run_ppu)
	{
		if (cpu_cycle >= 0)
			run_ppu = true;
	}
	else
	{
		m_ppu_dev->tick(1);
		m_ppu_dev->tick(2);
		m_ppu_dev->tick(3);

		if (m_is_pal && ++pal_cpu_ppu == 5)
		{
			m_ppu_dev->tick(4);
			pal_cpu_ppu = 0;
		}
	}
}

TIMER_CALLBACK_MEMBER(nesapu_device::apu_tick) {
	m_apu_timer->adjust(clocks_to_attotime(1));
	tick_apu();
}

void nesapu_device::tick_apu()
{
	// We run this callback after the CPU has already executed its current cycle.
	// To line APU timing up with the CPU cycle that just happened, use total_cycles()-1.
	cpu_cycle        = m_maincpu_dev->total_cycles() - 1;
	apu_clk1_is_high = ((cpu_cycle & 0x01) == 0);
	cpu_reading      = m_maincpu6502->get_cpu_is_reading();
	//IR               = m_maincpu6502->get_IR();

	// MMC5 has extra per-CPU-cycle behavior that must be serviced before the
	// normal APU/DMC/OAM DMA handling below. After that, run the shared APU tick
	// and stop here.
	//if (m_mmc5) {
		//m_mmc5->mmc5_cpu_cycle();
	//}

	// Track the interval until the DMC sample buffer becomes empty, and then
	// how many CPU cycles have elapsed since it became empty.
	//
	// These counters are used to detect the special timing windows exercised by
	// the DMC tests ("Test L / M" and implicit/explicit abort timing cases).
	if (dmc_cycles_until_buffer_empty > 0) {
		dmc_cycles_until_buffer_empty--;
		if (dmc_cycles_until_buffer_empty == 0) {
			dmc_cycles_since_buffer_empty = 0;
		}
	}
	else if (dmc_cycles_since_buffer_empty >= 0) {
		dmc_cycles_since_buffer_empty++;
	}

	// DMC Test L / M delayed execution:
	// Some test cases intentionally arm a DMC reload one CPU cycle later.
	// Count that delay down here and fire the sample-byte load when it expires.
	if (dmc_4015_load_defer_delay > 0) {
		--dmc_4015_load_defer_delay;
		if (dmc_4015_load_defer_delay == 0 && dmc_4015_load_defer_pending) {
			dmc_4015_load_defer_pending = false;
			dmc_4015_load_defer_delay = 0;
			m_dma_engine.dmc.load_request = true;
			load_dmc_sample_byte();
		}
	}

	if (dma_engine_run()) {
		return;
	}

	// Normal operation:
	// No DMA is active, so just advance the system by one tick.
	// (CPU/APU/PPU run normally unless some kind of DMA is running
	tick();
}

bool nesapu_device::dma_engine_run()
{
	const bool cpu_suspended = m_maincpu_dev->suspended();

	// --------------------------------------------------
	// Pending DMC request delay.
	// --------------------------------------------------
	if (m_dma_engine.dmc.active &&
		m_dma_engine.dmc.pending_delay > 0)
	{
		--m_dma_engine.dmc.pending_delay;

		if (m_dma_engine.dmc.pending_delay > 0)
		{
			tick();
			return true;
		}

		m_dma_engine.dmc.phase = dmc_dma_phase::waiting_for_halt;
	}

	// --------------------------------------------------
	// Count DMC halt-attempt wait cycles.
	//
	// NESdev rule:
	// DMA can only halt the CPU on read cycles. On write cycles, the halt
	// fails and the DMA unit tries again on the next CPU cycle.
	// --------------------------------------------------
	if (dma_engine_dmc_waiting_for_halt() && !cpu_suspended)
	{
		if (cpu_reading)
			++m_dma_engine.dmc.halt_read_cycles;
		else
			++m_dma_engine.dmc.halt_write_cycles;
	}

	// --------------------------------------------------
	// Implicit 8/9 write-cycle cancellation.
	//
	// NESdev bug rule:
	// When playback is stopped 2 or 3 CPU cycles before the reload halt
	// attempt, the DMA starts but is aborted after one cycle. If that halt is
	// delayed by a write cycle, the aborted DMA does not occur at all.
	//
	// This applies only to the second halt window. That second window is
	// armed by dma_engine_finish_dmc_read() after the first real DMC read:
	//
	//   implicit_abort_halt_reads_required = 2
	// --------------------------------------------------
	if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_abort_8_9 &&
		m_dma_engine.dmc.implicit_abort_halt_reads_required > 0 &&
		m_dma_engine.dmc.halt_write_cycles > 0)
	{
		if (delayed_frame_irq_after_dmc)
		{
			delayed_frame_irq_after_dmc = false;
			//delayed_frame_irq = 2;
			delayed_frame_irq = apu_clk1_is_high ? 2 : 1;
		}

		m_dma_engine.dmc.load_request = false;
		m_dma_engine.dmc.halt_read_cycles = 0;
		m_dma_engine.dmc.halt_write_cycles = 0;
		m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
		m_dma_engine.dmc.late_implicit_short = false;
		m_dma_engine.dmc.replay_pending = false;
		m_dma_engine.dmc.clear_implicit_after_read = false;
		m_dma_engine.dmc.pending_delay = 0;

		dmc_loading_sample_byte = false;

		m_maincpu6502->dmc_clear_halt();

		dma_engine_mark_dmc_idle();
		dma_engine_resume_cpu_if_done();

		tick();
		return true;
	}

	// --------------------------------------------------
	// Arm DMC halt requests while the CPU is still running.
	// --------------------------------------------------
	if (dma_engine_dmc_waiting_for_halt() && !cpu_suspended)
	{
		if (m_dma_engine.dmc.load_request &&
			cpu_reading &&
			m_dma_engine.dmc.halt_read_cycles >= 2 &&
			!apu_clk1_is_high)
		{
			// Normal DMC load DMA.
			//
			// NESdev:
			// The first load DMA after $4015 attempts to halt on the get cycle
			// during the 2nd following APU cycle.
			m_maincpu6502->dmc_halt_next_read(false, false);
		}
		else if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_0 ||
				 m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_1 ||
				 m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_2_or_3)
		{
			// Explicit stop bug.
			//
			// The halt request is an aborted-DMA halt. The individual explicit
			// cases are resolved after halt acknowledgement.
			if (m_dma_engine.dmc.halt_read_cycles >= 1)
				m_maincpu6502->dmc_halt_next_read(true, true);
		}
		else if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_abort_8_9)
		{
			// Implicit 8/9 has two windows:
			//
			//   delay == 0:
			//     first real DMC DMA read. It uses the normal dummy/read path.
			//
			//   delay == 2:
			//     second one-cycle aborted DMA. If a write happens before it
			//     halts, the cancellation block above kills it completely.
			if (m_dma_engine.dmc.implicit_abort_halt_reads_required == 0)
			{
				if (m_dma_engine.dmc.halt_read_cycles >= 1 &&
					m_dma_engine.dmc.halt_write_cycles == 0)
				{
					m_maincpu6502->dmc_halt_next_read(true, false);
				}
			}
			else
			{
				if (m_dma_engine.dmc.halt_read_cycles >= m_dma_engine.dmc.implicit_abort_halt_reads_required &&
					m_dma_engine.dmc.halt_write_cycles == 0)
				{
					m_maincpu6502->dmc_halt_next_read(true, false);
				}
			}
		}

		tick();
		return true;
	}

	// --------------------------------------------------
	// OAM waiting for halt while CPU is still running.
	// --------------------------------------------------
	if (dma_engine_oam_waiting_for_halt() && !cpu_suspended)
	{
		tick();
		return true;
	}

	// --------------------------------------------------
	// DMC halt acknowledged.
	//
	// NESdev:
	// The halt cycle is a no-operation DMA cycle. The halted CPU repeats the
	// last read externally on no-op DMA cycles.
	//
	// DMC halt is no-op from the DMC side, so OAM can still use this same
	// cycle if it has a valid get/put access.
	// --------------------------------------------------
	if (dma_engine_dmc_waiting_for_halt() && cpu_suspended)
	{
		m_dmc_cpu_bus_latch = m_maincpu6502->get_prevReadAddress();

		dma_engine_service_oam_access_this_cycle();

		// --------------------------------------------------
		// Implicit 8/9 second halt accepted.
		//
		// This is the one-cycle aborted DMA after the first real DMC read has
		// already happened. No dummy cycle, no alignment cycle, no DMC get.
		// --------------------------------------------------
		if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_abort_8_9 &&
			m_dma_engine.dmc.implicit_abort_halt_reads_required > 0)
		{
			if (delayed_frame_irq_after_dmc)
			{
				delayed_frame_irq_after_dmc = false;
				//delayed_frame_irq = 2;
				delayed_frame_irq = apu_clk1_is_high ? 2 : 1;
			}

			dmc_loading_sample_byte = false;

			m_maincpu6502->dmc_clear_halt();

			dma_engine_mark_dmc_idle();
			dma_engine_resume_cpu_if_done();

			tick();
			return true;
		}

		// --------------------------------------------------
		// Explicit -2/-3 abort.
		//
		// This is the one-cycle explicit abort case. The old working path
		// called dmc_read(), but dmc_read() returns immediately for explicit
		// aborts before any real sample fetch or get/put validation.
		// --------------------------------------------------
		if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_2_or_3)
		{
			m_dma_engine.dmc.phase = dmc_dma_phase::read;

			dmc_read();

			m_dma_engine.dmc.phase = dmc_dma_phase::done;
			m_maincpu6502->dmc_clear_halt();

			dma_engine_mark_dmc_idle();
			dma_engine_resume_cpu_if_done();

			m_dma_engine.dmc.load_request = false;

			tick();
			return true;
		}

		// --------------------------------------------------
		// Implicit 6/7 unexpected-DMA replay.
		//
		// This rearms the same byte for the second DMC get. The normal DMC
		// dummy/read state below supplies the replay timing.
		// --------------------------------------------------
		if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_unexpected_6_7 &&
			m_dma_engine.dmc.replay_pending)
		{
			dmc_sample_cur_addr--;
			dmc_bytes_remaining++;

			m_dma_engine.dmc.replay_pending = false;
			m_dma_engine.dmc.clear_implicit_after_read = true;
		}

		// Normal DMC, explicit 0/-1, initial implicit 8/9,
		// and implicit 6/7 replay.
		//
		// NESdev:
		// After the DMC halt cycle, DMC performs a dummy no-op cycle. If the
		// following cycle is not get, it also spends an alignment no-op cycle.
		m_dma_engine.dmc.phase = dmc_dma_phase::dummy;

		tick();
		return true;
	}

	// --------------------------------------------------
	// OAM halt acknowledged.
	//
	// NESdev:
	// OAM DMA halt cycle, optional alignment cycle, then 256 get/put pairs.
	// --------------------------------------------------
	if (dma_engine_oam_waiting_for_halt() && cpu_suspended)
	{
		m_maincpu6502->oam_clear_halt();

		m_dma_engine.oam.phase = dma_engine_is_put_cycle()
			? oam_dma_phase::read
			: oam_dma_phase::align;

		tick();
		return true;
	}

	// --------------------------------------------------
	// No active suspended DMA.
	// --------------------------------------------------
	if (!cpu_suspended)
		return false;

	// --------------------------------------------------
	// DMC dummy phase.
	//
	// NESdev:
	// DMC dummy is a no-op cycle. Since it is not a DMC access cycle, OAM can
	// overlap with it if OAM has a valid access on this get/put cycle.
	// --------------------------------------------------
	if (m_dma_engine.dmc.active &&
		m_dma_engine.dmc.phase == dmc_dma_phase::dummy)
	{
		const bool oam_accessed = dma_engine_service_oam_access_this_cycle();

		if (!oam_accessed)
			m_maincpu6502->run_suspended_cpu_dma_cycle();

		m_dma_engine.dmc.phase = dmc_dma_phase::read;

		tick();
		return true;
	}

	// --------------------------------------------------
	// DMC read phase.
	//
	// NESdev:
	// DMC can only get/read on get cycles. If the current cycle is put, this
	// is a DMC alignment no-op. DMC get has priority over OAM get.
	// --------------------------------------------------
	if (m_dma_engine.dmc.active &&
		m_dma_engine.dmc.phase == dmc_dma_phase::read)
	{
		if (dma_engine_is_get_cycle())
		{
			// DMC GET wins over OAM GET. Do not call the OAM access service
			// here, because an OAM read must retry later if DMC steals get.
			//
			// Keep the old passing ordering:
			//   tick();
			//   dmc_read();
			//
			// This keeps delayed_dmc_irq / delayed_frame_irq_after_dmc from
			// being armed before this tick's IRQ-delay service point.
			tick();
			dma_engine_finish_dmc_read();
			return true;
		}

		// DMC wants get but current cycle is put, so this is DMC alignment.
		// OAM may still write on this put cycle.
		const bool oam_accessed = dma_engine_service_oam_access_this_cycle();

		if (!oam_accessed)
			m_maincpu6502->run_suspended_cpu_dma_cycle();

		tick();
		return true;
	}

	// --------------------------------------------------
	// OAM alignment.
	// --------------------------------------------------
	if (m_dma_engine.oam.active &&
		m_dma_engine.oam.phase == oam_dma_phase::align)
	{
		m_maincpu6502->run_suspended_cpu_dma_cycle();
		m_dma_engine.oam.phase = oam_dma_phase::read;

		tick();
		return true;
	}

	// --------------------------------------------------
	// OAM read.
	// --------------------------------------------------
	if (m_dma_engine.oam.active &&
		m_dma_engine.oam.phase == oam_dma_phase::read)
	{
		if (dma_engine_is_get_cycle())
			dma_engine_oam_read();
		else
			m_maincpu6502->run_suspended_cpu_dma_cycle();

		tick();
		return true;
	}

	// --------------------------------------------------
	// OAM write.
	// --------------------------------------------------
	if (m_dma_engine.oam.active &&
		m_dma_engine.oam.phase == oam_dma_phase::write)
	{
		if (dma_engine_is_put_cycle())
			dma_engine_oam_write();
		else
			m_maincpu6502->run_suspended_cpu_dma_cycle();

		tick();
		return true;
	}

	dma_engine_resume_cpu_if_done();
	return false;
}

void nesapu_device::dma_engine_reset()
{
	m_dma_engine.data_bus = 0;

	m_dma_engine.dmc.active = false;
	m_dma_engine.dmc.mode = dmc_dma_mode::none;
	m_dma_engine.dmc.phase = dmc_dma_phase::idle;
	m_dma_engine.dmc.load_request = false;
	m_dma_engine.dmc.replay_pending = false;
	m_dma_engine.dmc.clear_implicit_after_read = false;
	m_dma_engine.dmc.late_implicit_short = false;
	m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
	m_dma_engine.dmc.pending_delay = 0;
	m_dma_engine.dmc.halt_read_cycles = 0;
	m_dma_engine.dmc.halt_write_cycles = 0;

	m_dma_engine.oam.active = false;
	m_dma_engine.oam.phase = oam_dma_phase::idle;
	m_dma_engine.oam.base_addr = 0;
	m_dma_engine.oam.index = 0;
}

void nesapu_device::dma_engine_request_oam(uint16_t base_addr)
{
	m_dma_engine.oam.active = true;
	m_dma_engine.oam.phase = oam_dma_phase::waiting_for_halt;
	m_dma_engine.oam.base_addr = base_addr;
	m_dma_engine.oam.index = 0;
}

void nesapu_device::dma_engine_request_dmc(dmc_dma_mode mode)
{
	m_dma_engine.dmc.active = (mode != dmc_dma_mode::none);
	m_dma_engine.dmc.mode = mode;
	m_dma_engine.dmc.phase = (mode == dmc_dma_mode::none)
		? dmc_dma_phase::idle
		: dmc_dma_phase::waiting_for_halt;

	m_dma_engine.dmc.halt_read_cycles = 0;
	m_dma_engine.dmc.halt_write_cycles = 0;
	m_dma_engine.dmc.pending_delay = 0;

	if (mode == dmc_dma_mode::none)
	{
		m_dma_engine.dmc.replay_pending = false;
		m_dma_engine.dmc.clear_implicit_after_read = false;
		m_dma_engine.dmc.late_implicit_short = false;
		m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
	}
}

nesapu_device::dma_cycle_kind nesapu_device::dma_engine_cycle_kind() const
{
	return apu_clk1_is_high ? dma_cycle_kind::get : dma_cycle_kind::put;
}

bool nesapu_device::dma_engine_is_get_cycle() const
{
	return dma_engine_cycle_kind() == dma_cycle_kind::get;
}

bool nesapu_device::dma_engine_is_put_cycle() const
{
	return dma_engine_cycle_kind() == dma_cycle_kind::put;
}

bool nesapu_device::dma_engine_dmc_waiting_for_halt() const
{
	return m_dma_engine.dmc.active &&
		m_dma_engine.dmc.phase == dmc_dma_phase::waiting_for_halt;
}

bool nesapu_device::dma_engine_oam_waiting_for_halt() const
{
	return m_dma_engine.oam.active &&
		m_dma_engine.oam.phase == oam_dma_phase::waiting_for_halt;
}

void nesapu_device::dma_engine_clear_dmc_replay_state()
{
	m_dma_engine.dmc.replay_pending = false;
	m_dma_engine.dmc.clear_implicit_after_read = false;
	m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
	m_dma_engine.dmc.late_implicit_short = false;
}

void nesapu_device::dma_engine_mark_dmc_idle()
{
	m_dma_engine.dmc.active = false;
	m_dma_engine.dmc.mode = dmc_dma_mode::none;
	m_dma_engine.dmc.phase = dmc_dma_phase::idle;
	m_dma_engine.dmc.load_request = false;
	m_dma_engine.dmc.pending_delay = 0;
	m_dma_engine.dmc.halt_read_cycles = 0;
	m_dma_engine.dmc.halt_write_cycles = 0;
	dma_engine_clear_dmc_replay_state();
}

void nesapu_device::dma_engine_resume_cpu_if_done()
{
	if (m_dma_engine.dmc.active || m_dma_engine.oam.active)
		return;

	m_maincpu6502->dmc_clear_halt();
	m_maincpu6502->oam_clear_halt();

	if (m_maincpu_dev->suspended())
		m_maincpu_dev->resume(SUSPEND_REASON_HALT);
}

bool nesapu_device::dma_engine_service_oam_access_this_cycle()
{
	if (!m_dma_engine.oam.active)
		return false;

	if (m_dma_engine.oam.phase == oam_dma_phase::waiting_for_halt)
	{
		m_maincpu6502->oam_clear_halt();

		m_dma_engine.oam.phase = dma_engine_is_put_cycle()
			? oam_dma_phase::read
			: oam_dma_phase::align;

		return false;
	}

	if (m_dma_engine.oam.phase == oam_dma_phase::read)
	{
		if (dma_engine_is_get_cycle())
		{
			dma_engine_oam_read();
			return true;
		}

		return false;
	}

	if (m_dma_engine.oam.phase == oam_dma_phase::write)
	{
		if (dma_engine_is_put_cycle())
		{
			dma_engine_oam_write();
			return true;
		}

		return false;
	}

	if (m_dma_engine.oam.phase == oam_dma_phase::align)
	{
		m_dma_engine.oam.phase = oam_dma_phase::read;
		return false;
	}

	return false;
}

void nesapu_device::dma_engine_oam_read()
{
	auto &program = m_maincpu_dev->space(AS_PROGRAM);

	const uint16_t dma_addr = m_dma_engine.oam.base_addr | (m_dma_engine.oam.index & 0xff);
	const uint8_t old_open_bus = m_maincpu6502->get_open_bus();

	// APU register visibility during OAM DMA is controlled by the 6502
	// address bus, not the OAM DMA source address.
	const uint16_t cpu_addr_bus = m_maincpu6502->get_adr_bus();
	const bool apu_regs_active = ((cpu_addr_bus & 0xffe0) == 0x4000);

	const bool source_is_open_bus = m_maincpu6502->is_open_bus_address(dma_addr);
	const uint8_t reg_slot = dma_addr & 0x1f;

	if (!apu_regs_active)
	{
		if ((dma_addr & 0xff00) == 0x4000)
			m_dma_engine.data_bus = old_open_bus;
		else
			m_dma_engine.data_bus = program.read_byte(dma_addr);
	}
	else
	{
		if (source_is_open_bus)
		{
			if (reg_slot == 0x15)
			{
				m_dma_engine.data_bus = status_r();
			}
			else if (reg_slot == 0x16)
			{
				const uint8_t z = program.read_byte(0x4016);
				//m_dma_engine.data_bus = (old_open_bus & 0xfa) | (z & 0x01);
				m_dma_engine.data_bus = (old_open_bus & 0xe0) | (z & 0x1f);
			}
			else if (reg_slot == 0x17)
			{
				const uint8_t z = program.read_byte(0x4017);
				//m_dma_engine.data_bus = (old_open_bus & 0xfa) | (z & 0x01);
				m_dma_engine.data_bus = (old_open_bus & 0xe0) | (z & 0x1f);
			}
			else
			{
				m_dma_engine.data_bus = old_open_bus;
			}
		}
		else
		{
			if (reg_slot == 0x15)
			{
				const uint8_t src = program.read_byte(dma_addr);
				const uint8_t st = status_r();

				m_dma_engine.data_bus = (st & ~0x20) | (src & 0x20);
			}
			else if (reg_slot == 0x16)
			{
				program.read_byte(0x4016);
				m_dma_engine.data_bus = program.read_byte(dma_addr);
			}
			else if (reg_slot == 0x17)
			{
				program.read_byte(0x4017);
				m_dma_engine.data_bus = program.read_byte(dma_addr);
			}
			else
			{
				m_dma_engine.data_bus = program.read_byte(dma_addr);
			}
		}
	}

	m_maincpu6502->set_open_bus(m_dma_engine.data_bus);
	m_dma_engine.oam.phase = oam_dma_phase::write;
}

void nesapu_device::dma_engine_oam_write()
{
	m_ppu_dev->write_oam_dma_byte(m_dma_engine.data_bus);

	++m_dma_engine.oam.index;

	if (m_dma_engine.oam.index >= 256)
	{
		m_dma_engine.oam.active = false;
		m_dma_engine.oam.phase = oam_dma_phase::done;
		dma_engine_resume_cpu_if_done();
	}
	else
	{
		m_dma_engine.oam.phase = oam_dma_phase::read;
	}
}

void nesapu_device::dma_engine_finish_dmc_read()
{
	dmc_read();

	const bool needs_implicit_6_7_replay =
		m_dma_engine.dmc.mode == dmc_dma_mode::implicit_unexpected_6_7 &&
		!m_dma_engine.dmc.replay_pending &&
		!m_dma_engine.dmc.clear_implicit_after_read;

	if (needs_implicit_6_7_replay)
	{
		m_maincpu6502->dmc_clear_halt();

		m_dma_engine.dmc.replay_pending = true;
		m_dma_engine.dmc.halt_read_cycles = 0;
		m_dma_engine.dmc.halt_write_cycles = 0;
		m_dma_engine.dmc.phase = dmc_dma_phase::waiting_for_halt;

		return;
	}

	// --------------------------------------------------
	// Implicit 8/9 rearm.
	//
	// Match the old working flag-soup behavior:
	//
	//   after the first DMC read:
	//       m_maincpu6502->dmc_clear_halt();
	//       resume CPU;
	//       dmc_implicit_abort_halt_reads_required = 2;
	//       dmc_halt = true;
	//       dmc_halt_read_cycles = 0;
	//       dmc_halt_write_cycles = 0;
	//
	// That second halt window is the special one-cycle aborted DMA:
	//   - if it hits a write cycle, it is cancelled completely
	//   - if accepted, it is halt-only and performs no DMC read
	// --------------------------------------------------
	if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_abort_8_9 &&
		m_dma_engine.dmc.implicit_abort_halt_reads_required == 0)
	{
		m_maincpu6502->dmc_clear_halt();

		if (!m_dma_engine.oam.active && m_maincpu_dev->suspended())
			m_maincpu_dev->resume(SUSPEND_REASON_HALT);

		m_dma_engine.dmc.active = true;
		m_dma_engine.dmc.phase = dmc_dma_phase::waiting_for_halt;
		m_dma_engine.dmc.load_request = false;
		m_dma_engine.dmc.pending_delay = 0;

		m_dma_engine.dmc.implicit_abort_halt_reads_required = 2;
		m_dma_engine.dmc.halt_read_cycles = 0;
		m_dma_engine.dmc.halt_write_cycles = 0;

		return;
	}

	dma_engine_mark_dmc_idle();
	dma_engine_resume_cpu_if_done();
}


void nesapu_device::clock_dmc() {
	if (dpcm_active) {
		if (dmc_shift_reg & 1) {
			if (dmc_counter < 126) {
				dmc_counter += 2;
			}
		}
		else {
			if (dmc_counter > 1) {
				dmc_counter -= 2;
			}
		}

		dmc_shift_reg >>= 1;
	}

	if (--dmc_bits_remaining == 0) {
		dmc_bits_remaining = 8;
			
		if (dmc_sample_buffer_has_data)
		{
			dpcm_active = true;
			dmc_shift_reg = dmc_sample_buffer;
			dmc_sample_buffer_has_data = false;
		}
		else
		{
			dpcm_active = false;
		}
	}
	
	if (!dmc_sample_buffer_has_data && !dmc_loading_sample_byte && dmc_bytes_remaining > 0) {
		//reload DMC
		m_dma_engine.dmc.load_request = false;
		m_maincpu6502->dmc_halt_next_read(true, false);
		load_dmc_sample_byte();
	}
}

void nesapu_device::clock_noise_generator()
{
	const uint16_t feedback =
		((noise_shift_reg & 1) ^ ((noise_shift_reg >> noise_feedback_bit) & 1)) & 1;

	noise_shift_reg = (noise_shift_reg >> 1) | (feedback << 14);
	update_noise_output_level();
}

void nesapu_device::clock_triangle_generator() 
{
	if (tri_len_cnt > 0 && tri_lin_cnt > 0)
	{
		tri_waveform_pos = (tri_waveform_pos + 1) & 0x1f;

		tri_output_level = tri_waveform_steps[tri_waveform_pos];
	}
}

// The frame IRQ is set during three consecutive CPU ticks at the end of the
// frame period when in four-step mode, so we factor out this helper
void nesapu_device::check_frame_irq() {
	if (!inhibit_frame_irq) {
		set_frame_irq(true);
	}
}

// Quarter frame / envelope clock.
//
// Native 2A03:
//   - clocks pulse envelopes
//   - clocks noise envelope
//   - clocks triangle linear counter
//
void nesapu_device::clock_env_and_tri_lin() 
{
	// Pulse channels
	for (int n = 0; n < 2; ++n)
	{
		if (m_APU.pulse[n].env_start_flag)
		{
			m_APU.pulse[n].env_start_flag = false;

			m_APU.pulse[n].env_vol     = 15;
			m_APU.pulse[n].env_div_cnt = m_APU.pulse[n].vol;
		}
		else
		{
			if (m_APU.pulse[n].env_div_cnt == 0)
			{
				m_APU.pulse[n].env_div_cnt = m_APU.pulse[n].vol;

				if (m_APU.pulse[n].env_vol > 0)
					--m_APU.pulse[n].env_vol;
				else if (m_APU.pulse[n].halt_len_loop_env)
					m_APU.pulse[n].env_vol = 15;
			}
			else
			{
				--m_APU.pulse[n].env_div_cnt;
			}
		}

		update_pulse_output_level(n);
	}

	// Noise channel
	if (noise_env_start_flag)
	{
		noise_env_start_flag = false;

		noise_env_vol     = 15;
		noise_env_div_cnt = noise_vol;
	}
	else
	{
		if (noise_env_div_cnt == 0)
		{
			noise_env_div_cnt = noise_vol;

			if (noise_env_vol > 0)
				--noise_env_vol;
			else if (noise_halt_len_loop_env)
				noise_env_vol = 15;
		}
		else
		{
			--noise_env_div_cnt;
		}
	}

	update_noise_output_level();

	// Triangle linear counter
	if (tri_lin_cnt_reload_flag)
	{
		tri_lin_cnt = tri_lin_cnt_load;
	}
	else if (tri_lin_cnt > 0)
	{
		--tri_lin_cnt;
	}

	// The control flag is the same bit as the triangle length-counter halt flag.
	// If clear, the linear-counter reload flag is cleared after the clock.
	// If set, the reload flag remains set, causing the counter to reload every
	// quarter-frame clock.
	if (!tri_halt_flag)
		tri_lin_cnt_reload_flag = false;
	}

// Half frame
/*void nesapu_device::clock_len_and_sweep() 
{
	for (int n = 0; n < 2; ++n) {
		if (!m_APU.pulse[n].halt_len_loop_env && m_APU.pulse[n].len_cnt > 0) {
			--m_APU.pulse[n].len_cnt;
			update_pulse_output_level(n);
		}

		if (m_APU.pulse[n].sweep_period_cnt == 0 &&
			m_APU.pulse[n].sweep_enabled &&
			m_APU.pulse[n].sweep_shift > 0 &&
			m_APU.pulse[n].period >= 8 &&
			m_APU.pulse[n].sweep_target_period <= 0x7ff) {
				m_APU.pulse[n].period = m_APU.pulse[n].sweep_target_period;
				update_sweep_target_period(n);
				update_pulse_output_level(n);
		}


		if (m_APU.pulse[n].sweep_reload_flag || m_APU.pulse[n].sweep_period_cnt == 0) {
			m_APU.pulse[n].sweep_reload_flag = false;
			m_APU.pulse[n].sweep_period_cnt = m_APU.pulse[n].sweep_period;
		} else {
			--m_APU.pulse[n].sweep_period_cnt;
		}
	}

	if (!tri_halt_flag && tri_len_cnt > 0)
		--tri_len_cnt;

	if (!noise_halt_len_loop_env && noise_len_cnt > 0) {
		--noise_len_cnt;
		update_noise_output_level();
	}
}*/

void nesapu_device::clock_len_and_sweep() 
{
	for (int n = 0; n < 2; ++n) {
		if (!m_APU.pulse[n].halt_len_loop_env && m_APU.pulse[n].len_cnt > 0) {
			--m_APU.pulse[n].len_cnt;
			update_pulse_output_level(n);
		}

		// BreakingNES SW_UVF blocks sweep when frequency bits [10:2] are zero,
		// i.e. period < 4.  Keep this separate from output muting, which still
		// mutes pulse output when period < 8.
		if (m_APU.pulse[n].sweep_period_cnt == 0 &&
			m_APU.pulse[n].sweep_enabled &&
			m_APU.pulse[n].sweep_shift > 0 &&
			m_APU.pulse[n].period >= 4 &&
			m_APU.pulse[n].sweep_target_period <= 0x7ff) {
				m_APU.pulse[n].period = m_APU.pulse[n].sweep_target_period;
				update_sweep_target_period(n);
				update_pulse_output_level(n);
		}

		if (m_APU.pulse[n].sweep_reload_flag || m_APU.pulse[n].sweep_period_cnt == 0) {
			m_APU.pulse[n].sweep_reload_flag = false;
			m_APU.pulse[n].sweep_period_cnt = m_APU.pulse[n].sweep_period;
		} else {
			--m_APU.pulse[n].sweep_period_cnt;
		}
	}

	if (!tri_halt_flag && tri_len_cnt > 0)
		--tri_len_cnt;

	if (!noise_halt_len_loop_env && noise_len_cnt > 0) {
		--noise_len_cnt;
		update_noise_output_level();
	}
}

// The actual frame counter counts at half the CPU frequency, but the half and
// quarter frame signals are delayed by one CPU cycle, making it easier to
// treat it as counting in CPU cycles.
//
// T1-T5 are the times in CPU ticks for the quarter frame and half frame
// signals, in ascending order. They differ between NTSC and PAL.
/*
		T1=2*3728;
		T2=2*7456;
		T3=2*11185;
		T4=2*14914;
		T5=2*18640;
		
*/

// Prevent duplicate frame-unit clocks when a scheduled frame step
// and delayed $4017 reset/immediate 5-step clock overlap the same
// APU put/get boundary.
bool nesapu_device::frame_unit_clock_allowed()
{
	const uint64_t apu_cycle = uint64_t(cpu_cycle) >> 1;

	return last_frame_unit_pulse_apu_cycle != apu_cycle;
}

void nesapu_device::arm_frame_unit_clock_block()
{
	const uint64_t apu_cycle = uint64_t(cpu_cycle) >> 1;

	last_frame_unit_pulse_apu_cycle = apu_cycle;
}

void nesapu_device::clock_frame_counter()
{
	const int q1 = m_is_pal ?  8313 :  7457;
	const int h2 = m_is_pal ? 16627 : 14913;
	const int q3 = m_is_pal ? 24939 : 22371;
	const int h4 = m_is_pal ? 33253 : 29829;

	const int irq0 = m_is_pal ? 33252 : 29828;
	const int irq1 = m_is_pal ? 33253 : 29829;
	const int irq2 = m_is_pal ? 33254 : 29830;

	const int wrap4 = m_is_pal ? 33254 : 29830;

	const int h5 = m_is_pal ? 41565 : 37281;
	const int wrap5 = m_is_pal ? 41566 : 37282;

	switch (frame_counter_mode)
	{
	case FOUR_STEP:
		++frame_counter_clock;

		if (frame_counter_clock == q1 || frame_counter_clock == q3) {
			if (frame_unit_clock_allowed()) {
				clock_env_and_tri_lin();
				arm_frame_unit_clock_block();
			}
		}

		if (frame_counter_clock == h2 || frame_counter_clock == h4) {
			if (frame_unit_clock_allowed()) {
				clock_len_and_sweep();
				clock_env_and_tri_lin();
				arm_frame_unit_clock_block();
			}
		}

		if (frame_counter_clock == irq0 ||
			frame_counter_clock == irq1 ||
			frame_counter_clock == irq2)
		{
			if (inhibit_frame_irq) {
				set_frame_irq_flag_only();

				if (frame_counter_clock == irq0)
					frame_irq_suppress_clear_cycle = 3;
				else if (frame_counter_clock == irq1)
					frame_irq_suppress_clear_cycle = 2;
				else
					frame_irq_suppress_clear_cycle = 1;
			} else {
				check_frame_irq();
			}
		}

		if (frame_counter_clock == wrap4)
			frame_counter_clock = 0;

		break;

	case FIVE_STEP:
		++frame_counter_clock;

		if (frame_counter_clock == h2 || frame_counter_clock == h5) {
			if (frame_unit_clock_allowed()) {
				clock_len_and_sweep();
				clock_env_and_tri_lin();
				arm_frame_unit_clock_block();
			}
		}

		if (frame_counter_clock == q1 || frame_counter_clock == q3) {
			if (frame_unit_clock_allowed()) {
				clock_env_and_tri_lin();
				arm_frame_unit_clock_block();
			}
		}

		if (frame_counter_clock == wrap5)
			frame_counter_clock = 0;

		break;
	}

	if (delayed_frame_timer_reset > 0 && --delayed_frame_timer_reset == 0) {
		frame_counter_mode = new_frame_counter_mode;
		frame_counter_clock = 0;

		if (frame_counter_mode == FIVE_STEP) {
			if (frame_unit_clock_allowed()) {
				clock_env_and_tri_lin();
				clock_len_and_sweep();
				arm_frame_unit_clock_block();
			}
		}
	}
}

void nesapu_device::clock_pulse_generator(unsigned n)
{
	assert(n < 2);
	assert(m_APU.pulse[n].duty < 4);
	assert(m_APU.pulse[n].waveform_pos < 8);

	// Advance the 8-step duty sequencer.
	m_APU.pulse[n].waveform_pos = (m_APU.pulse[n].waveform_pos + 1) & 7;

	// Refresh the current output level after the sequencer step changes.
	update_pulse_output_level(n);
}

void nesapu_device::update_pulse_output_level(unsigned n) 
{
	const bool sweep_mutes = (m_APU.pulse[n].sweep_target_period > 0x7FF);
	const bool period_mutes = (m_APU.pulse[n].period < 8);

	uint8_t new_output = 0;

	if (m_APU.pulse[n].len_cnt != 0 &&
		!period_mutes &&
		pulse_duties[m_APU.pulse[n].duty][m_APU.pulse[n].waveform_pos] &&
		!sweep_mutes)
	{
		new_output = m_APU.pulse[n].const_vol ? m_APU.pulse[n].vol : m_APU.pulse[n].env_vol;
	}

	m_APU.pulse[n].output_level = new_output;
}

void nesapu_device::update_sweep_target_period(unsigned n) {
    int addition = m_APU.pulse[n].period >> m_APU.pulse[n].sweep_shift;
    // The adder on the first pulse channel is missing the carry in to the
    // first bit for some unknown reason
    if (m_APU.pulse[n].sweep_negate) 
		addition = (n == 0) ? ~addition : -addition;
    m_APU.pulse[n].sweep_target_period = (int)m_APU.pulse[n].period + addition;
}

void nesapu_device::update_noise_output_level()
{
	uint8_t new_output = 0;

	if (noise_len_cnt != 0 && !(noise_shift_reg & 1))
		new_output = noise_const_vol ? noise_vol : noise_env_vol;

	noise_output_level = new_output;
}

uint8_t nesapu_device::read(offs_t offset)
{
	return m_maincpu6502->get_open_bus();
}

/* WRITE REGISTER VALUE */
void nesapu_device::write(offs_t offset, u8 value)
{
	//logerror("Write:  $%02X at $%04X, cpu: %d\n", value, offset, m_maincpu6502->total_cycles());

	// For pulse channel registers, bit 2 selects the channel:
	//   0 = pulse 1 ($4000-$4003)
	//   1 = pulse 2 ($4004-$4007)
	int chan = BIT(offset, 2);
	
	switch (offset)
	{
		/* squares */
		case apu_t::WRA0: //$4000 / $4004	DDLC VVVV	Duty (D), envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
		case apu_t::WRB0: // $4004
			// Bits 7:6 select the pulse duty sequence.
			m_APU.pulse[chan].duty = (value >> 6) & 0x03;

			// Bit 4 selects constant volume vs envelope.
			m_APU.pulse[chan].const_vol = (value & 0x10) != 0;

			// Low 4 bits are the constant volume value or envelope divider period.
			m_APU.pulse[chan].vol = value & 0x0F;

			// Bit 5 controls envelope looping / length counter halt.
			// In this core, apply that control change one APU cycle later
			// to match your delayed halt timing behavior.
			if (chan == 0) {
				temp_halt_len_loop_env_0 = (value & 0x20) != 0;
				delay_halt_len_loop_0 = 2;
			} else {
				temp_halt_len_loop_env_1 = (value & 0x20) != 0;
				delay_halt_len_loop_1 = 2;
			}

			// Refresh output state in case gating changes immediately.
			update_pulse_output_level(chan);
			break;

		case apu_t::WRA1: // $4001
		case apu_t::WRB1: // $4005
			// Bit 7 enables the sweep unit.
			m_APU.pulse[chan].sweep_enabled = (value & 0x80) != 0;

			// Bit 3 selects add vs negate mode.
			m_APU.pulse[chan].sweep_negate = (value & 0x08) != 0;

			// Bits 6:4 select the sweep divider period.
			m_APU.pulse[chan].sweep_period = (value >> 4) & 0x07;

			// Bits 2:0 select the sweep shift count.
			m_APU.pulse[chan].sweep_shift = value & 0x07;

			// Writing here reloads the sweep divider on the next sweep clock.
			m_APU.pulse[chan].sweep_reload_flag = true;

			// Recompute the sweep target and output gating immediately.
			update_sweep_target_period(chan);
			update_pulse_output_level(chan);
			break;

		case apu_t::WRA2: // $4002
		case apu_t::WRB2: // $4006
			// $4002/$4006 write the low 8 bits of the pulse timer period.
			m_APU.pulse[chan].period = (m_APU.pulse[chan].period & ~0x00FF) | value;

			// Recompute sweep target/output because the timer period changed.
			update_sweep_target_period(chan);
			update_pulse_output_level(chan);

			break;

		case apu_t::WRA3:
		case apu_t::WRB3: // $4003 / $4007
			// If the channel is enabled, reload the length counter from bits 7:3.
			// Schedule the length reload through the delayed register-effect path.
			// delay = 2 means it will not commit on the current post-write tick;
			// it can land on the next APU tick before the frame length clock.
			// This is needed for 11.len_reload_timing edge cases.
			if (m_APU.pulse[chan].enabled) {
				if (chan == 0) {
					temp_len_cnt_0_reload_value = len_table[(value >> 3) & 0x1F];
					temp_len_cnt_0 = m_APU.pulse[chan].len_cnt;
					delay_len_cnt_0 = 2;
				} else {
					temp_len_cnt_1_reload_value = len_table[(value >> 3) & 0x1F];
					temp_len_cnt_1 = m_APU.pulse[chan].len_cnt;
					delay_len_cnt_1 = 2;
				}
			}

			// Bits 2:0 provide the high 3 bits of the 11-bit pulse timer period.
			m_APU.pulse[chan].period =
				(m_APU.pulse[chan].period & ~0x700) | ((value & 0x07) << 8);

			// Writing here resets the duty sequencer and restarts the envelope.
			m_APU.pulse[chan].waveform_pos   = 0;
			m_APU.pulse[chan].env_start_flag = true;

			// Refresh sweep/output state after the timer update.
			update_sweep_target_period(chan);

			break;

		/* triangle */
		case apu_t::WRC0: //$4008	CRRR RRRR	Length counter halt / linear counter control (C), linear counter load (R)
			// $4008 bits 6:0 set the triangle linear counter reload value.
			tri_lin_cnt_load = value & 0x7F;

			// Bit 7 is the triangle control flag:
			//   - also acts as the linear counter control flag
			//   - also halts the length counter when set
			//
			// In this core, apply that control-flag change one APU cycle later
			// to match your delayed halt timing behavior.
			temp_tri_halt_flag = (value & 0x80) != 0;
			delay_tri_halt_flag = 2;
			break;

		case 0x4009:
			/* unused */
			break;

		case apu_t::WRC2: // $400A
			// $400A writes the low 8 bits of the triangle timer period.
			tri_period = (tri_period & ~0x00FF) | value;
			break;

		case apu_t::WRC3: // $400B
			// Writing $400B sets the triangle linear-counter reload flag.
			// The actual linear counter reload happens on the next quarter-frame clock.
			tri_lin_cnt_reload_flag = true;

			// Bits 2:0 provide the high 3 bits of the 11-bit triangle timer period.
			tri_period = (tri_period & ~0x700) | ((value & 0x07) << 8);

			// If the triangle channel is enabled, reload the length counter from bits 7:3.
			// In this core, the length reload is applied one APU cycle later.
			if (tri_enabled) {
				temp_tri_len_cnt_reload_value = len_table[(value >> 3) & 0x1F];
				temp_tri_len_cnt = tri_len_cnt;
				delay_tri_len_cnt = 2;
			}

			break;

		/* noise */
		case apu_t::WRD0: //$400C	--LC VVVV	Envelope loop / length counter halt (L), constant volume (C), volume/envelope (V)
			// Bit 4 selects constant volume vs envelope.
			noise_const_vol = (value & 0x10) != 0;

			// Low 4 bits are the constant volume value or envelope divider period.
			noise_vol = value & 0x0F;

			// Bit 5 controls envelope looping / length counter halt.
			// In this core, that control change is applied one APU cycle later
			// to match the delayed halt/loop timing behavior.
			temp_noise_halt_len_loop_env = (value & 0x20) != 0;
			delay_noise_halt_len_loop_env = 2;

			// Refresh output state in case gating changes immediately.
			update_noise_output_level();
			break;

		case 0x400D:
			/* unused */
			break;

		case apu_t::WRD2: // $400E  M--- PPPP
			// Bit 7 selects the noise shift-register tap:
			//   0 = normal mode  (tap bit 1)
			//   1 = short mode   (tap bit 6)
			noise_feedback_bit = BIT(value, 7) ? 6 : 1;

			// Low 4 bits select the noise timer period.
			noise_period = noise_periods[value & 0x0F];
			
			break;

		case apu_t::WRD3: // $400F
			// Writing $400F restarts the noise envelope.
			noise_env_start_flag = true;

			// If the noise channel is enabled, reload the length counter from
			// bits 7:3. In this core, the length reload is applied one APU cycle later
			// to match the delayed length-reload timing tests.
			if (noise_enabled) {
				temp_noise_len_cnt_reload_value = len_table[(value >> 3) & 0x1F];
				temp_noise_len_cnt = noise_len_cnt;
				delay_noise_len_cnt = 2;
			}

			// Refresh output state in case gating changes immediately.
			update_noise_output_level();
			break;

		/* DMC */
		case apu_t::WRE0: { // $4010  IL-- RRRR
			// Bit 7 enables DMC IRQ generation at sample end.
			const bool new_irq_enable = (value & 0x80) != 0;

			// Bit 6 enables looping of the sample.
			dmc_loop_sample = (value & 0x40) != 0;

			// Clearing IRQ enable also clears any pending DMC IRQ.
			if (!new_irq_enable)
				set_dmc_irq(false);

			// Latch the new IRQ enable state and update the playback period
			// from the low 4-bit rate index.
			dmc_irq_enabled = new_irq_enable;
			dmc_period = dmc_periods[value & 0x0F];
			break;
		}

		case apu_t::WRE1:  // $4011: direct 7-bit DMC DAC load
		{
			dmc_counter = value & 0x7F;
			break;
		}

		case apu_t::WRE2:
			// $4012 sets DMC sample start address as $C000 + (value * 64)
			dmc_sample_start_addr = 0xC000 | (value << 6);
			break;

		case apu_t::WRE3:
			// $4013 sets DMC sample length as (value * 16) + 1 bytes
			dmc_sample_len = (value << 4) | 0x0001;
			break;

		case apu_t::IRQCTRL:  
		{
			//$4017	MI-- ----	Mode (M, 0 = 4-step, 1 = 5-step), IRQ inhibit flag (I)
			// Bit 7 selects the frame counter mode:
			//   0 = four-step sequence
			//   1 = five-step sequence
			new_frame_counter_mode = BIT(value, 7) ? FIVE_STEP : FOUR_STEP;
			
			// Bit 6 inhibits the frame IRQ.
			// On hardware, writing $4017 with IRQ inhibit set also clears any
			// currently pending/asserted frame IRQ condition.
			inhibit_frame_irq = BIT(value, 6);
			if (inhibit_frame_irq) {
				// Inhibit immediately clears both the $4015-visible frame IRQ flag
				// and the actual CPU IRQ output contribution.
				frame_irq = false;
				frame_irq_output = false;
				//frame_irq_no_clear_before = 0;
				delayed_frame_irq = 0;
				delayed_frame_irq_clear = 0;
				frame_irq_suppress_clear_cycle = 0;
				update_irq_output();
			}

			// The frame counter reset does not happen immediately.
			// It takes effect after either 3 or 4 CPU cycles depending on the
			// current CPU/APU phase, so schedule the delayed reset here.
			delayed_frame_timer_reset = ((m_maincpu_dev->total_cycles() & 0x01) == 0) ? 4 : 3;
			break;
		}
		case apu_t::SMASK:	{			//status $4015
			// logerror("	Write to $4015: CPU Cycles: %d, Value: $%02X, high/low: %d\n", cpu_cycle+1, value, apu_clk1_is_high);
			// We need to clear the DMC IRQ before handling the DMC enable/disable in
			// case a one-byte sample is loaded below, which will immediately fire a
			// DMC IRQ
			if (dmc_irq)
				set_dmc_irq(false);
			
			for (int n = 0; n < 2; ++n)
			{
				const bool old_enabled = m_APU.pulse[n].enabled;
				const bool new_enabled = (value & (1 << n)) != 0;

				m_APU.pulse[n].enabled = new_enabled;

				if (!new_enabled)
					m_APU.pulse[n].len_cnt = 0;

				if (old_enabled != new_enabled || !new_enabled)
					update_pulse_output_level(n);
			}

			tri_enabled = (value & 0x04) != 0;
			if (!tri_enabled) {
				tri_len_cnt = 0;
			}

			noise_enabled = (value & 0x08) != 0;
			if (!noise_enabled) {
				noise_len_cnt = 0;
				update_noise_output_level();
			}

			// DMC enable bit. We model DMC enabled/disabled through the number of
			// sample bytes that remain (greater than zero => enabled).
			if (!(value & 0x10)) {
				/*
				DMC DMA suffers from two bugs related to sample playback stopping around the time a 
				DMC output cycle ends, which is what empties the sample buffer and triggers a reload DMA. 
				This can happen explicitly, where the sample is stopped by clearing $4015 D4.
				*/
				// explicit disabling DMC 0 cycle before reload causes the DMC to run 3 cycles
				if (dmc_cycles_until_buffer_empty == (dmc_period * 8)) {
					m_dma_engine.dmc.load_request = false;
					
					m_dma_engine.dmc.halt_read_cycles = 0;
					m_dma_engine.dmc.halt_write_cycles = 0;

					dma_engine_request_dmc(dmc_dma_mode::explicit_abort_0);
					
					detect_abort_0++;
					//logerror("Explicit: DMC Bug Delay = 0\n");
					//osd_printf_info("Explicit DMA Abort Detected: 0\n");
				} // explicit disabling DMC 1 cycle before reload causes the DMC to run 4 cycles
				else if (dmc_cycles_until_buffer_empty == 1) {
					m_dma_engine.dmc.load_request = false;
					
					m_dma_engine.dmc.halt_read_cycles = 0;
					m_dma_engine.dmc.halt_write_cycles = 0;

					dma_engine_request_dmc(dmc_dma_mode::explicit_abort_minus_1);

					detect_abort_1++;
					//logerror("Explicit: DMC Bug Delay = -1\n");
					//osd_printf_info("Explicit DMA Abort Detected: -1\n");
				} // explicit disabling DMC 2 cycle before reload causes the DMC to run 1 cycle
				else if (dmc_cycles_until_buffer_empty == 2 || dmc_cycles_until_buffer_empty == 3) {
					m_dma_engine.dmc.load_request = false;
					
					m_dma_engine.dmc.halt_read_cycles = 0;
					m_dma_engine.dmc.halt_write_cycles = 0;

					detect_abort_2_3++;

					dma_engine_request_dmc(dmc_dma_mode::explicit_abort_minus_2_or_3);
					
					//logerror("Explicit: DMC Bug Delay = -2 or -3\n");
					//osd_printf_info("Explicit DMA Abort Detected: -2 or -3\n");
				}
				dmc_bytes_remaining = 0;
				m_dma_engine.dmc.load_request = false;
			} else if (dmc_bytes_remaining == 0) {
				dmc_sample_cur_addr = dmc_sample_start_addr;
				dmc_bytes_remaining = dmc_sample_len;
					
				dmc_4015_load_defer_pending = false;
				dmc_4015_load_defer_delay = 0;
				
				//test L detection
				if (dmc_cycles_until_buffer_empty == 2) {
					//osd_printf_info("Test L [APU Delta Modulation Channel] Detected\n");
					detect_test_l++;
					dmc_4015_load_defer_pending = true;
					dmc_4015_load_defer_delay = 1;
				} //test M detection
				else if (dmc_cycles_until_buffer_empty == 1) {
					//osd_printf_info("Test M [APU Delta Modulation Channel] Detected\n");
					detect_test_m++;
					dmc_4015_load_defer_pending = true;
					dmc_4015_load_defer_delay = 1;
				} 
					
				const bool defer_4015_dmc_load = dmc_4015_load_defer_pending;

				if (!dmc_sample_buffer_has_data && !defer_4015_dmc_load)
				{
					m_dma_engine.dmc.load_request = true;
					m_dma_engine.dmc.late_implicit_short = false;

					const int implicitOffset = ((dmc_period * 8) - dmc_cycles_since_buffer_empty);

					if (!dmc_loop_sample && dmc_bytes_remaining == 1)
					{
						if (implicitOffset == 6 || implicitOffset == 7)
						{
							dma_engine_request_dmc(dmc_dma_mode::implicit_unexpected_6_7);

							m_dma_engine.dmc.replay_pending = false;
							m_dma_engine.dmc.clear_implicit_after_read = false;
							m_dma_engine.dmc.late_implicit_short = false;
							m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
						}
						else if (implicitOffset == 8 || implicitOffset == 9)
						{
							dma_engine_request_dmc(dmc_dma_mode::implicit_abort_8_9);

							m_dma_engine.dmc.replay_pending = false;
							m_dma_engine.dmc.clear_implicit_after_read = false;
							m_dma_engine.dmc.late_implicit_short = false;

							// Must be non-zero so the runner can distinguish accepted halt
							// from cancellation when the halt is delayed into a CPU write.
							m_dma_engine.dmc.implicit_abort_halt_reads_required = 1;

							detect_abort_8_9++;
						}
					}
					else if (dmc_loop_sample && dmc_bytes_remaining == 1)
					{
						if (implicitOffset == 6 || implicitOffset == 7)
						{
							dma_engine_request_dmc(dmc_dma_mode::implicit_unexpected_6_7);

							m_dma_engine.dmc.replay_pending = false;
							m_dma_engine.dmc.clear_implicit_after_read = false;
							m_dma_engine.dmc.late_implicit_short = true;
							m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;
						}
					}

					load_dmc_sample_byte();
				}
			}
		break;
		}
		default:
			#ifdef MAME_DEBUG
			logerror("invalid apu write: $%02X at $%04X\n", value, offset);
			#endif
		break;
	}
}

void nesapu_device::do_oam_dma(address_space &space, const uint8_t page)
{
	const uint16_t oam_base_addr = uint16_t(page) << 8;

	dma_engine_request_oam(oam_base_addr);

	// Do not clear DMC halt here. OAM DMA starting while a DMC DMA is already
	// requested must not cancel the DMC halt. The arbiter resolves the overlap.
	m_maincpu6502->oam_halt_next_read();
}

void nesapu_device::load_dmc_sample_byte()
{
	m_dma_engine.dmc.halt_read_cycles = 0;
	m_dma_engine.dmc.halt_write_cycles = 0;

	assert(dmc_bytes_remaining > 0);

	// A $4015 enable write can be followed closely by a DMC clock/reload.
	// If a DMC sample-byte DMA is already pending, do not restart or
	// reclassify it.
	if (dmc_loading_sample_byte)
		return;

	dmc_loading_sample_byte = true;

	// --------------------------------------------------
	// Select the DMA mode for this sample-byte request.
	//
	// Normal DMC:
	//   - normal_load   = explicit $4015 enable/load
	//   - normal_reload = output unit emptied the sample buffer
	//
	// Special DMC stop-bug modes are detected before this point and must
	// survive this request conversion.
	// --------------------------------------------------
	dmc_dma_mode dmc_mode = m_dma_engine.dmc.load_request
		? dmc_dma_mode::normal_load
		: dmc_dma_mode::normal_reload;

	if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_0)
		dmc_mode = dmc_dma_mode::explicit_abort_0;
	else if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_1)
		dmc_mode = dmc_dma_mode::explicit_abort_minus_1;
	else if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_2_or_3)
		dmc_mode = dmc_dma_mode::explicit_abort_minus_2_or_3;
	else if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_abort_8_9)
		dmc_mode = dmc_dma_mode::implicit_abort_8_9;
	else if (m_dma_engine.dmc.mode == dmc_dma_mode::implicit_unexpected_6_7)
		dmc_mode = dmc_dma_mode::implicit_unexpected_6_7;

	// dma_engine_request_dmc() resets per-request state. Preserve the
	// implicit-stop policy bits that were detected before the request was
	// armed.
	const bool old_replay_pending = m_dma_engine.dmc.replay_pending;
	const bool old_clear_implicit_after_read = m_dma_engine.dmc.clear_implicit_after_read;
	const bool old_late_implicit_short = m_dma_engine.dmc.late_implicit_short;

	dma_engine_request_dmc(dmc_mode);

	m_dma_engine.dmc.replay_pending = old_replay_pending;
	m_dma_engine.dmc.clear_implicit_after_read = old_clear_implicit_after_read;
	m_dma_engine.dmc.late_implicit_short = old_late_implicit_short;

	// --------------------------------------------------
	// Implicit-stop 8/9 setup.
	//
	// NESdev:
	// If playback is stopped shortly after a reload DMA, an aborted DMA can
	// occur. For the 8/9 case, the first DMA still performs the normal
	// halt/dummy/read sequence. After that first read completes,
	// dma_engine_finish_dmc_read() arms the second one-cycle aborted DMA by
	// setting implicit_abort_halt_reads_required = 2.
	//
	// Do not initialize this to 1 here. That incorrectly turns the first pass
	// into the later one-cycle aborted DMA and breaks the implicit abort timing
	// test.
	// --------------------------------------------------
	if (dmc_mode == dmc_dma_mode::implicit_abort_8_9)
		m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;

	// --------------------------------------------------
	// Reload timing counters.
	//
	// These are part of the DMC reload/stop-bug detector and must be reset
	// when the sample-byte request is armed, not later in dmc_read().
	// --------------------------------------------------
	dmc_cycles_until_buffer_empty = (dmc_period * 8);
	dmc_cycles_since_buffer_empty = -1;
}

void nesapu_device::dmc_read()
{
	// --------------------------------------------------
	// Explicit DMC abort cleanup.
	//
	// NESdev DMC stop-bug cases:
	// These explicit abort modes are not real DMC sample fetches here.
	// They are cleanup paths for an aborted DMA. They must return before:
	//
	//   - GET/PUT validation
	//   - memory read from dmc_sample_cur_addr
	//   - sample buffer/address/count updates
	//
	// This preserves the old passing behavior where explicit 0, -1, and
	// -2/-3 all returned at the top of dmc_read().
	// --------------------------------------------------
	if (m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_0 ||
		m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_1 ||
		m_dma_engine.dmc.mode == dmc_dma_mode::explicit_abort_minus_2_or_3)
	{
		dma_engine_request_dmc(dmc_dma_mode::none);

		if (delayed_frame_irq_after_dmc)
		{
			// A frame IRQ was held back while DMC loading/abort handling was active.
			delayed_frame_irq_after_dmc = false;
			//delayed_frame_irq = 2;
			delayed_frame_irq = apu_clk1_is_high ? 2 : 1;
		}

		dmc_loading_sample_byte = false;
		return;
	}

	m_dma_engine.dmc.load_request = false;

	// --------------------------------------------------
	// Implicit 6/7 replay cleanup.
	//
	// The replay path temporarily backs up the address/count so the same byte
	// can be fetched again. After the replay read completes, clear the replay
	// policy state and return to normal DMC behavior.
	// --------------------------------------------------
	if (m_dma_engine.dmc.clear_implicit_after_read)
	{
		m_dma_engine.dmc.implicit_abort_halt_reads_required = 0;

		m_dma_engine.dmc.replay_pending = false;
		m_dma_engine.dmc.mode = dmc_dma_mode::none;

		m_dma_engine.dmc.late_implicit_short = false;
		m_dma_engine.dmc.clear_implicit_after_read = false;
	}

	// DMC DMA performs the actual sample fetch only on a GET cycle.
	if (!dma_engine_is_get_cycle())
	{
		logerror("Detected Error: DMC_READ On Put Cycle\n");
		osd_printf_info("Detected Error: DMC_READ On Put Cycle\n");
	}

	if (dmc_bytes_remaining > 0)
	{
		const uint16_t cpu_bus_addr = m_dmc_cpu_bus_latch;

		// --------------------------------------------------
		// Real DMC sample fetch.
		//
		// The cartridge/sample ROM provides the raw byte first. If the mixed
		// Ricoh/APU address activates $4016/$4017, the value accepted by the
		// DMC sample buffer can be the live bus-conflict value, while the
		// value left on external/open bus can settle differently afterward.
		// --------------------------------------------------
		dmc_sample_buffer = m_mem_read_cb(dmc_sample_cur_addr);

		const uint8_t raw_dmc_sample = dmc_sample_buffer;
		uint8_t external_bus = raw_dmc_sample;

		if ((cpu_bus_addr & 0xffe0) == 0x4000)
		{
			const uint16_t apu_reg = 0x4000 | (dmc_sample_cur_addr & 0x1f);

			if (apu_reg == 0x4015)
			{
				status_r();
			}
			else if (apu_reg == 0x4016 || apu_reg == 0x4017)
			{
				const uint8_t ctrl = m_maincpu6502->read_4016_4017(apu_reg);

				// Value the DMC/Ricoh-side sample fetch sees during the live bus conflict:
				// D0-D4 are an AND conflict between joypad/expansion and DMC/cart.
				// D5-D7 come from the DMC/cart byte.
				const uint8_t dmc_conflicted_sample =
					(raw_dmc_sample & 0xe0) |
					((ctrl & 0x1f) & (raw_dmc_sample & 0x1f));

				// Value left on the external/open bus after the read ends:
				// D0-D4 are left by the joypad hex inverter.
				// D5-D7 remain from the DMC/cart byte.
				external_bus =
					(ctrl & 0x1f) |
					(raw_dmc_sample & 0xe0);

				dmc_sample_buffer = dmc_conflicted_sample;
			}
		}

		// DMC DMA publishes to the external/open bus.
		// It does not update the CPU internal data bus like a normal CPU read.
		m_dma_engine.data_bus = external_bus;
		m_maincpu6502->set_open_bus(external_bus);

		dmc_sample_buffer_has_data = true;
		dmc_loading_sample_byte = false;

		dmc_sample_cur_addr = ((dmc_sample_cur_addr + 1) & 0x7fff) | 0x8000;

		if (--dmc_bytes_remaining == 0)
		{
			if (dmc_loop_sample)
			{
				dmc_sample_cur_addr = dmc_sample_start_addr;
				dmc_bytes_remaining = dmc_sample_len;
			}
			else if (dmc_irq_enabled)
			{
				set_dmc_irq(true);
			}
		}
	}

	// --------------------------------------------------
	// Frame IRQ delayed behind DMC DMA.
	//
	// Interrupt flag latency tests require frame IRQ publication to wait while
	// DMC loading/abort handling is active. Once the DMC path completes, arm
	// the normal delayed frame IRQ publication.
	// --------------------------------------------------
	if (delayed_frame_irq_after_dmc)
	{
		delayed_frame_irq_after_dmc = false;
		//delayed_frame_irq = 2;
		delayed_frame_irq = apu_clk1_is_high ? 2 : 1;
	}
}

void nesapu_device::set_frame_irq(bool s)
{
	if (!s) {
		// Do not clear frame_irq immediately here.
		//
		// frame_irq is the $4015-visible flag, and the tests care about
		// whether a read clear lands on a put->get or get->put edge.
		//
		// Cancel any pending CPU IRQ-output publication, but leave the actual
		// $4015-visible flag clear to delayed_frame_irq_clear below.
		delayed_frame_irq = 0;
		delayed_frame_irq_after_dmc = false;

		// frame_irq_suppress_clear_cycle is a local countdown.
		// 0 means inactive.
		frame_irq_suppress_clear_cycle = 0;

		if (apu_clk1_is_high)
			delayed_frame_irq_clear = 1;	// put -> get clears now
		else
			delayed_frame_irq_clear = 2;	// get -> put does not clear yet

		return;
	}

	// delayed_frame_irq_clear is a local countdown.
	// 0 means inactive.
	delayed_frame_irq_clear = 0;

	// frame_irq_suppress_clear_cycle is a local countdown.
	// 0 means inactive.
	frame_irq_suppress_clear_cycle = 0;

	if (!frame_irq) {
		// Set the $4015-visible frame IRQ flag now.
		//
		// The actual CPU IRQ output is intentionally delayed and is published
		// later by delayed_frame_irq.
		frame_irq = true;

		if (!dmc_loading_sample_byte) {
			// This is armed during clock_frame_counter(), before the countdown
			// service point in tick(), so add one extra count:
			delayed_frame_irq = apu_clk1_is_high ? 3 : 4;
			delayed_frame_irq_after_dmc = false;
		}
		else {
			// DMC is loading the sample byte. Do not publish the CPU IRQ output
			// yet. dmc_read() or the DMC abort paths will release this through
			// delayed_frame_irq once the DMC read/loading path completes.
			delayed_frame_irq = 0;
			delayed_frame_irq_after_dmc = true;
		}
	}
}

void nesapu_device::set_dmc_irq(bool s)
{
	dmc_irq = s;

	if (s)
	{
		// Publish DMC IRQ to the CPU after the core's IRQ-output delay.
		delayed_dmc_irq = 2;
	}
	else
	{
		// Clearing DMC IRQ is immediate from the APU side.
		delayed_dmc_irq = 0;

		// Keep the IRQ line asserted if frame IRQ is still active.
		update_irq_output();
	}
}

void nesapu_device::update_irq_output()
{
	// frame_irq is only the $4015-visible status flag.
	// frame_irq_output is the actual frame IRQ contribution to the CPU IRQ line.
	m_irq_handler(dmc_irq || (frame_irq_output && !inhibit_frame_irq));
}

void nesapu_device::set_frame_irq_flag_only()
{
	frame_irq = true;
	frame_irq_output = false;

	// No CPU IRQ-output publication should be pending from this path.
	delayed_frame_irq = 0;
	delayed_frame_irq_after_dmc = false;

	// delayed_frame_irq_clear is a local countdown.
	// 0 means inactive.
	delayed_frame_irq_clear = 0;

	update_irq_output();
}

// Read status register at $4015
u8 nesapu_device::status_r()
{
	uint8_t res =
		(dmc_irq                   		 << 7) |
		(frame_irq                 		 << 6) |
		(m_maincpu6502->get_data_bus() & 0x20) |
		((dmc_bytes_remaining > 0) 		 << 4) |
		((noise_len_cnt > 0)	 		 << 3) |
		((tri_len_cnt > 0) 				 << 2) |
		((m_APU.pulse[1].len_cnt > 0) 	 << 1) |
		((m_APU.pulse[0].len_cnt > 0)	 << 0);

	// Reading $4015 at specific intervals after the IRQ flag is set should
	// not clear the frame IRQ flag yet.
	//   0  = clear is allowed
	//   >0 = reads may see bit 6, but must not clear the flag yet
	if (frame_irq)
		set_frame_irq(false);

	return res;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------
void nesapu_device::push_out_sample(sound_stream::sample_t sample)
{
	const uint32_t next_w = (m_out_fifo_w + 1) % OUT_FIFO_SIZE;

	if (next_w == m_out_fifo_r) {
		 ++m_audio_fifo_overflows;
		m_out_fifo_r = (m_out_fifo_r + 1) % OUT_FIFO_SIZE;
	}

	m_out_fifo[m_out_fifo_w] = sample;
	m_out_fifo_w = next_w;
	m_last_out_sample = sample;
}

bool nesapu_device::pop_out_sample(sound_stream::sample_t &sample)
{
	if (m_out_fifo_r == m_out_fifo_w) {
		++m_audio_fifo_underflows;
		return false;
	}

	sample = m_out_fifo[m_out_fifo_r];
	m_out_fifo_r = (m_out_fifo_r + 1) % OUT_FIFO_SIZE;
	return true;
}

void nesapu_device::accumulate_output_sample(sound_stream::sample_t level)
{
	// Assert that m_resample_step is valid
    assert(m_resample_step > 0); // Ensures resample step is correctly initialized
	
	constexpr uint64_t ONE = 1ULL << 32;

	uint64_t remaining = ONE;

	while (remaining > 0)
	{
		//uint64_t to_boundary = m_resample_step - m_resample_phase;
		uint64_t to_boundary =
			(m_resample_phase < m_resample_step)
				? (m_resample_step - m_resample_phase)
				: 0;
		uint64_t slice = (remaining < to_boundary) ? remaining : to_boundary;

		m_output_accum += level * sound_stream::sample_t(double(slice) / double(ONE));
		m_resample_phase += slice;
		remaining -= slice;

		if (m_resample_phase >= m_resample_step)
		{
			sound_stream::sample_t sample =
				m_output_accum / sound_stream::sample_t(double(m_resample_step) / double(ONE));

			sample = apply_analog_filter(sample);
			push_out_sample(sample);

			m_resample_phase -= m_resample_step;
			m_output_accum = 0.0;
		}
	}
}

sound_stream::sample_t nesapu_device::apply_analog_filter(sound_stream::sample_t in)
{
	const double sr = double(m_stream->sample_rate());

	assert(sr > 0.0);

	// NES front-loader-ish post-mixer filtering:
	//   1st-order high-pass around 90 Hz
	//   1st-order high-pass around 440 Hz
	//   1st-order low-pass around 14 kHz
	const sound_stream::sample_t a_hp90 =
		sound_stream::sample_t(std::exp(-2.0 * M_PI * 90.0 / sr));

	const sound_stream::sample_t a_hp440 =
		sound_stream::sample_t(std::exp(-2.0 * M_PI * 440.0 / sr));

	const sound_stream::sample_t a_lp14k =
		sound_stream::sample_t(1.0 - std::exp(-2.0 * M_PI * 14000.0 / sr));

	// HPF 90 Hz
	const sound_stream::sample_t hp90 =
		a_hp90 * (m_hp90_prev_out + in - m_hp90_prev_in);

	m_hp90_prev_in = in;
	m_hp90_prev_out = hp90;

	// HPF 440 Hz
	const sound_stream::sample_t hp440 =
		a_hp440 * (m_hp440_prev_out + hp90 - m_hp440_prev_in);

	m_hp440_prev_in = hp90;
	m_hp440_prev_out = hp440;

	// LPF 14 kHz
	m_lp14k_prev_out =
		m_lp14k_prev_out + a_lp14k * (hp440 - m_lp14k_prev_out);

	return m_lp14k_prev_out;
}

sound_stream::sample_t nesapu_device::calc_current_output()
{
	const int pulse_sum =
		m_APU.pulse[0].output_level + m_APU.pulse[1].output_level;

	// Hardware does not mute triangle periods 0/1.
	// The raw sequencer runs ultrasonically; the analog output is effectively
	// a center-ish DC level after filtering.  Use this only for mixer output,
	// not for the triangle sequencer state.
	const int tri_mixer_level =
		(tri_len_cnt > 0 && tri_lin_cnt > 0 && tri_period <= 1)
			? 7
			: tri_output_level;

	return
		m_square_lut[pulse_sum] +
		m_tnd_lut[tri_mixer_level][noise_output_level][dmc_counter];
}

void nesapu_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		sound_stream::sample_t sample;

		if (!pop_out_sample(sample))
			sample = m_last_out_sample;

		stream.put(0, sampindex, sample);
	}
}


