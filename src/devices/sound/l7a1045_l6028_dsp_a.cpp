// license:LGPL-2.1+
// copyright-holders:R. Belmont, David Haywood, Angelo Salese, ElSemi
/**************************************************************************************************

    L7A1045 L6028 DSP-A
    (QFP120 package)

    This is the audio chip used in the following:
    * SNK Hyper NeoGeo 64 (arcade platform)
    * AKAI MPC2000XL (sampler/synth)
    * AKAI MPC Classic (sampler/synth)
    * AKAI MPC3000 (sampler/synth)

    Paired with an NEC V53 CPU in all cases.

    Voice register format (thanks to Happy for reverse-engineering assistance):

       offset 3           offset 2           offset 1
       fedcba9876543210 | fedcba9876543210 | fedcba9876543210

    0  ffffffffssssaaaa   aaaaaaaaaaaaaaaa   aaaa------------
        f = flags?  always 01
        s = sample type? always 1
        a = sample start address (24 bits, 16 MiB addressable)

    1  ffffffff----aaaa   aaaaaaaaaaaaaaaa   rrrrrrrrrrrrrrrr
        f = flags.  0 for looping, 1 for one-shot
        a = sample end address, bits 23-4 if looping, ignored for one-shot
        r = sample rate in 4.12 fixed point relative to 44100 Hz (0x1000 = 44100 Hz)

    2  ----------------   ????????????????   ????????????????
        All bits unknown.  Data is usually 0000 4000 C000 for one-shot and
        something more complex for looping samples.

        Happy suggests the second word is bits 20-4 of the loop start
        address, but that does not line up with reality.  Needs more research.

    3  ----------------   vvvvvvvvvvvvvvvv   ----------------
        v = volume envelope starting value (16 bit, maaaaybe signed?)

    4  ----------------   vvvvvvvvvvvvvvvv   rrrrrrrrrrrrrrrr
        v = volume envelope target value
        r = volume envelope rate in 8.8 fixed point (0x100 = change the
            volume by 1 sample per sample)

    5  ----------------   cccccccccccccccc   ----------------
        c = lowpass filter cutoff frequency (16 bit, 0xffff = the Nyquist frequency)

    6  ----------------   ccccccccccccRRRR   rrrrrrrrrrrrrrrr]
        c = filter cutoff frequency target bits 15-4
        R = filter resonance (4 bits, 0 = 1.0, 0xf = 0.0)
        r = filter cutoff frequency envelope rate in 8.8 fixed point

    7  ----------------   eeeeeeeeeeeedddd   llllllllrrrrrrrr left/right volume
        e = delay effect parameters, unknown encoding
        d = routing destination?  MPC3000 has 8 discrete outputs plus a stereo master pair.
            The 8 discrete outputs are 4 stereo pairs, and the master pair appears to be
            a mix of the other 4 pairs.
        l = left volume (8 bit, 0-255)
        r = right volume (8 bit, 0-255)
        (Is this correct or is the volume 16 bits and d is a panpot?)

    8  ----------------   ----------------   ---------------- (read only?)

    9  ----------------   ----------------   ---------------- (read only?)

    a  ----------------   ----------------   ----------------
        Unknown, written once on bootup for HNG64 games.

    TODO:
    - sams64 and sams64_2 sometimes have samples get stuck on.  Other games do not
      seem to have this problem.  Why?
    - Sample format seems wrong.
    - Filter parameters are guesswork.  Cutoff frequency seems right, but
      resonance is guesswork.  Getting the MPC3000 usable will help a lot.
    - How does the delay effect work?
    - How does DMA work?

**************************************************************************************************/

#include "emu.h"
#include "l7a1045_l6028_dsp_a.h"
#include "debugger.h"

DEFINE_DEVICE_TYPE(L7A1045, l7a1045_sound_device, "l7a1045", "L7A1045 L6028 DSP-A")

l7a1045_sound_device::l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L7A1045, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_key(0),
		m_rom(*this, DEVICE_SELF)
{
}

void l7a1045_sound_device::device_start()
{
	// Check that the ROM region length is a power of two that we can make a mask from
	assert(!(m_rom.length() & (m_rom.length() - 1)));

	// Allocate the stream
	m_sample_rate = clock() / 768.0f;
	m_stream = stream_alloc(0, 2, m_sample_rate);

	save_item(STRUCT_MEMBER(m_voice, loop_start));
	save_item(STRUCT_MEMBER(m_voice, start));
	save_item(STRUCT_MEMBER(m_voice, end));
	save_item(STRUCT_MEMBER(m_voice, step));
	save_item(STRUCT_MEMBER(m_voice, mode));
	save_item(STRUCT_MEMBER(m_voice, pos));
	save_item(STRUCT_MEMBER(m_voice, frac));
	save_item(STRUCT_MEMBER(m_voice, l_volume));
	save_item(STRUCT_MEMBER(m_voice, r_volume));
	save_item(STRUCT_MEMBER(m_voice, env_volume));
	save_item(STRUCT_MEMBER(m_voice, env_target));
	save_item(STRUCT_MEMBER(m_voice, env_step));
	save_item(STRUCT_MEMBER(m_voice, env_pos));
	save_item(STRUCT_MEMBER(m_voice, flt_freq));
	save_item(STRUCT_MEMBER(m_voice, flt_target));
	save_item(STRUCT_MEMBER(m_voice, flt_step));
	save_item(STRUCT_MEMBER(m_voice, flt_pos));
	save_item(STRUCT_MEMBER(m_voice, flt_resonance));
	save_item(STRUCT_MEMBER(m_voice, x1));
	save_item(STRUCT_MEMBER(m_voice, x2));
	save_item(STRUCT_MEMBER(m_voice, y1));
	save_item(STRUCT_MEMBER(m_voice, y2));
	save_item(STRUCT_MEMBER(m_voice, a1));
	save_item(STRUCT_MEMBER(m_voice, a2));
	save_item(STRUCT_MEMBER(m_voice, b0));
	save_item(STRUCT_MEMBER(m_voice, b1));
	save_item(STRUCT_MEMBER(m_voice, b2));
	save_item(NAME(m_key));
	save_item(NAME(m_audiochannel));
	save_item(NAME(m_audioregister));
	save_item(STRUCT_MEMBER(m_audiodat, dat));

	// init all voices' filters to the Nyquist frequency and transparent resonance
	for (l7a1045_voice &voice: m_voice)
	{
		set_filter(voice, m_sample_rate / 2.0f, 0.707f);
	}
}

void l7a1045_sound_device::device_reset()
{
	m_key = 0;
}

void l7a1045_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < 32; i++)
	{
		if (m_key & (1 << i))
		{
			l7a1045_voice *vptr = &m_voice[i];

			uint32_t start = vptr->start;
			const uint32_t end = vptr->end;
			const uint32_t step  = vptr->step;

			uint32_t pos = vptr->pos;
			uint32_t frac = vptr->frac;

			for (int j = 0; j < stream.samples(); j++)
			{
				int32_t sample;
				uint8_t data;

				pos += (frac >> 12);
				frac &= 0xfff;

				if ((start + pos) >= end)
				{
					pos = vptr->pos = 0;
				}
				const uint32_t address = (start + pos) & (m_rom.length() - 1);
				data = m_rom[address];
				sample = int8_t(data & 0xfc) << (3 - (data & 3));
				frac += step;

				// volume envelope processing
				vptr->env_pos += vptr->env_step;
				const int steps = ((uint32_t)vptr->env_pos / 0x100);
				if (steps > 0)
				{
					if (vptr->env_volume < vptr->env_target)
					{
						vptr->env_volume += std::min(steps, (vptr->env_target - vptr->env_volume));
					}
					else if (vptr->env_volume > vptr->env_target)
					{
						vptr->env_volume -= std::min(steps, (vptr->env_volume - vptr->env_target));
					}
				}
				vptr->env_pos &= 0xff;

				// filter envelope processing
				vptr->flt_pos += vptr->flt_step;
				const int flt_steps = ((uint32_t)vptr->flt_pos / 0x100);
				if (flt_steps > 0)
				{
					if (vptr->flt_freq < vptr->flt_target)
					{
						vptr->flt_freq += std::min(flt_steps, (vptr->flt_target - vptr->flt_freq));
					}
					else if (vptr->flt_freq > vptr->flt_target)
					{
						vptr->flt_freq -= std::min(flt_steps, (vptr->flt_freq - vptr->flt_target));
					}
				}
				vptr->flt_pos &= 0xff;

				const double cutoff = ((double)m_sample_rate) * ((double)vptr->flt_freq / 0x10000);
				const double resonance = //vptr->flt_resonance ?
					(double)vptr->flt_resonance / 15.0f; // : 0.707f;
				set_filter(*vptr, cutoff, resonance);

				// low pass filter processing - this is a direct form 2 biquad implementation
				const double fsample = sample / 32768.0f;
				const double output = vptr->b0 * fsample + vptr->b1 * vptr->x1 + vptr->b2 * vptr->x2 - vptr->a1 * vptr->y1 - vptr->a2 * vptr->y2;
				vptr->x2 = vptr->x1;
				vptr->x1 = fsample;
				vptr->y2 = vptr->y1;
				vptr->y1 = output;

				const int32_t fout = (output * 32768.0f);
				const int64_t left = (fout * (uint64_t(vptr->l_volume) * uint64_t(vptr->env_volume))) >> 17;
				const int64_t right = (fout * (uint64_t(vptr->r_volume) * uint64_t(vptr->env_volume))) >> 17;

				stream.add_int(0, j, left, 32768);
				stream.add_int(1, j, right, 32768);
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}

// TODO: needs proper memory map
void l7a1045_sound_device::l7a1045_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_stream->update();

	if (offset == 0)
	{
		sound_select_w(offset, data, mem_mask);
	}
	else if (offset == 8/2)
	{
		sound_status_w(data);
	}
	else
	{
		sound_data_w(offset - 1, data);
	}
}


uint16_t l7a1045_sound_device::l7a1045_sound_r(offs_t offset, uint16_t mem_mask)
{
	m_stream->update();

	if (offset > 0)
	{
		return sound_data_r(offset - 1);
	}

	return 0xffff;
}


void l7a1045_sound_device::sound_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// ---- ---- 000c cccc
	// c = channel

	if (ACCESSING_BITS_0_7)
	{
		m_audiochannel = data;
		if (m_audiochannel & 0xe0)
		{
			logerror("%s l7a1045_sound_select_w unknown channel %01x\n", machine().describe_context(), m_audiochannel & 0xff);
		}
		m_audiochannel &= 0x1f;
	}

	if (ACCESSING_BITS_8_15)
	{
		m_audioregister = (data >> 8);
		if (m_audioregister >0x0a)
		{
			logerror("%s l7a1045_sound_select_w unknown register %01x\n", machine().describe_context(), m_audioregister & 0xff);
		}
		m_audioregister &= 0x0f;
	}
}

void l7a1045_sound_device::sound_data_w(offs_t offset, uint16_t data)
{
	l7a1045_voice *vptr = &m_voice[m_audiochannel];

	m_audiodat[m_audioregister][m_audiochannel].dat[offset] = data;

	switch (m_audioregister)
	{
		// sample start address
		case 0x00:
			vptr->start = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
			vptr->start |= (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xf000) >> (12);
			vptr->start &= m_rom.length() - 1;

			// clear the pos on start writes (required for DMA tests on MPC3000, and HNG64 likes to leave voices keyed on and just write new parameters)
			vptr->pos = 0;
			vptr->frac = 0;
			// clear the filter state too
			vptr->x1 = vptr->x2 = vptr->y1 = vptr->y2 = 0.0;
			break;

		// loop end address and pitch step
		case 0x01:
			if ((m_audiodat[2][m_audiochannel].dat[0] == 0xc000) && (m_audiodat[2][m_audiochannel].dat[1] == 0x4000))
			{
				vptr->end = m_rom.length() - 1;
				vptr->mode = false;
			}
			else
			{
				vptr->end = (m_audiodat[m_audioregister][m_audiochannel].dat[2] & 0x000f) << (16 + 4);
				vptr->end |= (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xffff) << (4);
				vptr->mode = true;
				vptr->end &= m_rom.length() - 1;
			}

			vptr->step = m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xffff;
			break;

		// unknown exactly what this does
		case 0x02:
			break;

		// starting envelope volume
		case 0x03:
			vptr->env_volume = m_audiodat[m_audioregister][m_audiochannel].dat[1];
			vptr->env_pos = 0;
			break;

		// envelope target volumes plus step rate
		case 0x04:
			vptr->env_target = m_audiodat[m_audioregister][m_audiochannel].dat[1];
			vptr->env_step = m_audiodat[m_audioregister][m_audiochannel].dat[0];
			break;

		// reg 5 = starting lowpass cutoff frequency
		case 0x05:
			vptr->flt_freq = m_audiodat[m_audioregister][m_audiochannel].dat[1];
			vptr->flt_pos = 0;

			{
				const double cutoff = ((double)m_sample_rate) * ((double)vptr->flt_freq / 0x10000);
				set_filter(*vptr, cutoff, 0.707f);
				vptr->x1 = vptr->x2 = vptr->y1 = vptr->y2 = 0.0;
			}
			break;

		// reg 6 = lowpass cutoff target, resonance, and step rate
		case 0x06:
			vptr->flt_target = m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0xfff0;
			vptr->flt_resonance = (m_audiodat[m_audioregister][m_audiochannel].dat[1] & 0x000f) ^ 0xf;
			vptr->flt_step = m_audiodat[m_audioregister][m_audiochannel].dat[0];
			break;

		// voice main volume plus effects routing
		case 0x07:
			vptr->r_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] & 0xff);
			vptr->l_volume = (m_audiodat[m_audioregister][m_audiochannel].dat[0] >> 8) & 0xff;
			break;
	}
}


uint16_t l7a1045_sound_device::sound_data_r(offs_t offset)
{
	const l7a1045_voice *vptr = &m_voice[m_audiochannel];

	// refresh the register shadow from the current voice status if necessary
	switch(m_audioregister)
	{
		case 0x00:
			{
				const uint32_t current_addr = vptr->start + vptr->pos;

				// Reads back the current playback position in the original register 0 format.
				// (roadedge at 0x9DA0)
				m_audiodat[m_audioregister][m_audiochannel].dat[2] &= 0xfff0;
				m_audiodat[m_audioregister][m_audiochannel].dat[2] |= (current_addr >> 20);
				m_audiodat[m_audioregister][m_audiochannel].dat[1] = ((current_addr >> 4) & 0xffff);
				m_audiodat[m_audioregister][m_audiochannel].dat[0] = vptr->frac & 0x0fff;
				m_audiodat[m_audioregister][m_audiochannel].dat[0] |= ((current_addr & 0xf) << 12);
				return m_audiodat[m_audioregister][m_audiochannel].dat[offset];
			}
			break;

		case 0x03:
			m_audiodat[m_audioregister][m_audiochannel].dat[1] = vptr->env_volume;
			break;

		case 0x05:
			m_audiodat[m_audioregister][m_audiochannel].dat[1] = vptr->flt_freq;
			break;
	}

	return m_audiodat[m_audioregister][m_audiochannel].dat[offset];
}

void l7a1045_sound_device::sound_status_w(uint16_t data)
{
	if (data & 0x100) // key on
	{
		l7a1045_voice *vptr = &m_voice[m_audiochannel];

		vptr->frac = 0;
		vptr->pos = 0;
		m_key |= 1 << m_audiochannel;
	}
	else    // key off
	{
		m_key &= ~(1 << m_audiochannel);
	}
}

// sets up parameters for biquad resonant low pass 12 dB/octave filter for a voice
// cutoff in Hz, resonance is 0 to 1.
void l7a1045_sound_device::set_filter(l7a1045_voice &voice, double cutoff, double resonance)
{
	const double omega = 2.0 * M_PI * cutoff / (double)m_sample_rate;
	const double sin_omega = sin(omega);
	const double cos_omega = cos(omega);
	const double alpha = sin_omega / (2.0 * resonance);
	const double a0 = 1.0 + alpha;

	voice.b0 = (1.0 - cos_omega) / (2.0 * a0);
	voice.b1 = (1.0 - cos_omega) / a0;
	voice.b2 = (1.0 - cos_omega) / (2.0 * a0);
	voice.a1 = (-2.0 * cos_omega) / a0;
	voice.a2 = (1.0 - alpha) / a0;
}

// TODO: stub functions not really used
void l7a1045_sound_device::dma_hreq_cb(int state)
{
}

uint8_t l7a1045_sound_device::dma_r_cb(offs_t offset)
{
	m_voice[0].pos++;
	return 0;
}

void l7a1045_sound_device::dma_w_cb(offs_t offset, uint8_t data)
{
	m_voice[0].pos++;
}
