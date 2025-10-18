// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
/**************************************************************************************************

    L7A1045 L6028 DSP-A (called just "L6028" on the Akai schematics)
    (QFP120 package)
    Emulation by R. Belmont and O. Galibert
    Thanks to Happy for invaluable reverse-engineering assistance.
    Thanks also to original authors David Haywood, Angelo Salese, and ElSemi.

    This is the audio chip used in the following:
    * SNK Hyper NeoGeo 64 (arcade platform)
    * AKAI MPC2000 Classic (sampler/synth)
    * AKAI MPC2000XL (sampler/synth)
    * AKAI MPC3000 (sampler/synth)
    * AKAI S2000 (rack mount sampler)
    * AKAI S3000 / CD3000 (rack mount samplers)
    * AKAI S3000XL / CD3000XL (rack mount samplers)
    * AKAI S3200 (rack mount sampler)

    Paired with an NEC V53 CPU in all cases.

    The chip has a total of 10 unique outputs, 8 individual outputs and a stereo pair.  Each voice
    can be sent to an individual output, the stereo pair, or both.

    Companion chips include
    L7A0906 L6029 DFL - "second digital filter"
    L7A1414 L6038 DFX - digital multi-effects processor (S3200, optional add-on for S2000 and S3000)

    The DSP takes 16 bytes of space (8 16-bit words) in the CPU memory map
    0  ---- rrrr ---v vvvv
        v = voice (0-31)
        r = register in the channel

    1  xxxx xxxx xxxx xxxx
    2  xxxx xxxx xxxx xxxx
    3  xxxx xxxx xxxx xxxx
        Currently selected voice register contents (see below)

    4  ---- ---k -??? ????
        k = key on the selected voice if a 1 is written
        V53 writes 0x004f here to cause a DMA request,
        which will then send the key on command to this register.

    5  ---- ---- ---- ----
        Unknown, written by HNG64 when F1 0x commands received from the MIPS
        Debug LEDs or something?

    6  ---- ---- ---- ----
        Performs an atomic update where all 48 bits of the current voice's
        current register are zeroed all at once.  (Are they zeroed or are 3
        copies of what's written here put into them?)


    Voice register format (thanks to Happy for reverse-engineering assistance):
       offset 2           offset 1           offset 0
       fedcba9876543210 | fedcba9876543210 | fedcba9876543210

    0  ffffffffssssaaaa   aaaaaaaaaaaaaaaa   aaaa------------
        f = flags?  always 01
        s = sample type? always 1
        a = sample start address (24 bits, 16 MiB addressable)

    1  ffffffff----aaaa   aaaaaaaaaaaaaaaa   rrrrrrrrrrrrrrrr
        f = flags.  0 for loop encoded by distance from sample end, 1 for loop encoded as an absolute address (see register 2)
        a = sample end address, bits 23-4
        r = sample rate in 4.12 fixed point relative to 44100 Hz (0x1000 = 44100 Hz)

    If the flags field of register 1 is 0, register 2 is encoded like this:
    2  ----------------   mmmmmmmmmmmmmmmm   bbbbbbbbbbbbbbbb
        b = loop length.  Loop start = sample end - loop length.
        m = 2's complement negative of the loop length multiplier, in the same
            4.12 fixed point format as the sample rate.
            A multiplier of 0x1000 means b is exactly the loop length, whereas
            a multiplier of 0x0800 means b is double the loop length so you must
            divide it by 2 to get the actual loop length.

    If the flags field of register 1 is 1, register 2 is encoded like this:
    2  ----------------   aaaaaaaaaaaaaaaa   aaaa--------AAAA
        a = loop start address, bits 19-0
        A = loop start address, bits 23-20
        If bit 15 of m is NOT set, then the base is the offset from the start of the sample
        to the loop start, and the end of the sample is the loop start plus the multiplier.
        This is currently speculation based on HNG64 behavior.  Once the MPC3000 runs it should
        be possible to better understand this.

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

    7  ----------------   vvvvvvvveeeedddd   llllllllrrrrrrrr left/right volume
        e = delay effect parameters, unknown encoding
        d = routing destination
            0xf means "send to delay effect"
            0-7 sends to one of the individual outputs as follows:
            5, 1, 4, 0, 7, 3, 6, 2 maps to outputs 0-7 in order.
        l = left volume (8 bit, 0-255)
        r = right volume (8 bit, 0-255)
        v = destination send volume (8 bit, 0-255)

    8  ----------------   ----------------   ---------------- (written as an atomic update)

    9  ----------------   ----------------   ---------------- (written as an atomic update)

    a  ----------------   ----------------   ----------------
        Unknown, written once on bootup for HNG64 games.

    TODO:
    - How does the delay effect work?

**************************************************************************************************/

#include "emu.h"
#include "l7a1045_l6028_dsp_a.h"
#include "debugger.h"

#define LOG_REGISTERS   (1U << 1)
#define LOG_READBACK    (1U << 2)
#define LOG_KEYON       (1U << 3)
#define LOG_DMA         (1U << 4)

#define VERBOSE (0)

// #define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

enum
{
	L6028_Start = 0,
	L6028_End,
	L6028_Loop_Start,
	L6028_Volume_Env,
	L6028_Volume_Env_Target,
	L6028_Filter_Env,
	L6028_Filter_Env_Target,
	L6028_Mixer_Params
};

DEFINE_DEVICE_TYPE(L7A1045, l7a1045_sound_device, "l7a1045", "L7A1045 L6028 DSP-A")

// channel mapping is weird
static constexpr int channel_remap[8] = { 3, 1, 7, 5, 2, 0, 6, 4 };

l7a1045_sound_device::l7a1045_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L7A1045, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_drq_handler(*this),
	  m_stream(nullptr),
	  m_key(0),
	  m_rom(*this, DEVICE_SELF),
	  m_ram_mask(0)
{
}

void l7a1045_sound_device::map(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(l7a1045_sound_device::voice_select_w));
	map(0x0002, 0x0007).rw(FUNC(l7a1045_sound_device::voiceregs_r), FUNC(l7a1045_sound_device::voiceregs_w));
	map(0x0008, 0x0009).rw(FUNC(l7a1045_sound_device::control_r), FUNC(l7a1045_sound_device::control_w));
	map(0x000c, 0x000d).w(FUNC(l7a1045_sound_device::atomic_w));
}

void l7a1045_sound_device::device_start()
{
	// Check that the ROM region length is a power of two that we can make a mask from
	assert(!(m_rom.length() & (m_rom.length() - 1)));
	m_ram_mask = m_rom.length() - 1;

	// Allocate the stream
	m_sample_rate = clock() / 768.0f;
	m_stream = stream_alloc(0, 10, m_sample_rate);

	save_item(STRUCT_MEMBER(m_voice, loop_start));
	save_item(STRUCT_MEMBER(m_voice, start));
	save_item(STRUCT_MEMBER(m_voice, end));
	save_item(STRUCT_MEMBER(m_voice, step));
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
	save_item(STRUCT_MEMBER(m_voice, b));
	save_item(STRUCT_MEMBER(m_voice, l));
	save_item(STRUCT_MEMBER(m_voice, send_dest));
	save_item(STRUCT_MEMBER(m_voice, send_level));
	save_item(STRUCT_MEMBER(m_voice, sample_type));
	save_item(NAME(m_key));
	save_item(NAME(m_cur_channel));
	save_item(NAME(m_cur_register));
	save_item(NAME(m_regs));
}

void l7a1045_sound_device::device_reset()
{
	m_key = 0;
}

void l7a1045_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < NUM_VOICES; i++)
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
				uint32_t address;
				int32_t sample;
				uint8_t data;

				pos += (frac >> 12);
				frac &= 0xfff;

				if ((start + pos) >= end)
				{
					pos = (vptr->end - vptr->start) - vptr->loop_start;
				}

				switch (vptr->sample_type)
				{
					case 0: // 16-bit linear, little-endian
						address = ((start << 1) + (pos << 1));
						sample = (int8_t(m_rom[address + 1]) << 8) | int8_t(m_rom[address]);

						break;

					case 1: // 12-bit non-linear, encoded into 8 bits
						address = (start + pos) & (m_ram_mask);
						data = m_rom[address];
						sample = (data & 0xfc) >> 2;
						if (sample & 0x20)
							sample -= 0x40;
						sample <<= 4 + 2 * (~data & 3);
						break;

					default:
						logerror("l7a1045: unknown sample type %d\n", vptr->sample_type);
						sample = 0;
						break;
				}

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

				// low pass filter processing using a chamberlin configuration
				// q is 0..1 where 1 is normal and 0 is self-resonating
				// k is 0..2 where 2 is nyquist (2 * sin(pi * fc/fs))
				//    B(0) = L(0) = 0
				//    H' = x0 - L - B  (highpass)
				//    B' = B + k * H'  (bandpass)
				//    L' = L + k * B'  (lowpass)
				//    y0 = L'
				// (fwiw, if you want notch it's H' + L)

				const int32_t h = sample - vptr->l - vptr->b + ((vptr->flt_resonance * vptr->b) >> 4);
				vptr->b += (vptr->flt_freq * h) >> 15;
				vptr->l += (vptr->flt_freq * vptr->b) >> 15;

				const int32_t fout = vptr->l;
				const int64_t left = (fout * (uint64_t(vptr->l_volume) * uint64_t(vptr->env_volume))) >> 24;
				const int64_t right = (fout * (uint64_t(vptr->r_volume) * uint64_t(vptr->env_volume))) >> 24;
				stream.add_int(0, j, left, 32768);
				stream.add_int(1, j, right, 32768);

				if (vptr->send_level > 0)
				{
					const int dest = vptr->send_dest & 0xf;
					if (dest != 0xf)
					{
						const int64_t send = (fout * (uint64_t(vptr->send_level) * uint64_t(vptr->env_volume))) >> 24;
						stream.add_int(2 + channel_remap[dest], j, send, 32768);
					}
				}
			}

			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
}

void l7a1045_sound_device::voice_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// ---- rrrr 000c cccc
	// r = register
	// c = channel

	m_stream->update();

	if (ACCESSING_BITS_0_7)
	{
		m_cur_channel = data;
		if (m_cur_channel & 0xe0)
		{
			logerror("%s l7a1045_sound_select_w unknown channel %01x\n", machine().describe_context(), m_cur_channel & 0xff);
		}
		m_cur_channel &= 0x1f;
	}

	if (ACCESSING_BITS_8_15)
	{
		m_cur_register = (data >> 8);
		if (m_cur_register > 0x0a)
		{
			logerror("%s l7a1045_sound_select_w unknown register %01x\n", machine().describe_context(), m_cur_register & 0xff);
		}
		m_cur_register &= 0x0f;
	}
}

uint16_t l7a1045_sound_device::voiceregs_r(offs_t offset)
{
	const l7a1045_voice *vptr = &m_voice[m_cur_channel];

	m_stream->update();

	// refresh the register shadow from the current voice status if necessary
	switch (m_cur_register)
	{
	case L6028_Start:
	{
		const uint32_t current_addr = vptr->start + vptr->pos;

		// Reads back the current playback position in the original register 0 format.
		// (roadedge at 0x9DA0)
		m_regs[0][m_cur_channel] &= 0xfff0'0000'0000;
		m_regs[0][m_cur_channel] |= (uint64_t(current_addr) << 12);
		m_regs[0][m_cur_channel] |= vptr->frac & 0x0fff;

		LOGMASKED(LOG_REGISTERS, "ch %d cur pos %08x final %012llx\n", m_cur_channel, current_addr, m_regs[0][m_cur_channel]);
	}
	break;

	case L6028_Volume_Env:
		m_regs[3][m_cur_channel] &= 0xffff'0000'ffff;
		m_regs[3][m_cur_channel] |= (uint64_t(vptr->env_volume) << 16);
		LOGMASKED(LOG_READBACK, "ch %d read env vol %x => %012llx\n", m_cur_channel, vptr->env_volume, m_regs[3][m_cur_channel]);
		break;

	case L6028_Filter_Env:
		m_regs[5][m_cur_channel] &= 0xffff'0000'ffff;
		m_regs[5][m_cur_channel] |= (uint64_t(vptr->flt_freq) << 16);
		break;
	}

	return (m_regs[m_cur_register][m_cur_channel] >> (offset * 16)) & 0xffff;
}

void l7a1045_sound_device::voiceregs_w(offs_t offset, uint16_t data)
{
	l7a1045_voice* const vptr = &m_voice[m_cur_channel];
	const uint64_t offset_mask[3] = { 0xffff'ffff'0000ULL, 0xffff'0000'ffffULL, 0x0000'ffff'ffffULL };

	m_stream->update();

	m_regs[m_cur_register][m_cur_channel] &= offset_mask[offset];
	m_regs[m_cur_register][m_cur_channel] |= (uint64_t(data) << (offset * 16));

	LOGMASKED(LOG_REGISTERS, "ch %d reg %x: write %04x offset %d = %012llx\n", m_cur_channel, m_cur_register, data, offset, m_regs[m_cur_register][m_cur_channel]);

	switch (m_cur_register)
	{
		// sample start address
		case L6028_Start:
			vptr->start = (m_regs[L6028_Start][m_cur_channel] >> 12) & 0x00ff'ffff;
			vptr->sample_type = (m_regs[L6028_Start][m_cur_channel] >> 36) & 0xf;

			// clear the pos on start writes (required for DMA tests on MPC3000, and HNG64 likes to leave voices keyed on and just write new parameters)
			vptr->pos = 0;
			vptr->frac = 0;
			// clear the filter state too
			vptr->flt_pos = 0;
			vptr->l = vptr->b = 0;

			if (offset == 2)
			{
				m_regs[L6028_Loop_Start][m_cur_channel] = 0;
			}
			break;

		// loop end address and pitch step
		case L6028_End:
			vptr->end = (m_regs[L6028_End][m_cur_channel] >> 12) & 0x00ff'fff0;

			vptr->step = m_regs[1][m_cur_channel] & 0xffff;
			if (offset == 2)
			{
				recalc_loop_start(vptr);
			}
			break;

		// loop start
		case L6028_Loop_Start:
			recalc_loop_start(vptr);
			break;

		// starting envelope volume
		case L6028_Volume_Env:
			vptr->env_volume = (m_regs[L6028_Volume_Env][m_cur_channel] & 0xffff'0000) >> 16;
			vptr->env_pos = 0;
			break;

		// envelope target volumes plus step rate
		case L6028_Volume_Env_Target:
			vptr->env_target = (m_regs[L6028_Volume_Env_Target][m_cur_channel] & 0xffff'0000) >> 16;
			vptr->env_step = m_regs[L6028_Volume_Env_Target][m_cur_channel] & 0xffff;
			LOGMASKED(LOG_REGISTERS, "ch %d env target %04x step %04x\n", m_cur_channel, vptr->env_target, vptr->env_step);
			break;

		// reg 5 = starting lowpass cutoff frequency
		case L6028_Filter_Env:
			if (vptr->flt_pos == 0)
			{
				vptr->flt_freq = (m_regs[L6028_Filter_Env][m_cur_channel] & 0xffff'0000) >> 16;
			}
			break;

		// reg 6 = lowpass cutoff target, resonance, and step rate
		case L6028_Filter_Env_Target:
			vptr->flt_target = (m_regs[L6028_Filter_Env_Target][m_cur_channel] & 0xfff0'0000) >> 16;
			vptr->flt_resonance = (m_regs[L6028_Filter_Env_Target][m_cur_channel] & 0x000f'0000) >> 16;
			vptr->flt_step = m_regs[6][m_cur_channel] & 0xffff;
			break;

		// voice main volume plus effects routing
		case L6028_Mixer_Params:
			vptr->r_volume = (m_regs[L6028_Mixer_Params][m_cur_channel] & 0xff);
			vptr->l_volume = (m_regs[L6028_Mixer_Params][m_cur_channel] >> 8) & 0xff;
			vptr->send_dest = (m_regs[L6028_Mixer_Params][m_cur_channel] >> 16) & 0xff;
			vptr->send_level = (m_regs[L6028_Mixer_Params][m_cur_channel] >> 24) & 0xff;
			break;
	}
}

void l7a1045_sound_device::recalc_loop_start(l7a1045_voice *vptr)
{
	if (BIT(m_regs[L6028_End][m_cur_channel], 8 + 32))
	{
		const uint32_t length = vptr->end - vptr->start;

		vptr->loop_start = (m_regs[L6028_Loop_Start][m_cur_channel] & 0xffff'f000) >> 12;
		vptr->loop_start |= (m_regs[L6028_Loop_Start][m_cur_channel] & 0x000f) << 20;

		vptr->loop_start = vptr->end - vptr->loop_start;
		if (vptr->loop_start > length)
		{
			vptr->loop_start = length;
		}
	}
	else
	{
		const uint32_t multiplier = (((m_regs[L6028_Loop_Start][m_cur_channel] & 0xffff'0000) >> 16) ^ 0xffff) + 1;
		const uint32_t base = m_regs[2][m_cur_channel] & 0xffff;
		vptr->loop_start = (base * multiplier) >> 12;
	}
}

uint16_t l7a1045_sound_device::control_r()
{
	return 0;
}

// 4f is written here to assert DRQ, at which point the V53 DMAs the key on word
void l7a1045_sound_device::control_w(uint16_t data)
{
	m_stream->update();

	LOGMASKED(LOG_REGISTERS, "%s: %04x to control (ch %d)\n", tag(), data, m_cur_channel);

	if (BIT(data, 8)) // key on
	{
		l7a1045_voice* const vptr = &m_voice[m_cur_channel];

		vptr->frac = 0;
		vptr->pos = 0;
		m_key |= 1 << m_cur_channel;

		recalc_loop_start(vptr);

		LOGMASKED(LOG_KEYON, "ch %d key on start %08x end %08x loop %08x mixer %016llx\n", m_cur_channel, vptr->start, vptr->end, vptr->loop_start, m_regs[L6028_Mixer_Params][m_cur_channel]);
		LOGMASKED(LOG_KEYON, "      raw 0 %012llx 1 %012llx 2 %012llx\n", m_regs[0][m_cur_channel], m_regs[1][m_cur_channel], m_regs[2][m_cur_channel]);
		LOGMASKED(LOG_KEYON, "      raw 3 %012llx 4 %012llx 5 %012llx\n", m_regs[3][m_cur_channel], m_regs[4][m_cur_channel], m_regs[5][m_cur_channel]);
		LOGMASKED(LOG_KEYON, "      raw 6 %012llx 7 %012llx\n", m_regs[6][m_cur_channel], m_regs[7][m_cur_channel]);
	}

	m_drq_handler(BIT(data, 0));
}

void l7a1045_sound_device::atomic_w(uint16_t data)
{
	LOGMASKED(LOG_REGISTERS, "%s atomic write %04x to reg %x\n", tag(), data, m_cur_register);
	m_regs[m_cur_register][m_cur_channel] = 0;
}

uint8_t l7a1045_sound_device::dma_r_cb(offs_t offset)
{
	const offs_t byteoffs = m_voice[0].start + m_voice[0].pos;
	LOGMASKED(LOG_DMA, "%s DMA 8 read: start %08x offs %08x => %08x\n", tag(), m_voice[0].start, m_voice[0].pos, byteoffs);
	m_voice[0].pos++;
	return m_rom[byteoffs];
}

void l7a1045_sound_device::dma_w_cb(offs_t offset, uint8_t data)
{
	const offs_t byteoffs = m_voice[0].start + m_voice[0].pos;
	LOGMASKED(LOG_DMA, "%s DMA 8 write %02x: start %08x offs %08x => %08x\n", tag(), data, m_voice[0].start, m_voice[0].pos, byteoffs);
	m_voice[0].pos++;
	m_rom[byteoffs] = data;
}

uint16_t l7a1045_sound_device::dma_r16_cb(offs_t offset, uint16_t mem_mask)
{
	const offs_t byteoffs = (m_voice[0].start << 1) + (m_voice[0].pos << 1);
	LOGMASKED(LOG_DMA, "%s DMA 16 read: start %08x offs %08x => %08x\n", tag(), m_voice[0].start, m_voice[0].pos, byteoffs);

	m_voice[0].pos++;
	if (byteoffs > m_ram_mask)
	{
		return 0xffff;
	}
	return m_rom[byteoffs] | (m_rom[byteoffs + 1] << 8);
}

void l7a1045_sound_device::dma_w16_cb(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t byteoffs = (m_voice[0].start << 1) + (m_voice[0].pos << 1);
	LOGMASKED(LOG_DMA, "%s DMA 16 write %04x: start %08x offs %08x => %08x\n", tag(), data, m_voice[0].start, m_voice[0].pos, byteoffs);

	m_voice[0].pos++;
	if (byteoffs <= m_ram_mask)
	{
		m_rom[byteoffs] = data & 0xff;
		m_rom[byteoffs + 1] = (data >> 8) & 0xff;
	}
}
