// license:GPL-2.0+
// copyright-holders:Matthew Conte,Brian H. Gilbert
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

   - Based on Matthew Conte's Nofrendo/Nosefart core and redesigned to
     use MAME system calls and to enable multiple APUs.

   Two implementations live here.  The APU inside an RP2A03 device (the NES and
   the machines that use that device) is cycle-accurate: the frame counter,
   length counters, DMC memory reader and the IRQs follow NESdev and the
   hardware-verified test ROMs, timed by the CPU's own cycle count.  The
   original Nofrendo-based channel code is still used where the APU is not
   inside an RP2A03 (nes_sh6578 and vgmplay).

   Current known limitations (original channel code only)

   * The envelope, sweep, linear and length counter timing comes from tables
     built for the 4-step frame sequence, so the 5-step mode of $4017 is
     recorded but does not change those rates.
   * The triangle's linear counter starts a short, fixed time after a $400B
     write (write_latency), a delay left over from the sound queue this core
     was first written with, not a measured one.
   * The DMC reads its sample bytes directly and does not stall the CPU.

 *****************************************************************************

   BUGFIXES:

   - Various bugs concerning the DPCM channel fixed. (Oliver Achten)
   - Fixed $4015 read behaviour. (Oliver Achten)

 *****************************************************************************/

#include "emu.h"
#include "nes_apu.h"

// LFSR reload values giving the DMC periods (NTSC as TriCNES; PAL derived with the same feedback rule). Checked by
// simulating dmc_step_lfsr() from each seed (one step per APU cycle, the reload step included): all 32 give NESdev's
// published DMC periods in CPU cycles (APU DMC, rate table): NTSC 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142,
// 128, 106, 84, 72, 54; PAL 398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118, 98, 78, 66, 50
static const u16 dmc_lfsr_reload[2][16] = {
	{ 0x173, 0x08a, 0x143, 0x0db, 0x1ee, 0x03f, 0x07d, 0x1d1, 0x009, 0x0dc, 0x0f1, 0x0f9, 0x08d, 0x189, 0x18e, 0x156 },
	{ 0x1d7, 0x1cc, 0x136, 0x185, 0x10f, 0x1bc, 0x074, 0x099, 0x0a4, 0x18b, 0x1e7, 0x167, 0x0a8, 0x071, 0x1b1, 0x1d5 } };

DEFINE_DEVICE_TYPE(NES_APU,  nesapu_device,  "nesapu",  "RP2A0X APU")
DEFINE_DEVICE_TYPE(APU_2A03, apu2a03_device, "apu2a03", "RP2A03 APU")

nesapu_device::nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_is_pal(0)
	, m_samps_per_sync(0)
	, m_stream(nullptr)
	, m_irq_handler(*this)
	, m_mem_read_cb(*this, 0x00)
	, m_dmc_dma_cb(*this)
	, m_dmc_engine(false)
	, m_dmc_load_timer(nullptr)
	, m_frame_timer(nullptr)
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


void nesapu_device::device_reset()
{
	write(0x15, 0x00);
	if (m_dmc_engine)
	{
		m_dmc_buf_full = false;
		m_dmc_fetching = false;
		m_dmc_bits = 8;
		m_dmc_shift = 0;
		m_dmc_silent = true;
		// the rate timer is stepped by the CPU (dmc_cycle_end); power-on state as TriCNES
		m_dmc_lfsr = 0;
		m_dmc_status = m_dmc_status_next = false;
		m_dmc_status_delay = m_dmc_load_delay = 0;
		m_dmc_quiet = true;
		m_dmc_impl_abort = m_dmc_impl_abort_set = false;
		m_dmc_unexpected = m_dmc_ended = m_dmc_fetched = false;
	}
	// NESdev "CPU power up state" (APU, after reset): the triangle's phase is 0 (output 15) and $4011 keeps only bit 0
	m_APU.tri.adder = 0;
	m_t.step = 0;
	m_APU.dpcm.regs[1] &= 1;
	m_APU.dpcm.vol &= 1;

	if (m_acc)
		acc_reset();
	else if (m_APU.frame_irq_enabled)
		m_frame_timer->adjust(m_frame_period, 0, m_frame_period);
}

void nesapu_device::device_clock_changed()
{
	calculate_rates();
	m_is_pal = m_clock == PAL_APU_CLOCK;
}

void nesapu_device::calculate_rates()
{
	m_samps_per_sync = m_frame_clocks / 4; // the tables are for the 4-step sequence (see the limitations at the top)

	// initialize sample times in terms of vsyncs
	for (int i = 0; i < SYNCS_MAX1; i++)
	{
		m_vbl_times[i] = vbl_length[i] * m_samps_per_sync / 2;
		m_sync_times1[i] = m_samps_per_sync * (i + 1);
	}

	for (int i = 0; i < SYNCS_MAX2; i++)
		m_sync_times2[i] = (m_samps_per_sync * i) >> 2;

	int rate = clock() / 4;

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = stream_alloc(0, 1, rate);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nesapu_device::device_start()
{
	m_frame_timer = timer_alloc(FUNC(nesapu_device::frame_timer_cb), this);

	m_dmc_engine = !m_dmc_dma_cb.isunset();
	m_dmc_load_timer = timer_alloc(FUNC(nesapu_device::dmc_load_tick), this);

	m_acc = m_dmc_engine;
	m_fc_timer = timer_alloc(FUNC(nesapu_device::fc_step_tick), this);
	m_fc_reset_timer = timer_alloc(FUNC(nesapu_device::fc_reset_tick), this);
	m_fc_clear_timer = timer_alloc(FUNC(nesapu_device::fc_clear_tick), this);
	m_fc_mode5 = m_fc_inhibit = false;
	m_fc_step = 0;
	m_fc_base = attotime::zero;
	m_fc_value = 0;
	for (int i = 0; i < 4; i++) { m_halt_old[i] = false; m_halt_cycle[i] = ~u64(0); m_skip_clock[i] = ~u64(0); }
	save_item(NAME(m_fc_started)); save_item(NAME(m_halt_old)); save_item(NAME(m_halt_cycle)); save_item(NAME(m_skip_clock));
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_p[i].duty), i); save_item(NAME(m_p[i].step), i); save_item(NAME(m_p[i].vol), i); save_item(NAME(m_p[i].length), i);
		save_item(NAME(m_p[i].halt), i); save_item(NAME(m_p[i].constvol), i); save_item(NAME(m_p[i].enabled), i);
		save_item(NAME(m_p[i].env.start), i); save_item(NAME(m_p[i].env.divider), i); save_item(NAME(m_p[i].env.decay), i);
		save_item(NAME(m_p[i].sweep_en), i); save_item(NAME(m_p[i].sweep_neg), i); save_item(NAME(m_p[i].sweep_reload), i);
		save_item(NAME(m_p[i].sweep_p), i); save_item(NAME(m_p[i].sweep_shift), i); save_item(NAME(m_p[i].sweep_div), i);
		save_item(NAME(m_p[i].period), i); save_item(NAME(m_p[i].timer), i);
	}
	save_item(NAME(m_t.control)); save_item(NAME(m_t.reload_flag)); save_item(NAME(m_t.enabled)); save_item(NAME(m_t.reload));
	save_item(NAME(m_t.linear)); save_item(NAME(m_t.length)); save_item(NAME(m_t.step)); save_item(NAME(m_t.period)); save_item(NAME(m_t.timer));
	save_item(NAME(m_n.halt)); save_item(NAME(m_n.constvol)); save_item(NAME(m_n.mode)); save_item(NAME(m_n.enabled)); save_item(NAME(m_n.vol));
	save_item(NAME(m_n.period_idx)); save_item(NAME(m_n.length)); save_item(NAME(m_n.env.start)); save_item(NAME(m_n.env.divider));
	save_item(NAME(m_n.env.decay)); save_item(NAME(m_n.lfsr)); save_item(NAME(m_n.timer));
	save_item(NAME(m_fc_mode5)); save_item(NAME(m_fc_inhibit)); save_item(NAME(m_fc_step)); save_item(NAME(m_fc_base)); save_item(NAME(m_fc_value));
	m_dmc_addr = 0xc000; m_dmc_bytes = 0; m_dmc_buf = 0; m_dmc_buf_full = false; m_dmc_bits = 8;
	m_dmc_shift = 0; m_dmc_silent = true;
	save_item(NAME(m_dmc_addr));
	save_item(NAME(m_dmc_bytes));
	save_item(NAME(m_dmc_buf));
	save_item(NAME(m_dmc_buf_full));
	save_item(NAME(m_dmc_bits));
	save_item(NAME(m_dmc_shift));
	save_item(NAME(m_dmc_silent));
	m_dmc_fetching = false;
	save_item(NAME(m_dmc_fetching));
	m_dmc_load_pending = false;
	save_item(NAME(m_dmc_load_pending));
	save_item(NAME(m_dmc_lfsr));
	save_item(NAME(m_dmc_status));
	save_item(NAME(m_dmc_status_next));
	save_item(NAME(m_dmc_status_delay));
	save_item(NAME(m_dmc_load_delay));
	save_item(NAME(m_dmc_quiet));
	save_item(NAME(m_dmc_impl_abort));
	save_item(NAME(m_dmc_impl_abort_set));
	save_item(NAME(m_dmc_unexpected));
	save_item(NAME(m_dmc_ended));
	save_item(NAME(m_dmc_fetched));
	save_item(NAME(m_dmc_ended_addr));
	save_item(NAME(m_dmc_enable_cycle));
	m_frame_clocks = m_is_pal ? 33254 : 29830;
	m_frame_period = clocks_to_attotime(m_frame_clocks);
	m_APU.step_mode = 4;
	m_APU.frame_irq_enabled = true;

	calculate_rates();

	// calculate mixer output
	/*
	pulse channel output:

	         95.88
	-----------------------
	      8128
	----------------- + 100
	pulse 1 + pulse 2

	*/
	for (int i = 0; i < 31; i++)
	{
		sound_stream::sample_t pulse_out = (i == 0) ? 0.0 : 95.88 / ((8128.0 / i) + 100.0);
		m_square_lut[i] = pulse_out;
	}

	/*
	triangle, noise, DMC channel output:

	             159.79
	-------------------------------
	            1
	------------------------- + 100
	triangle   noise    dmc
	-------- + ----- + -----
	  8227     12241   22638

	*/
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

	/* register for save */
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_APU.squ[i].regs), i);
		save_item(NAME(m_APU.squ[i].vbl_length), i);
		save_item(NAME(m_APU.squ[i].freq), i);
		save_item(NAME(m_APU.squ[i].phaseacc), i);
		save_item(NAME(m_APU.squ[i].env_phase), i);
		save_item(NAME(m_APU.squ[i].sweep_phase), i);
		save_item(NAME(m_APU.squ[i].adder), i);
		save_item(NAME(m_APU.squ[i].env_vol), i);
		save_item(NAME(m_APU.squ[i].enabled), i);
		save_item(NAME(m_APU.squ[i].output), i);
	}

	save_item(NAME(m_APU.tri.regs));
	save_item(NAME(m_APU.tri.linear_length));
	save_item(NAME(m_APU.tri.linear_reload));
	save_item(NAME(m_APU.tri.vbl_length));
	save_item(NAME(m_APU.tri.write_latency));
	save_item(NAME(m_APU.tri.phaseacc));
	save_item(NAME(m_APU.tri.adder));
	save_item(NAME(m_APU.tri.counter_started));
	save_item(NAME(m_APU.tri.enabled));
	save_item(NAME(m_APU.tri.output));

	save_item(NAME(m_APU.noi.regs));
	save_item(NAME(m_APU.noi.lfsr));
	save_item(NAME(m_APU.noi.vbl_length));
	save_item(NAME(m_APU.noi.phaseacc));
	save_item(NAME(m_APU.noi.env_phase));
	save_item(NAME(m_APU.noi.env_vol));
	save_item(NAME(m_APU.noi.enabled));
	save_item(NAME(m_APU.noi.output));

	save_item(NAME(m_APU.dpcm.regs));
	save_item(NAME(m_APU.dpcm.address));
	save_item(NAME(m_APU.dpcm.length));
	save_item(NAME(m_APU.dpcm.bits_left));
	save_item(NAME(m_APU.dpcm.phaseacc));
	save_item(NAME(m_APU.dpcm.cur_byte));
	save_item(NAME(m_APU.dpcm.enabled));
	save_item(NAME(m_APU.dpcm.irq_occurred));
	save_item(NAME(m_APU.dpcm.vol));
	save_item(NAME(m_APU.dpcm.output));

	save_item(NAME(m_APU.step_mode));
	save_item(NAME(m_APU.frame_irq_enabled));
	save_item(NAME(m_APU.frame_irq_occurred));
	save_item(NAME(m_fc_clear_due));
}


TIMER_CALLBACK_MEMBER(nesapu_device::frame_timer_cb)
{
	m_stream->update();

	if (m_APU.step_mode == 4 && m_APU.frame_irq_enabled)
	{
		m_APU.frame_irq_occurred = true;
		m_irq_handler(true);
	}
}


/* DC decay is not a property of the channels: on the console it comes from the high-pass filters of the analog output
   stage after the mixer, which the machine models (nes.cpp), so the channels output their raw DAC levels */

/* OUTPUT SQUARE WAVE SAMPLE (VALUES FROM 0 to +15) */
void nesapu_device::apu_square(apu_t::square_t *chan)
{
	int env_delay;
	int sweep_delay;

	/* reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
	** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
	** reg2: 8 bits of freq
	** reg3: 0-2=high freq, 7-4=vbl length counter
	*/

	if (!chan->enabled)
	{
		chan->output = 0;
		return;
	}

	/* enveloping */
	env_delay = m_sync_times1[chan->regs[0] & 0x0f];

	/* decay is at a rate of (env_regs + 1) / 240 secs */
	chan->env_phase -= 4;
	while (chan->env_phase < 0)
	{
		chan->env_phase += env_delay;
		if (chan->regs[0] & 0x20)
			chan->env_vol = (chan->env_vol + 1) & 15;
		else if (chan->env_vol < 15)
			chan->env_vol++;
	}

	/* vbl length counter */
	if (chan->vbl_length > 0 && !(chan->regs[0] & 0x20))
		chan->vbl_length--;

	if (!chan->vbl_length)
	{
		chan->output = 0;
		return;
	}

	/* freqsweeps */
	if ((chan->regs[1] & 0x80) && (chan->regs[1] & 7))
	{
		sweep_delay = m_sync_times1[(chan->regs[1] >> 4) & 7];
		chan->sweep_phase -= 2;
		while (chan->sweep_phase < 0)
		{
			chan->sweep_phase += sweep_delay;
			if (chan->regs[1] & 8)
				chan->freq -= chan->freq >> (chan->regs[1] & 7);
			else
				chan->freq += chan->freq >> (chan->regs[1] & 7);
		}
	}

	if ((!(chan->regs[1] & 8) && (chan->freq >> 16) > freq_limit[chan->regs[1] & 7])
			|| (chan->freq >> 16) < 4)
	{
		chan->output = 0;
		return;
	}

	chan->phaseacc -= 4;

	while (chan->phaseacc < 0)
	{
		chan->phaseacc += (chan->freq >> 16);
		chan->adder = (chan->adder + 1) & 0x0f;
	}

	if (chan->regs[0] & 0x10) /* fixed volume */
		chan->output = chan->regs[0] & 0x0f;
	else
		chan->output = 0x0f - chan->env_vol;

	chan->output *= BIT(duty_lut[chan->regs[0] >> 6], 7 - BIT(chan->adder, 1, 3));
}

/* OUTPUT TRIANGLE WAVE SAMPLE (VALUES FROM 0 to +15) */
void nesapu_device::apu_triangle(apu_t::triangle_t *chan)
{
	/* reg0: 7=holdnote, 6-0=linear length counter
	** reg2: low 8 bits of frequency
	** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
	*/

	if (!chan->enabled)
		return;

	bool not_held = !BIT(chan->regs[0], 7);

	if (!chan->counter_started && not_held)
	{
		if (chan->write_latency)
			chan->write_latency--;
		if (!chan->write_latency)
			chan->counter_started = true;
	}

	if (chan->counter_started)
	{
		if (chan->linear_reload)
			chan->linear_length = m_sync_times2[chan->regs[0] & 0x7f];
		else if (chan->linear_length > 0)
			chan->linear_length--;

		if (not_held)
			chan->linear_reload = false;

		if (chan->vbl_length && not_held)
			chan->vbl_length--;
	}

	if (!(chan->linear_length && chan->vbl_length))
		return;

	int freq = ((chan->regs[3] & 7) << 8) + chan->regs[2] + 1;

// Legacy mixer only: this halts the triangle at ultrasonic timer values. On hardware the sequencer keeps running (timer
// 0 is how Mega Man 2 silences the triangle, with a pop when an audible pitch resumes, easily heard in Crash Man's stage),
// as it does in the cycle-accurate path (m_acc) the NES uses. Halting avoids that pop and the aliasing of the ultrasonic
// output at this sample rate, and drivers that do not model the output filters still rely on it.
	if (freq < 2)
		return;

	chan->phaseacc -= 4;
	while (chan->phaseacc < 0)
	{
		chan->phaseacc += freq;
		chan->adder++;

		chan->output = chan->adder & 0xf;
		if (!BIT(chan->adder, 4))
			chan->output ^= 0xf;
	}
}

/* OUTPUT NOISE WAVE SAMPLE (VALUES FROM 0 to +15) */
void nesapu_device::apu_noise(apu_t::noise_t *chan)
{
	int freq, env_delay;

	/* reg0: 0-3=volume, 4=envelope, 5=hold
	** reg2: 7=small(93 byte) sample,3-0=freq lookup
	** reg3: 7-4=vbl length counter
	*/

	if (!chan->enabled)
	{
		chan->output = 0;
		return;
	}

	/* enveloping */
	env_delay = m_sync_times1[chan->regs[0] & 0x0f];

	/* decay is at a rate of (env_regs + 1) / 240 secs */
	chan->env_phase -= 4;
	while (chan->env_phase < 0)
	{
		chan->env_phase += env_delay;
		if (chan->regs[0] & 0x20)
			chan->env_vol = (chan->env_vol + 1) & 15;
		else if (chan->env_vol < 15)
			chan->env_vol++;
	}

	/* length counter */
	if (!(chan->regs[0] & 0x20))
	{
		if (chan->vbl_length > 0)
			chan->vbl_length--;
	}

	if (!chan->vbl_length)
	{
		chan->output = 0;
		return;
	}

	freq = noise_freq[m_is_pal][chan->regs[2] & 0x0f];
	chan->phaseacc -= 4;
	while (chan->phaseacc < 0)
	{
		chan->phaseacc += freq;
		update_lfsr(*chan);
	}

	if (BIT(chan->lfsr, 0)) /* silence channel */
	{
		chan->output = 0;
		return;
	}

	if (chan->regs[0] & 0x10) /* fixed volume */
		chan->output = chan->regs[0] & 0x0f;
	else
		chan->output = 0x0f - chan->env_vol;
}

void nesapu_device::update_lfsr(apu_t::noise_t &chan)
{
	chan.lfsr |= (BIT(chan.lfsr, 0) ^ BIT(chan.lfsr, (chan.regs[2] & 0x80) ? 6 : 1)) << 15;
	chan.lfsr >>= 1;
}

void apu2a03_device::update_lfsr(apu_t::noise_t &chan)
{
	chan.lfsr |= (BIT(chan.lfsr, 0) ^ BIT(chan.lfsr, 1)) << 15;
	chan.lfsr >>= 1;
}

/* RESET DPCM PARAMETERS */
static inline void apu_dpcmreset(apu_t::dpcm_t *chan)
{
	chan->address = 0xc000 + u16(chan->regs[2] << 6);
	chan->length = u16(chan->regs[3] << 4) + 1;
	chan->bits_left = chan->length << 3;
	chan->enabled = true; /* Fixed * Proper DPCM channel ENABLE/DISABLE flag behaviour*/
}

/* OUTPUT DPCM WAVE SAMPLE (VALUES FROM 0 to +127) */
void nesapu_device::apu_dpcm(apu_t::dpcm_t *chan)
{
	int freq, bit_pos;

	/* reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
	** reg1: output dc level, 7 bits unsigned
	** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
	** reg3: length, (value * 16) + 1
	*/

	if (m_dmc_engine)
	{
		// the output unit is clocked by the rate timer (dmc_step_sample); the stream only plays the output level
		chan->output = u8(chan->vol);
		return;
	}

	if (chan->enabled)
	{
		freq = dpcm_clocks[m_is_pal][chan->regs[0] & 0x0f];
		chan->phaseacc -= 4;

		while (chan->phaseacc < 0)
		{
			chan->phaseacc += freq;

			if (!chan->length)
			{
				chan->enabled = false; /* Fixed * Proper DPCM channel ENABLE/DISABLE flag behaviour*/
				if (chan->regs[0] & 0x40)
					apu_dpcmreset(chan);
				else
				{
					if (chan->regs[0] & 0x80) /* IRQ Generator */
					{
						chan->irq_occurred = true;
						m_irq_handler(true);
					}
					break;
				}
			}

			chan->bits_left--;
			bit_pos = 7 - (chan->bits_left & 7);
			if (7 == bit_pos)
			{
				chan->cur_byte = m_mem_read_cb(chan->address);
				chan->address++;
				chan->length--;
			}

			if ((chan->cur_byte & (1 << bit_pos)) && (chan->vol <= 125))
//              chan->regs[1] += 2;
				chan->vol += 2; /* FIXED * DPCM channel only uses the upper 6 bits of the DAC */
			else if (chan->vol >= 2)
//              chan->regs[1] -= 2;
				chan->vol -= 2;
		}
	}

	chan->output = (u8)(chan->vol);
}

// DMC memory reader: ask the CPU's DMA unit for the next byte (the CPU is halted while it fetches it)
void nesapu_device::dmc_fetch(bool load)
{
	// NESdev DMA: a "load" fetch follows a $4015 write, a "reload" fetch follows the output unit emptying the buffer;
	// the CPU's DMA unit schedules them differently, so it is told which one this is (offset 1 = load)
	m_dmc_fetching = true;
	m_dmc_dma_cb(load ? 1 : 0, m_dmc_addr);
}

void nesapu_device::dmc_dma_done(u8 data)
{
	const u16 fetched = m_dmc_addr;
	const bool unexpected = m_dmc_unexpected;
	m_dmc_unexpected = false;
	m_dmc_fetching = false;
	m_dmc_fetched = true;
	m_dmc_buf = data;
	m_dmc_buf_full = true;
	m_dmc_addr = (m_dmc_addr == 0xffff) ? 0x8000 : (m_dmc_addr + 1);
	m_dmc_quiet = false;

	if (m_dmc_bytes)
		m_dmc_bytes--;
	if (!m_dmc_bytes)
	{
		if (m_APU.dpcm.regs[0] & 0x40)
		{
			m_dmc_addr = 0xc000 + u16(m_APU.dpcm.regs[2] << 6);
			m_dmc_bytes = u16(m_APU.dpcm.regs[3] << 4) + 1;
		}
		else
		{
			// the sample ended: the DMC turns itself off (TriCNES DMCDMA_Get)
			m_dmc_status = false;
			// implicit abort (NESdev "DMA", Bugs): playback that stops in the APU cycle before a reload DMA would be
			// scheduled leaves a DMA that starts and is aborted after one cycle. What counts is when the sample ends
			// (this read), not when $4015 was written: the rate timer is one step from its last (17, 8, 4, 2, 1). On a
			// 2A03 the read follows the write at a fixed distance (a write on the 8th or 9th cycle before); on a 2A07
			// the load DMA waits for an opcode fetch and can be a cycle later ("Implicit DMA Abort")
			if (m_dmc_lfsr == 2)
				m_dmc_impl_abort_set = true;
			if (!unexpected)
			{
				m_dmc_ended = true;
				m_dmc_ended_addr = fetched;
			}
			if (m_APU.dpcm.regs[0] & 0x80)
			{
				m_APU.dpcm.irq_occurred = true;
				m_irq_handler(true);
			}
		}
	}
}

void nesapu_device::dmc_step_lfsr()
{
	if (m_dmc_lfsr == 1)
	{
		m_dmc_lfsr = dmc_lfsr_reload[m_is_pal ? 1 : 0][m_APU.dpcm.regs[0] & 0x0f];
		dmc_step_sample();
	}
	else
	{
		const bool b0 = BIT(m_dmc_lfsr, 0), b4 = BIT(m_dmc_lfsr, 4);
		const bool a = b0 && b4;
		const bool n = !(b0 || b4 || (m_dmc_lfsr == 0));
		m_dmc_lfsr = (m_dmc_lfsr >> 1) | ((!(a || n)) ? 0x100 : 0);
	}
}

// The output unit, clocked by the rate timer (NESdev "APU DMC", Output unit): unless the silence flag is set the output
// level goes up by 2 when bit 0 of the shift register is 1 and down by 2 when it is 0, and stays where it is when that
// would take it outside 0-127; the shift register is clocked right; the bits-remaining counter is decremented and, when
// it reaches zero, a new output cycle starts: the counter is loaded with 8 and, if the sample buffer is empty, the
// silence flag is set, otherwise it is cleared and the sample buffer is emptied into the shift register. At that byte
// boundary, with sample bytes left (or an implicit abort armed) the reader asks for the next byte, otherwise the DMC
// goes quiet.
void nesapu_device::dmc_step_sample()
{
	if (!m_dmc_silent)
	{
		const u8 level = m_APU.dpcm.vol;
		u8 next = level;
		if (m_dmc_shift & 1)
		{
			if (level <= 125)
				next = level + 2;
		}
		else if (level >= 2)
			next = level - 2;
		if (next != level)
		{
			m_stream->update();   // the sound up to now is played at the old level
			m_APU.dpcm.vol = next;
		}
	}
	m_dmc_shift >>= 1;

	if (--m_dmc_bits > 0)
		return;
	m_dmc_bits = 8;
	m_dmc_silent = !m_dmc_buf_full;
	if (m_dmc_buf_full)
		m_dmc_shift = m_dmc_buf;
	m_dmc_buf_full = false;
	if (m_dmc_bytes || m_dmc_impl_abort_set)
	{
		m_dmc_fetching = true;
		// on the later chips a reload DMA can follow a fetch made in this same cycle (a looping 1-byte sample)
		m_dmc_dma_cb((m_dmc_late_rev && m_dmc_fetched) ? 2 : 0, m_dmc_addr);
		if (m_dmc_impl_abort_set)
		{
			m_dmc_impl_abort = true;
			m_dmc_impl_abort_set = false;
		}
		m_dmc_quiet = false;
	}
	else if (m_dmc_late_rev && m_dmc_ended)
	{
		// NESdev "DMA", Bugs: on the RP2A03H and the RP2A03G made from 1990 on, a sample that ends (the fetch of the
		// only byte of a non-looping 1-byte sample) on the APU cycle where a reload DMA would be scheduled gets that
		// reload DMA anyway, from the same address; the byte goes into the sample buffer and is played after the first
		m_dmc_unexpected = true;
		m_dmc_fetching = true;
		m_dmc_addr = m_dmc_ended_addr;
		m_dmc_dma_cb(2, m_dmc_addr);   // 2: not held off by the fetch that has just been made
		m_dmc_quiet = false;
	}
	else
		m_dmc_quiet = true;
}

// The APU's part of a CPU cycle, after the CPU's access (TriCNES _EmulateAPU): the rate timer steps at the end of a get
// cycle, a pending load DMA counts down at the end of a put cycle, and a $4015 write's DMC status lands when its
// delay runs out (a disable then also clears the bytes remaining)
void nesapu_device::dmc_cycle_end(bool get)
{
	if (get)
		dmc_step_lfsr();
	else if (m_dmc_load_delay && (--m_dmc_load_delay == 0))
	{
		m_dmc_fetching = true;
		m_dmc_dma_cb(1, m_dmc_addr);
		m_dmc_quiet = false;
	}
	if (m_dmc_status_delay && (--m_dmc_status_delay == 0))
	{
		m_dmc_status = m_dmc_status_next;
		if (!m_dmc_status)
			m_dmc_bytes = 0;
	}
	m_dmc_ended = m_dmc_fetched = false;   // only a byte boundary in the same CPU cycle sees them
}

TIMER_CALLBACK_MEMBER(nesapu_device::dmc_load_tick)
{
	if (!m_dmc_buf_full && !m_dmc_fetching && m_dmc_bytes)
		dmc_fetch(true);
}

//**************************************************************************
//  cycle-accurate APU core (2A03)
//**************************************************************************

namespace {

const u8 acc_length_table[32] = {
	10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

const u8 acc_duty_table[4][8] = {
	{ 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 0, 0, 0 },
	{ 1, 0, 0, 1, 1, 1, 1, 1 } };

// noise timer periods in CPU cycles
const u16 acc_noise_period[2][16] = {
	{ 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 },
	{ 4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708, 944, 1890, 3778 } };

// frame counter step positions in CPU cycles since the sequence (re)started: NTSC/Dendy and PAL, 4-step and 5-step
const u32 acc_fc_cycles[2][2][6] = {
	{ { 7457, 14913, 22371, 29828, 29829, 29830 }, { 7457, 14913, 22371, 29829, 37281, 37282 } },
	{ { 8313, 16627, 24939, 33252, 33253, 33254 }, { 8313, 16627, 24939, 33253, 41565, 41566 } } };

// what each step does: quarter frame, half frame, frame IRQ flag (4-step only), end of sequence
enum { FC_Q = 1, FC_H = 2, FC_I = 4, FC_END = 8 };
const u8 acc_fc_events[2][6] = {
	{ FC_Q, FC_Q | FC_H, FC_Q, FC_I, FC_Q | FC_H | FC_I, FC_I | FC_END },
	{ FC_Q, FC_Q | FC_H, FC_Q, 0, FC_Q | FC_H, FC_END } };

} // anonymous namespace

// Converts an emulated time to the CPU cycle it falls on. Emulated time can sit a few attoseconds below an exact
// cycle boundary, so plain truncation sometimes lands on the previous cycle; that flipped the parity that decides the
// $4017 restart delay (3 or 4 cycles) and the same-cycle length-clock races. Round to the nearest cycle instead.
u64 nesapu_device::cycle_at(const attotime &t) const
{
	return attotime_to_clocks(t + clocks_to_attotime(1) / 2);
}

void nesapu_device::acc_reset()
{
	// a reset only clears $4015 (done by the caller) and rewrites $4017; the channel registers keep their values
	// (power-on state is the default-initialized one)
	m_APU.frame_irq_occurred = false;
	if (!m_APU.dpcm.irq_occurred)
		m_irq_handler(false);

	// on reset the frame counter acts as if the last $4017 value had been written again; the earliest 2A03 does not
	// restart it (NESdev "CPU variants": "APU Frame Counter not restarted on reset"), but still has to be started at
	// power-on
	m_fc_inhibit = BIT(m_fc_value, 6);
	if (fc_restarts_on_reset() || !m_fc_started)
		m_fc_reset_timer->adjust(attotime::zero);
	m_fc_started = true;
}

void nesapu_device::fc_schedule()
{
	const u32 when = acc_fc_cycles[m_is_pal ? 1 : 0][m_fc_mode5 ? 1 : 0][m_fc_step];
	const attotime target = m_fc_base + clocks_to_attotime(when);
	const attotime now = machine().time();
	m_fc_timer->adjust((target > now) ? (target - now) : attotime::zero);
}

TIMER_CALLBACK_MEMBER(nesapu_device::fc_step_tick)
{
	m_stream->update();
	const u8 ev = acc_fc_events[m_fc_mode5 ? 1 : 0][m_fc_step];
	if (ev & FC_Q)
		acc_quarter();
	if (ev & FC_H)
		acc_half();
	if ((ev & FC_I) && m_fc_inhibit)
	{
		// with interrupts inhibited the flag still shows on the first two set cycles (29828 and 29829) and is gone on the
		// third; the IRQ line is never driven (AccuracyCoin "Frame Counter IRQ" tests I-M)
		m_APU.frame_irq_occurred = !(ev & FC_END);
	}
	else if (ev & FC_I)
	{
		// the sequencer setting the flag wins over a $4015-read clear due on the same cycle (AccuracyCoin "Frame Counter
		// IRQ" tests E-H); a clear due later still happens after the last set
		if (m_fc_clear_timer->enabled() && (m_fc_clear_due <= cycle_at(machine().time())))
			m_fc_clear_timer->adjust(attotime::never);
		m_APU.frame_irq_occurred = true;
		// The flag is set and drives IRQ on this cycle; the 2A03 samples IRQ when phi2 falls at its end and takes it from
		// the next cycle (NESdev talk: the frame IRQ takes effect at 29829)
		m_irq_handler(true);
	}
	if (ev & FC_END)
	{
		m_fc_base += clocks_to_attotime(acc_fc_cycles[m_is_pal ? 1 : 0][m_fc_mode5 ? 1 : 0][5]);
		m_fc_step = 0;
	}
	else
		m_fc_step++;
	fc_schedule();
}

TIMER_CALLBACK_MEMBER(nesapu_device::fc_reset_tick)
{
	m_stream->update();
	m_fc_mode5 = BIT(m_fc_value, 7);
	m_fc_base = machine().time();
	m_fc_step = 0;
	if (m_fc_mode5)
	{
		acc_quarter();
		acc_half();
	}
	fc_schedule();
}

void nesapu_device::acc_quarter()
{
	auto clock_env = [] (acc_env_t &env, u8 period, bool loop)
	{
		if (env.start)
		{
			env.start = false;
			env.decay = 15;
			env.divider = period;
		}
		else if (env.divider == 0)
		{
			env.divider = period;
			if (env.decay)
				env.decay--;
			else if (loop)
				env.decay = 15;
		}
		else
			env.divider--;
	};
	clock_env(m_p[0].env, m_p[0].vol, m_p[0].halt);
	clock_env(m_p[1].env, m_p[1].vol, m_p[1].halt);
	clock_env(m_n.env, m_n.vol, m_n.halt);

	if (m_t.reload_flag)
		m_t.linear = m_t.reload;
	else if (m_t.linear)
		m_t.linear--;
	if (!m_t.control)
		m_t.reload_flag = false;
}

u16 nesapu_device::acc_sweep_target(int i) const
{
	const acc_pulse_t &p = m_p[i];
	const u16 change = p.period >> p.sweep_shift;
	if (p.sweep_neg)
	{
		// pulse 1 negates with ones' complement; a negative sum is clamped to zero (NESdev "APU Sweep"), which is what
		// pulse 1 gets with a shift count of 0 ($08, the usual way to switch the sweep unit's muting off)
		const int target = int(p.period) - int(change) - ((i == 0) ? 1 : 0);
		return (target < 0) ? 0 : u16(target);
	}
	return p.period + change;
}

u8 &nesapu_device::acc_length(int ch)
{
	return (ch < 2) ? m_p[ch].length : (ch == 2) ? m_t.length : m_n.length;
}

bool &nesapu_device::acc_halt(int ch)
{
	return (ch < 2) ? m_p[ch].halt : (ch == 2) ? m_t.control : m_n.halt;
}

void nesapu_device::acc_halt_write(int ch, bool halt)
{
	m_halt_old[ch] = acc_halt(ch);
	m_halt_cycle[ch] = cycle_at(machine().time());
	acc_halt(ch) = halt;
}

// is the frame sequencer's next event a half-frame (length) clock on the given CPU cycle?
bool nesapu_device::half_clock_due(u64 cycle) const
{
	const u32 when = acc_fc_cycles[m_is_pal ? 1 : 0][m_fc_mode5 ? 1 : 0][m_fc_step];
	return (acc_fc_events[m_fc_mode5 ? 1 : 0][m_fc_step] & FC_H) && (cycle_at(m_fc_base) + when == cycle);
}

// blargg's length reload/halt timing tests (PAL 10/11): in this CPU-cycle numbering a register write collides with
// a length clock when it is made on the cycle before the clock; a write on the clock's own cycle comes after it.
//  - reload one cycle before a clock: ignored if the counter is non-zero; if it is zero it loads and that clock
//    does not decrement it
//  - reload on the clock's cycle: loads normally and is not decremented by that clock (whether or not the clock's
//    timer has fired yet)
void nesapu_device::acc_length_load(int ch, u8 value)
{
	const u64 cycle = cycle_at(machine().time());
	if (half_clock_due(cycle + 1))
	{
		if (acc_length(ch))
			return;
		acc_length(ch) = acc_length_table[value >> 3];
		m_skip_clock[ch] = cycle + 1;
		return;
	}
	acc_length(ch) = acc_length_table[value >> 3];
	if (half_clock_due(cycle))
		m_skip_clock[ch] = cycle;
}

void nesapu_device::acc_half()
{
	const u64 cycle = cycle_at(machine().time());
	for (int ch = 0; ch < 4; ch++)
	{
		u8 &len = acc_length(ch);
		// a halt write on this clock's cycle or the one before does not affect this clock (PAL 10.len_halt_timing)
		const bool recent = (m_halt_cycle[ch] == cycle) || (m_halt_cycle[ch] + 1 == cycle);
		const bool halt = recent ? m_halt_old[ch] : acc_halt(ch);
		if (len && !halt && m_skip_clock[ch] != cycle)
			len--;
	}

	for (int i = 0; i < 2; i++)
	{
		acc_pulse_t &p = m_p[i];

		const u16 target = acc_sweep_target(i);
		const bool muted = (p.period < 8) || (target > 0x7ff);
		if (p.sweep_div == 0 && p.sweep_en && p.sweep_shift && !muted)
			p.period = target;
		if (p.sweep_div == 0 || p.sweep_reload)
		{
			p.sweep_div = p.sweep_p;
			p.sweep_reload = false;
		}
		else
			p.sweep_div--;
	}
}

u8 nesapu_device::acc_pulse_out(int i) const
{
	const acc_pulse_t &p = m_p[i];
	if (!p.length || p.period < 8 || acc_sweep_target(i) > 0x7ff)
		return 0;
	if (!acc_duty_table[p.duty][p.step])
		return 0;
	return p.constvol ? p.vol : p.env.decay;
}

void nesapu_device::acc_advance(int cycles)
{
	for (auto &p : m_p)
	{
		p.timer -= cycles;
		while (p.timer <= 0)
		{
			p.timer += (p.period + 1) * 2;      // pulse timers count APU cycles
			p.step = (p.step + 1) & 7;
		}
	}

	m_t.timer -= cycles;
	while (m_t.timer <= 0)
	{
		m_t.timer += m_t.period + 1;           // the triangle timer counts CPU cycles
		if (m_t.linear && m_t.length)
			m_t.step = (m_t.step + 1) & 31;
	}

	m_n.timer -= cycles;
	while (m_n.timer <= 0)
	{
		m_n.timer += (!m_is_pal && (m_n.period_idx == 15)) ? noise_longest_period() : acc_noise_period[m_is_pal ? 1 : 0][m_n.period_idx];
		const u16 feedback = (m_n.lfsr ^ (m_n.lfsr >> (m_n.mode ? 6 : 1))) & 1;
		m_n.lfsr = (m_n.lfsr >> 1) | (feedback << 14);
	}
}

void nesapu_device::acc_enable(u8 value)
{
	m_p[0].enabled = BIT(value, 0);
	m_p[1].enabled = BIT(value, 1);
	m_t.enabled = BIT(value, 2);
	m_n.enabled = BIT(value, 3);
	if (!m_p[0].enabled) m_p[0].length = 0;
	if (!m_p[1].enabled) m_p[1].length = 0;
	if (!m_t.enabled) m_t.length = 0;
	if (!m_n.enabled) m_n.length = 0;
}

u8 nesapu_device::acc_status()
{
	u8 readval = 0;
	if (m_p[0].length) readval |= 0x01;
	if (m_p[1].length) readval |= 0x02;
	if (m_t.length) readval |= 0x04;
	if (m_n.length) readval |= 0x08;
	// bit 4: bytes remaining and the DMC enable as last written (TriCNES), so a disable shows at once although its
	// effect on the DMA side lands a few cycles later (AccuracyCoin "Delta Modulation Channel" tests 5 and C)
	if (m_dmc_bytes && (!m_dmc_engine || m_dmc_status_next)) readval |= 0x10;
	if (m_APU.frame_irq_occurred) readval |= 0x40;
	if (m_APU.dpcm.irq_occurred) readval |= 0x80;

	// NESdev/AccuracyCoin "Frame Counter IRQ": a $4015 read does not clear the frame IRQ flag immediately;
	// the clear takes effect on the next APU "get" cycle, so the flag (and the IRQ line) stays readable
	// through a get->put transition that happens on this same read cycle
	// (AccuracyCoin "Frame Counter IRQ" tests 6 and 7): the clear happens as the APU enters its next get cycle, so a
	// read on a put cycle sees the flag gone on the very next cycle, while a read on a get cycle still sees it set on
	// the following put cycle and only loses it on the cycle after that
	const u64 cycle = cycle_at(machine().time());
	const bool get = (cycle & 1) == 0;   // matches rp2a03_core_device::dma_get_cycle()'s fixed get/put convention
	m_fc_clear_due = cycle + (get ? 2 : 1);
	m_fc_clear_timer->adjust(clocks_to_attotime(get ? 2 : 1));
	return readval;
}

TIMER_CALLBACK_MEMBER(nesapu_device::fc_clear_tick)
{
	m_APU.frame_irq_occurred = false;
	if (!m_APU.dpcm.irq_occurred)
		m_irq_handler(false);
}

void nesapu_device::acc_write(offs_t offset, u8 value)
{
	switch (offset)
	{
	case 0x00: case 0x04:
	{
		acc_pulse_t &p = m_p[BIT(offset, 2)];
		p.duty = value >> 6;
		acc_halt_write(BIT(offset, 2), BIT(value, 5));
		p.constvol = BIT(value, 4);
		p.vol = value & 0x0f;
		break;
	}
	case 0x01: case 0x05:
	{
		acc_pulse_t &p = m_p[BIT(offset, 2)];
		p.sweep_en = BIT(value, 7);
		p.sweep_p = (value >> 4) & 7;
		p.sweep_neg = BIT(value, 3);
		p.sweep_shift = value & 7;
		p.sweep_reload = true;
		break;
	}
	case 0x02: case 0x06:
	{
		acc_pulse_t &p = m_p[BIT(offset, 2)];
		p.period = (p.period & 0x700) | value;
		break;
	}
	case 0x03: case 0x07:
	{
		acc_pulse_t &p = m_p[BIT(offset, 2)];
		p.period = (p.period & 0x0ff) | ((value & 7) << 8);
		if (p.enabled)
			acc_length_load(BIT(offset, 2), value);
		p.step = 0;
		p.env.start = true;
		break;
	}
	case 0x08:
		acc_halt_write(2, BIT(value, 7));
		m_t.reload = value & 0x7f;
		break;
	case 0x0a:
		m_t.period = (m_t.period & 0x700) | value;
		break;
	case 0x0b:
		m_t.period = (m_t.period & 0x0ff) | ((value & 7) << 8);
		if (m_t.enabled)
			acc_length_load(2, value);
		m_t.reload_flag = true;
		break;
	case 0x0c:
		acc_halt_write(3, BIT(value, 5));
		m_n.constvol = BIT(value, 4);
		m_n.vol = value & 0x0f;
		break;
	case 0x0e:
		m_n.mode = noise_has_mode() && BIT(value, 7);   // the earliest 2A03 has no mode flag (NESdev "APU Noise")
		m_n.period_idx = value & 0x0f;
		break;
	case 0x0f:
		if (m_n.enabled)
			acc_length_load(3, value);
		m_n.env.start = true;
		break;
	case apu_t::IRQCTRL:
	{
		// NESdev APU Frame Counter: the inhibit flag clears the frame IRQ flag right away; the sequencer restarts
		// 3 CPU cycles after a write that lands during an APU cycle, 4 after one between APU cycles
		m_fc_value = value;
		m_fc_inhibit = BIT(value, 6);
		if (m_fc_inhibit)
		{
			m_APU.frame_irq_occurred = false;
			if (!m_APU.dpcm.irq_occurred)
				m_irq_handler(false);
		}
		const u64 cycle = cycle_at(machine().time());
		m_fc_reset_timer->adjust(clocks_to_attotime((cycle & 1) ? 3 : 4));
		break;
	}
	default:
		break;
	}
}

/* WRITE REGISTER VALUE */
void nesapu_device::write(offs_t offset, u8 value)
{
	m_stream->update();

	if (m_acc)
	{
		if (offset < 0x10 || offset == apu_t::IRQCTRL)
		{
			acc_write(offset, value);
			return;
		}
		if (offset == apu_t::SMASK)
			acc_enable(value);
	}

	int chan = BIT(offset, 2);

	switch (offset)
	{
	/* squares */
	case apu_t::WRA0:
	case apu_t::WRB0:
		m_APU.squ[chan].regs[0] = value;
		break;

	case apu_t::WRA1:
	case apu_t::WRB1:
		m_APU.squ[chan].regs[1] = value;
		break;

	case apu_t::WRA2:
	case apu_t::WRB2:
		m_APU.squ[chan].regs[2] = value;
		if (m_APU.squ[chan].enabled)
			m_APU.squ[chan].freq = ((((m_APU.squ[chan].regs[3] & 7) << 8) + value) + 1) << 16;
		break;

	case apu_t::WRA3:
	case apu_t::WRB3:
		m_APU.squ[chan].regs[3] = value;

		if (m_APU.squ[chan].enabled)
		{
			m_APU.squ[chan].vbl_length = m_vbl_times[value >> 3];
			m_APU.squ[chan].env_vol = 0;
			m_APU.squ[chan].freq = ((((value & 7) << 8) + m_APU.squ[chan].regs[2]) + 1) << 16;
		}

		break;

	/* triangle */
	case apu_t::WRC0:
		m_APU.tri.regs[0] = value;

		if (m_APU.tri.enabled)
		{                                          /* ??? */
			if (!m_APU.tri.counter_started)
				m_APU.tri.linear_length = m_sync_times2[value & 0x7f];
		}

		break;

	case 0x4009:
		/* unused */
		m_APU.tri.regs[1] = value;
		break;

	case apu_t::WRC2:
		m_APU.tri.regs[2] = value;
		break;

	case apu_t::WRC3:
		m_APU.tri.regs[3] = value;

		/* the linear length counter starts counting a short time after this
		** write rather than at once, so a program that writes the frequency
		** registers first, then reg 0, does not see it run in between.  The
		** delay is a few output samples (3 * (1789772.727 / 44100) = ~122
		** cycles, about one scanline), based on samples/sync because the
		** clock runs faster than it used to; it is a limit of this core, not
		** a measured hardware delay (see the limitations at the top).
		*/
		m_APU.tri.write_latency = (m_samps_per_sync + 239) / 240;

		if (m_APU.tri.enabled)
		{
			m_APU.tri.counter_started = false;
			m_APU.tri.vbl_length = m_vbl_times[value >> 3];
			m_APU.tri.linear_length = m_sync_times2[m_APU.tri.regs[0] & 0x7f];
			m_APU.tri.linear_reload = true;
		}

		break;

	/* noise */
	case apu_t::WRD0:
		m_APU.noi.regs[0] = value;
		break;

	case 0x400D:
		/* unused */
		m_APU.noi.regs[1] = value;
		break;

	case apu_t::WRD2:
		m_APU.noi.regs[2] = value;
		break;

	case apu_t::WRD3:
		m_APU.noi.regs[3] = value;

		if (m_APU.noi.enabled)
		{
			m_APU.noi.vbl_length = m_vbl_times[value >> 3];
			m_APU.noi.env_vol = 0; /* reset envelope */
		}
		break;

	/* DMC */
	case apu_t::WRE0:
		m_APU.dpcm.regs[0] = value;
		if (!(value & 0x80))
		{
			m_APU.dpcm.irq_occurred = false;
			if (!frame_irq_asserted())
				m_irq_handler(false);
		}
		break;

	case apu_t::WRE1: /* 7-bit DAC */
		m_APU.dpcm.regs[1] = value & 0x7f;
		m_APU.dpcm.vol = m_APU.dpcm.regs[1];
		break;

	case apu_t::WRE2:
		m_APU.dpcm.regs[2] = value;
		//apu_dpcmreset(m_APU.dpcm);
		break;

	case apu_t::WRE3:
		m_APU.dpcm.regs[3] = value;
		break;

	case apu_t::IRQCTRL:
		if (value & 0x80)
			m_APU.step_mode = 5;
		else
			m_APU.step_mode = 4;

		m_APU.frame_irq_enabled = !BIT(value, 6);
		if (m_APU.frame_irq_enabled)
		{
			m_frame_timer->adjust(m_frame_period, 0, m_frame_period);
		}
		else
		{
			m_APU.frame_irq_occurred = false;
			if (!m_APU.dpcm.irq_occurred)
				m_irq_handler(false);
			m_frame_timer->reset();
		}

		break;

	case apu_t::SMASK:
		if (value & 0x01)
			m_APU.squ[0].enabled = true;
		else
		{
			m_APU.squ[0].enabled = false;
			m_APU.squ[0].vbl_length = 0;
		}

		if (value & 0x02)
			m_APU.squ[1].enabled = true;
		else
		{
			m_APU.squ[1].enabled = false;
			m_APU.squ[1].vbl_length = 0;
		}

		if (value & 0x04)
			m_APU.tri.enabled = true;
		else
		{
			m_APU.tri.enabled = false;
			m_APU.tri.vbl_length = 0;
			m_APU.tri.linear_length = 0;
			m_APU.tri.counter_started = false;
			m_APU.tri.write_latency = 0;
		}

		if (value & 0x08)
			m_APU.noi.enabled = true;
		else
		{
			m_APU.noi.enabled = false;
			m_APU.noi.vbl_length = 0;
		}

		if (m_dmc_engine)
		{
			// TriCNES $4015: the DMC status follows the write after 2 (put) or 3 (get) more CPU cycles. An enable with no
			// bytes left restarts the sample, and only if the DMC is quiet does a load DMA follow, 2 APU cycles later; with
			// a byte still playing, the next fetch is the timer's at the coming byte boundary.
			const bool put = (cycle_at(machine().time()) & 1) != 0;   // even = get, as rp2a03_core_device
			const bool enable = BIT(value, 4);
			m_dmc_status_next = enable;
			m_dmc_status_delay = put ? 3 : 4;
			if (enable && !m_dmc_bytes)
			{
				m_dmc_addr = 0xc000 + u16(m_APU.dpcm.regs[2] << 6);
				m_dmc_bytes = u16(m_APU.dpcm.regs[3] << 4) + 1;
				if (m_dmc_quiet)
					m_dmc_load_delay = 2;
			}
			const u16 reload = dmc_lfsr_reload[m_is_pal ? 1 : 0][m_APU.dpcm.regs[0] & 0x0f];
			// explicit abort: a disable made in the APU cycle where the rate timer fires lands later, so a DMA that is
			// just starting is not cut short ("Explicit DMA Abort"). The same holds for a disable written while a DMA
			// has been requested and its halt has not happened yet: on a 2A03 that is only the write on the request
			// cycle itself (covered by the timer test), on a 2A07, whose DMA waits for an opcode fetch, also the one
			// or two cycles after it. NESdev documents the first case (the DMA completes); the others are not
			// documented and follow from the same rule (VERIFY)
			if (!enable && (((m_dmc_lfsr == 1) && !put) || ((m_dmc_lfsr == reload) && put) || m_dmc_fetching))
				m_dmc_status_delay = put ? 5 : 6;
		}
		else if (value & 0x10)
		{
			/* only reset dpcm values if DMA is finished */
			if (!m_APU.dpcm.enabled)
			{
				m_APU.dpcm.enabled = true;
				apu_dpcmreset(&m_APU.dpcm);
			}
		}
		else
			m_APU.dpcm.enabled = false;

		m_APU.dpcm.irq_occurred = false;
		if (!frame_irq_asserted())
			m_irq_handler(false);

		break;
	default:
#ifdef MAME_DEBUG
		if (value)
			logerror("invalid apu write: $%02X at $%04X\n", value, offset);
#endif
		break;
	}
}

// Read status register at $4015
u8 nesapu_device::status_r()
{
	m_stream->update();

	if (m_acc)
		return acc_status();

	u8 readval = 0;
	if (m_APU.squ[0].vbl_length > 0)
		readval |= 0x01;

	if (m_APU.squ[1].vbl_length > 0)
		readval |= 0x02;

	if (m_APU.tri.vbl_length > 0)
		readval |= 0x04;

	if (m_APU.noi.vbl_length > 0)
		readval |= 0x08;

	// bit 4: bytes remaining and the DMC enable as last written (TriCNES), so a disable shows at once although its
	// effect on the DMA side lands a few cycles later (AccuracyCoin "Delta Modulation Channel" test C)
	if (m_dmc_engine ? ((m_dmc_bytes > 0) && m_dmc_status_next) : m_APU.dpcm.enabled)
		readval |= 0x10;

	if (m_APU.frame_irq_occurred)
		readval |= 0x40;

	if (m_APU.dpcm.irq_occurred)
		readval |= 0x80;

	m_APU.frame_irq_occurred = false;
	if (!m_APU.dpcm.irq_occurred)
		m_irq_handler(false);

	return readval;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void nesapu_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t accum = 0.0;

	if (m_acc)
	{
		static const u8 tri_seq[32] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			acc_advance(4);
			apu_dpcm(&m_APU.dpcm);
			const u8 noise = ((m_n.lfsr & 1) || !m_n.length) ? 0 : (m_n.constvol ? m_n.vol : m_n.env.decay);
			accum = m_square_lut[acc_pulse_out(0) + acc_pulse_out(1)];
			accum += m_tnd_lut[tri_seq[m_t.step]][noise][m_APU.dpcm.output];
			stream.put(0, sampindex, accum);
		}
		return;
	}

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		apu_square(&m_APU.squ[0]);
		apu_square(&m_APU.squ[1]);
		apu_triangle(&m_APU.tri);
		apu_noise(&m_APU.noi);
		apu_dpcm(&m_APU.dpcm);

		accum = m_square_lut[m_APU.squ[0].output + m_APU.squ[1].output];
		accum += m_tnd_lut[m_APU.tri.output][m_APU.noi.output][m_APU.dpcm.output];

		stream.put(0, sampindex, accum);
	}
}
