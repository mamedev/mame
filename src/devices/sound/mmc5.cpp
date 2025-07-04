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

   mmc5.cpp

   MMC5 sound emulation core, heavily based from sound/nes_apu.cpp.

 *****************************************************************************/

#include "emu.h"
#include "mmc5.h"

DEFINE_DEVICE_TYPE(MMC5_SOUND,  mmc5_sound_device,  "mmc5_sound",  "Nintendo MMC5 (sound)")

mmc5_sound_device::mmc5_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MMC5_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_samps_per_sync(0)
	, m_stream(nullptr)
	, m_irq_handler(*this)
{
}


void mmc5_sound_device::device_reset()
{
	write(0x15, 0x00);
}

void mmc5_sound_device::device_clock_changed()
{
	calculate_rates();
}

void mmc5_sound_device::calculate_rates()
{
	m_samps_per_sync = m_frame_clocks / 4;

	// initialize sample times in terms of vsyncs
	for (int i = 0; i < SYNCS_MAX1; i++)
	{
		m_vbl_times[i] = vbl_length[i] * m_samps_per_sync / 4; // twice as fast as NES APU
		m_sync_times1[i] = m_samps_per_sync * (i + 1);
	}

	int const rate = clock() / 4;

	if (m_stream != nullptr)
		m_stream->set_sample_rate(rate);
	else
		m_stream = stream_alloc(0, 1, rate);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mmc5_sound_device::device_start()
{
	m_frame_clocks = 29830;

	calculate_rates();

	// calculate mixer output
	/*
	pulse channel output (same as NES APU?):

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

	/* register for save */
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_core.squ[i].regs), i);
		save_item(NAME(m_core.squ[i].vbl_length), i);
		save_item(NAME(m_core.squ[i].freq), i);
		save_item(NAME(m_core.squ[i].phaseacc), i);
		save_item(NAME(m_core.squ[i].env_phase), i);
		save_item(NAME(m_core.squ[i].adder), i);
		save_item(NAME(m_core.squ[i].env_vol), i);
		save_item(NAME(m_core.squ[i].enabled), i);
		save_item(NAME(m_core.squ[i].output), i);
	}

	save_item(NAME(m_core.pcm.regs));
	save_item(NAME(m_core.pcm.irq_enabled));
	save_item(NAME(m_core.pcm.irq_line));
	save_item(NAME(m_core.pcm.output));
}


/* TODO: sound channels should *ALL* have DC volume decay */

/* OUTPUT SQUARE WAVE SAMPLE (VALUES FROM 0 to +15) */
void mmc5_sound_device::tick_square(mmc5_sound_t::square_t &chan)
{
	/* reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
	** reg2: 8 bits of freq
	** reg3: 0-2=high freq, 7-4=vbl length counter
	*/

	if (!chan.enabled)
	{
		chan.output = 0;
		return;
	}

	/* enveloping */
	int const env_delay = m_sync_times1[chan.regs[0] & 0x0f];

	/* decay is at a rate of (env_regs + 1) / 240 secs */
	chan.env_phase -= 4;
	while (chan.env_phase < 0)
	{
		chan.env_phase += env_delay;
		if (BIT(chan.regs[0], 5))
			chan.env_vol = (chan.env_vol + 1) & 15;
		else if (chan.env_vol < 15)
			chan.env_vol++;
	}

	/* vbl length counter */
	if (chan.vbl_length > 0 && BIT(~chan.regs[0], 5))
		chan.vbl_length--;

	if (!chan.vbl_length)
	{
		chan.output = 0;
		return;
	}

	chan.phaseacc -= 4;

	while (chan.phaseacc < 0)
	{
		chan.phaseacc += (chan.freq >> 16);
		chan.adder = (chan.adder + 1) & 0x0f;
	}

	if (BIT(chan.regs[0], 4)) /* fixed volume */
		chan.output = chan.regs[0] & 0x0f;
	else
		chan.output = 0x0f - chan.env_vol;

	chan.output *= BIT(duty_lut[chan.regs[0] >> 6], 7 - BIT(chan.adder, 1, 3));
}

/* WRITE REGISTER VALUE */
void mmc5_sound_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	int chan = BIT(offset, 2);

	switch (offset)
	{
	/* squares */
	case mmc5_sound_t::WRA0:
	case mmc5_sound_t::WRB0:
		m_core.squ[chan].regs[0] = data;
		break;

	case mmc5_sound_t::WRA2:
	case mmc5_sound_t::WRB2:
		m_core.squ[chan].regs[2] = data;
		if (m_core.squ[chan].enabled)
			m_core.squ[chan].freq = ((((m_core.squ[chan].regs[3] & 7) << 8) + data) + 1) << 16;
		break;

	case mmc5_sound_t::WRA3:
	case mmc5_sound_t::WRB3:
		m_core.squ[chan].regs[3] = data;

		if (m_core.squ[chan].enabled)
		{
			m_core.squ[chan].vbl_length = m_vbl_times[data >> 3];
			m_core.squ[chan].env_vol = 0;
			m_core.squ[chan].freq = ((((data & 7) << 8) + m_core.squ[chan].regs[2]) + 1) << 16;
		}

		break;

	/* PCM */
	case mmc5_sound_t::WRE0:
		m_core.pcm.regs[0] = data;
		m_core.pcm.irq_enabled = BIT(data, 7);
		m_irq_handler(m_core.pcm.irq_line & m_core.pcm.irq_enabled);
		break;

	case mmc5_sound_t::WRE1: /* PCM output */
		m_core.pcm.regs[1] = data;
		if (BIT(~m_core.pcm.regs[0], 0))
		{
			if (data == 0x00)
			{
				m_core.pcm.irq_line = true;
				m_irq_handler(m_core.pcm.irq_line & m_core.pcm.irq_enabled);
			}
			else
				m_core.pcm.output = data;
		}
		break;

	case mmc5_sound_t::SMASK:
		if (BIT(data, 0))
			m_core.squ[0].enabled = true;
		else
		{
			m_core.squ[0].enabled = false;
			m_core.squ[0].vbl_length = 0;
		}

		if (BIT(data, 1))
			m_core.squ[1].enabled = true;
		else
		{
			m_core.squ[1].enabled = false;
			m_core.squ[1].vbl_length = 0;
		}
		break;
	default:
#ifdef MAME_DEBUG
logerror("invalid mmc5 sound write: $%02X at $%04X\n", data, offset);
#endif
		break;
	}
}

// Read registers
u8 mmc5_sound_device::read(offs_t offset)
{
	m_stream->update();

	u8 readval = 0;
	switch (offset)
	{
	case mmc5_sound_t::WRE0:
		readval |= (m_core.pcm.irq_line & m_core.pcm.irq_enabled) ? 0x80 : 0;
		if (!machine().side_effects_disabled())
		{
			m_core.pcm.irq_line = false;
			m_irq_handler(m_core.pcm.irq_line & m_core.pcm.irq_enabled);
		}
		break;
	case mmc5_sound_t::SMASK:
		if (m_core.squ[0].vbl_length > 0)
			readval |= 0x01;

		if (m_core.squ[1].vbl_length > 0)
			readval |= 0x02;
		break;
	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Invalid mmc5 sound read at $%02x\n", offset);
		break;
	}

	return readval;
}

// Write PCM data
void mmc5_sound_device::pcm_w(u8 data)
{
	if (BIT(m_core.pcm.regs[0], 0))
		m_core.pcm.output = data;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void mmc5_sound_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t accum = 0.0;

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		tick_square(m_core.squ[0]);
		tick_square(m_core.squ[1]);

		accum = m_square_lut[m_core.squ[0].output + m_core.squ[1].output];
		accum += sound_stream::sample_t(m_core.pcm.output) / 255.0f;

		stream.put(0, sampindex, -accum);
	}
}
