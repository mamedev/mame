// license:GPL-2.0+
// copyright-holders:Matthew Conte
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES N2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.  This core is freely available for
  use in any freeware project, subject to the following terms:

  Any modifications to this code must be duly noted in the source and
  approved by Matthew Conte and myself prior to public submission.

  timing notes:
  master = 21477270
  2A03 clock = master/12
  sequencer = master/89490 or CPU/7457

 *****************************************************************************

   NES_APU.CPP

   Actual NES APU interface.

   LAST MODIFIED 02/29/2004

   - Based on Matthew Conte's Nofrendo/Nosefart core and redesigned to
     use MAME system calls and to enable multiple APUs.  Sound at this
     point should be just about 100% accurate, though I cannot tell for
     certain as yet.

 *****************************************************************************

   BUGFIXES:

   - Various bugs concerning the DPCM channel fixed. (Oliver Achten)
   - Fixed $4015 read behaviour. (Oliver Achten)

 *****************************************************************************/

#include "emu.h"
#include "nes_apu.h"

DEFINE_DEVICE_TYPE(NES_APU, nesapu_device, "nesapu", "N2A03 APU")

nesapu_device::nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_is_pal(0)
	, m_samps_per_sync(0)
	, m_stream(nullptr)
	, m_irq_handler(*this)
	, m_mem_read_cb(*this)
{
}

nesapu_device::nesapu_device(const machine_config& mconfig, const char* tag, device_t* owner, u32 clock)
	: nesapu_device(mconfig, NES_APU, tag, owner, clock)
{
}

void nesapu_device::device_reset()
{
	write(0x15, 0x00);
}

void nesapu_device::device_clock_changed()
{
	calculate_rates();
	m_is_pal = m_clock == PAL_APU_CLOCK;
}

void nesapu_device::calculate_rates()
{
	m_samps_per_sync = 89490 / 12; // Is there a different PAL value?

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
	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_mem_read_cb.resolve_safe(0x00);

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
		stream_buffer::sample_t pulse_out = (i == 0) ? 0.0 : 95.88 / ((8128.0 / i) + 100.0);
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
				stream_buffer::sample_t tnd_out = (t / 8227.0) + (n / 12241.0) + (d / 22638.0);
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
	save_item(NAME(m_APU.noi.seed));
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
}

/* TODO: sound channels should *ALL* have DC volume decay */

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

// FIXME: This halts ultrasonic frequencies. On hardware there should be some popping noise? Crash Man's stage in Mega Man 2 is an example. This can probably be removed if hardware filters are implemented (they vary by machine, NES, FC, VS, etc).
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
		chan->seed = (chan->seed >> 1) | ((BIT(chan->seed, 0) ^ BIT(chan->seed, (chan->regs[2] & 0x80) ? 6 : 1)) << 14);
	}

	if (BIT(chan->seed, 0)) /* make it silence */
	{
		chan->output = 0;
		return;
	}

	if (chan->regs[0] & 0x10) /* fixed volume */
		chan->output = chan->regs[0] & 0x0f;
	else
		chan->output = 0x0f - chan->env_vol;
}

/* RESET DPCM PARAMETERS */
static inline void apu_dpcmreset(apu_t::dpcm_t *chan)
{
	chan->address = 0xc000 + u16(chan->regs[2] << 6);
	chan->length = u16(chan->regs[3] << 4) + 1;
	chan->bits_left = chan->length << 3;
	chan->irq_occurred = false;
	chan->enabled = true; /* Fixed * Proper DPCM channel ENABLE/DISABLE flag behaviour*/
}

/* OUTPUT DPCM WAVE SAMPLE (VALUES FROM 0 to +127) */
/* TODO: centerline naughtiness */
void nesapu_device::apu_dpcm(apu_t::dpcm_t *chan)
{
	int freq, bit_pos;

	/* reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
	** reg1: output dc level, 7 bits unsigned
	** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
	** reg3: length, (value * 16) + 1
	*/

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

/* WRITE REGISTER VALUE */
void nesapu_device::write(offs_t offset, u8 value)
{
	m_stream->update();

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

		/* this is somewhat of a hack.  there is some latency on the Real
		** Thing between when trireg0 is written to and when the linear
		** length counter actually begins its countdown.  we want to prevent
		** the case where the program writes to the freq regs first, then
		** to reg 0, and the counter accidentally starts running because of
		** the sound queue's timestamp processing.
		**
		** set to a few NES sample -- should be sufficient
		**
		**    3 * (1789772.727 / 44100) = ~122 cycles, just around one scanline
		**
		** should be plenty of time for the 6502 code to do a couple of table
		** dereferences and load up the other triregs
		*/

	/* used to be 3, but now we run the clock faster, so base it on samples/sync */
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
		if (!(value & 0x80)) {
			m_irq_handler(false);
			m_APU.dpcm.irq_occurred = false;
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
		if(value & 0x80)
			m_APU.step_mode = 5;
		else
			m_APU.step_mode = 4;
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

		if (value & 0x10)
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

		//m_irq_handler(false);
		m_APU.dpcm.irq_occurred = false;

		break;
	default:
#ifdef MAME_DEBUG
logerror("invalid apu write: $%02X at $%04X\n", value, offset);
#endif
		break;
	}
}

/* READ VALUES FROM REGISTERS */
u8 nesapu_device::read(offs_t offset)
{
	if (offset == 0x15) /*FIXED* Address $4015 has different behaviour*/
	{
		int readval = 0;
		if (m_APU.squ[0].vbl_length > 0)
			readval |= 0x01;

		if (m_APU.squ[1].vbl_length > 0)
			readval |= 0x02;

		if (m_APU.tri.vbl_length > 0)
			readval |= 0x04;

		if (m_APU.noi.vbl_length > 0)
			readval |= 0x08;

		if (m_APU.dpcm.enabled)
			readval |= 0x10;

		if (m_APU.dpcm.irq_occurred)
			readval |= 0x80;

		return readval;
	}
	else
		return 0xff; // FIXME: this should be open bus?
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void nesapu_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	stream_buffer::sample_t accum = 0.0;
	auto &output = outputs[0];

	for (int sampindex = 0; sampindex < output.samples(); sampindex++)
	{
		apu_square(&m_APU.squ[0]);
		apu_square(&m_APU.squ[1]);
		apu_triangle(&m_APU.tri);
		apu_noise(&m_APU.noi);
		apu_dpcm(&m_APU.dpcm);

		accum = m_square_lut[m_APU.squ[0].output + m_APU.squ[1].output];
		accum += m_tnd_lut[m_APU.tri.output][m_APU.noi.output][m_APU.dpcm.output];

		output.put(sampindex, accum);
	}
}
