// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap, superctr
/*
    ZOOM ZSG-2 custom wavetable synthesizer

    Written by Olivier Galibert
    MAME conversion by R. Belmont
    Working emulation by The Talentuous Hands Of The Popularious hap
	Improvements by superctr
    ---------------------------------------------------------

    Additional notes on the sample format, reverse-engineered
    by Olivier Galibert and David Haywood:

    The zoom sample rom is decomposed in 0x40000 bytes pages.  Each page
    starts by a header and is followed by compressed samples.

    The header is a vector of 16 bytes structures composed of 4 32bits
    little-endian values representing:
    - sample start position in bytes, always a multiple of 4
    - sample end position in bytes, minus 4, always...
    - loop position in bytes, always....
    - flags, probably

    It is interesting to note that this header is *not* parsed by the
    ZSG.  The main program reads the rom through appropriate ZSG
    commands, and use the results in subsequent register setups.  It's
    not even obvious that the ZSG cares about the pages, it may just
    see the address space as linear.  In the same line, the
    interpretation of the flags is obviously dependent on the main
    program, not the ZSG, but some of the bits are directly copied to
    some of the registers.

    The samples are compressed with a 2:1 ratio.  Each block of 4-bytes
    becomes 4 16-bits samples.  Reading the 4 bytes as a *little-endian*
    32bits values, the structure is:

    42222222 51111111 60000000 ssss3333

    's' is a 4-bit scale value.  '0000000', '1111111', '2222222' and
    '6543333' are signed 7-bits values corresponding to the 4 samples.
    To compute the final 16bits value, left-align and shift right by s.
    Yes, that simple.

    ---------------------------------------------------------

TODO:
- Filter behavior might not be perfect.
- Volume ramping probably behaves differently on hardware.
  Perhaps this is what the the "filter cutoff" register is for...
- hook up DSP, it's used for reverb and chorus effects.
- identify sample flags
  * bassdrum in shikigam level 1 music is a good hint: it should be one octave
    lower, indicating possible stereo sample, or base octave(like in ymf278)
- memory reads out of range sometimes

*/

#include "emu.h"
#include "zsg2.h"

#include <algorithm>
#include <fstream>
#include <cmath>

#define EMPHASIS_CUTOFF_BASE 0x800
#define EMPHASIS_CUTOFF_SHIFT 1
#define EMPHASIS_OUTPUT_SHIFT 15

// device type definition
DEFINE_DEVICE_TYPE(ZSG2, zsg2_device, "zsg2", "ZOOM ZSG-2")

//-------------------------------------------------
//  zsg2_device - constructor
//-------------------------------------------------

zsg2_device::zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZSG2, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_mem_base(*this, DEVICE_SELF)
	, m_read_address(0)
	, m_ext_read_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zsg2_device::device_start()
{
	m_ext_read_handler.resolve();

	memset(&m_chan, 0, sizeof(m_chan));

	m_stream = stream_alloc(0, 4, clock() / 768);

	m_mem_blocks = m_mem_base.length();
	m_mem_copy = make_unique_clear<uint32_t[]>(m_mem_blocks);
	m_full_samples = make_unique_clear<int16_t[]>(m_mem_blocks * 4 + 4); // +4 is for empty block

	// register for savestates
	save_pointer(NAME(m_mem_copy), m_mem_blocks / sizeof(uint32_t));
	save_pointer(NAME(m_full_samples), (m_mem_blocks * 4 + 4) / sizeof(int16_t));
	save_item(NAME(m_read_address));

	// Generate the output gain table. Assuming -1dB per step for now.
	for (int i = 0, history=0; i < 32; i++)
	{
		double val = pow(10, -(31 - i) / 20.) * 65535.;
		gain_tab[i] = val;
		gain_tab_frac[i] = val-history;
		history = val;
	}

	for (int ch = 0; ch < 48; ch++)
	{
		save_item(NAME(m_chan[ch].v), ch);
		save_item(NAME(m_chan[ch].is_playing), ch);
		save_item(NAME(m_chan[ch].cur_pos), ch);
		save_item(NAME(m_chan[ch].step_ptr), ch);
		save_item(NAME(m_chan[ch].step), ch);
		save_item(NAME(m_chan[ch].start_pos), ch);
		save_item(NAME(m_chan[ch].end_pos), ch);
		save_item(NAME(m_chan[ch].loop_pos), ch);
		save_item(NAME(m_chan[ch].page), ch);

		save_item(NAME(m_chan[ch].vol), ch);
		save_item(NAME(m_chan[ch].vol_initial), ch);
		save_item(NAME(m_chan[ch].vol_target), ch);

		save_item(NAME(m_chan[ch].emphasis_cutoff), ch);
		save_item(NAME(m_chan[ch].emphasis_cutoff_initial), ch);
		save_item(NAME(m_chan[ch].emphasis_cutoff_target), ch);

		save_item(NAME(m_chan[ch].output_cutoff), ch);
		save_item(NAME(m_chan[ch].output_cutoff_initial), ch);
		save_item(NAME(m_chan[ch].output_cutoff_target), ch);

		save_item(NAME(m_chan[ch].emphasis_filter_state), ch);
		save_item(NAME(m_chan[ch].output_filter_state), ch);

		save_item(NAME(m_chan[ch].output_gain), ch);

		save_item(NAME(m_chan[ch].samples), ch);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void zsg2_device::device_reset()
{
	m_read_address = 0;

	// stop playing and clear all channels
	control_w(4, 0xffff);
	control_w(5, 0xffff);
	control_w(6, 0xffff);

	for (int ch = 0; ch < 48; ch++)
		for (int reg = 0; reg < 0x10; reg++)
			chan_w(ch, reg, 0);

#if 0
	for (int i = 0; i < m_mem_blocks; i++)
		prepare_samples(i);

	FILE* f;

	f = fopen("zoom_samples.bin","wb");
	fwrite(m_mem_copy,1,m_mem_blocks*4,f);
	fclose(f);

	f = fopen("zoom_samples.raw","wb");
	fwrite(m_full_samples,2,m_mem_blocks*4,f);
	fclose(f);
#endif
}

/******************************************************************************/

uint32_t zsg2_device::read_memory(uint32_t offset)
{
	if (offset >= m_mem_blocks)
		return 0;

	if (m_ext_read_handler.isnull())
		return m_mem_base[offset];

	return m_ext_read_handler(offset);
}

int16_t *zsg2_device::prepare_samples(uint32_t offset)
{
	uint32_t block = read_memory(offset);

	if (block == 0)
		return &m_full_samples[m_mem_blocks]; // overflow or 0

	if (block == m_mem_copy[offset])
		return &m_full_samples[offset * 4]; // cached

	m_mem_copy[offset] = block;
	offset *= 4;

	// decompress 32 bit block to 4 16-bit samples
	// 42222222 51111111 60000000 ssss3333
	m_full_samples[offset|0] = block >> 8 & 0x7f;
	m_full_samples[offset|1] = block >> 16 & 0x7f;
	m_full_samples[offset|2] = block >> 24 & 0x7f;
	m_full_samples[offset|3] = (block >> (8+1) & 0x40) | (block >> (16+2) & 0x20) | (block >> (24+3) & 0x10) | (block & 0xf);

	// sign-extend and shift
	uint8_t shift = block >> 4 & 0xf;
	for (int i = offset; i < (offset + 4); i++)
	{
		m_full_samples[i] <<= 9;
		m_full_samples[i] >>= shift;
	}

	return &m_full_samples[offset];
}

// Fill the buffer with filtered samples
void zsg2_device::filter_samples(zchan *ch)
{
	int16_t *raw_samples = prepare_samples(ch->page | ch->cur_pos);
	ch->samples[0] = ch->samples[4]; // we want to remember the last sample

	for (int i = 0; i < 4; i++)
	{
		ch->samples[i+1] = raw_samples[i];

		// not sure if the filter works exactly this way, however I am pleased
		// with the output for now.
		ch->emphasis_filter_state += (raw_samples[i]-(ch->emphasis_filter_state>>16)) * (EMPHASIS_CUTOFF_BASE /* - ch->emphasis_cutoff*/);
		ch->samples[i+1] = (ch->emphasis_filter_state) >> EMPHASIS_OUTPUT_SHIFT;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void zsg2_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// DSP is programmed to expect 24-bit samples! So we're not limiting to 16-bit here
	for (int i = 0; i < samples; i++)
	{
		int32_t mix[4] = {};

		int ch = 0;

		// loop over all channels
		for (auto & elem : m_chan)
		//auto & elem = m_chan[0];
		{
			ch++;
			if (!elem.is_playing)
				continue;

			elem.step_ptr += elem.step;
			if (elem.step_ptr & 0x10000)
			{
				elem.step_ptr &= 0xffff;
				if (++elem.cur_pos >= elem.end_pos)
				{
					// loop sample
					elem.cur_pos = elem.loop_pos;
					if ((elem.cur_pos + 1) >= elem.end_pos)
					{
						// end of sample
						elem.is_playing = false;
						continue;
					}
				}
				filter_samples(&elem);
				//elem.samples = prepare_samples(elem.page | elem.cur_pos);
			}

			uint8_t sample_pos = elem.step_ptr >> 14 & 3;
			int32_t sample; // = elem.samples[sample_pos];

			// linear interpolation (hardware certainly does something similar)
			sample = elem.samples[sample_pos];
			sample += ((uint16_t)(elem.step_ptr<<2&0xffff) * (int16_t)(elem.samples[sample_pos+1] - sample))>>16;

			sample = (sample * elem.vol) >> 16;
			//uint8_t vol_base = ~elem.vol >> 11 & 0x1f;
			//uint16_t vol = (~gain_tab[vol_base]) + ((gain_tab_frac[vol_base]*(elem.vol&0x7ff))>>11);
			//sample = (sample * vol) >> 16;

			// another filter...
			elem.output_filter_state += (sample - (elem.output_filter_state>>16)) * elem.output_cutoff;
			sample = elem.output_filter_state >> 16;

			for(int output=0; output<4; output++)
			{
				int output_gain = elem.output_gain[output] & 0x1f; // left / right
				int32_t output_sample = sample;

				if (elem.output_gain[output] & 0x80) // perhaps ?
					output_sample = -output_sample;

				mix[output] += (output_sample * gain_tab[output_gain&0x1f]) >> 13;
			}

			// Apply transitions (This is not accurate yet)
			elem.vol = ramp(elem.vol, elem.vol_target);
			elem.output_cutoff = ramp(elem.output_cutoff, elem.output_cutoff_target);
			elem.emphasis_cutoff = ramp(elem.emphasis_cutoff, elem.emphasis_cutoff_target);
		}

		ch = 0;

		for(int output=0; output<4; output++)
			outputs[output][i] = mix[output];

	}
}

/******************************************************************************/

void zsg2_device::chan_w(int ch, int reg, uint16_t data)
{
	switch (reg)
	{
		case 0x0:
			// lo byte: unknown, 0 on most games
			// hi byte: start address low
			m_chan[ch].start_pos = (m_chan[ch].start_pos & 0xff00) | (data >> 8 & 0xff);
			break;

		case 0x1:
			// lo byte: start address high
			// hi byte: address page
			m_chan[ch].start_pos = (m_chan[ch].start_pos & 0x00ff) | (data << 8 & 0xff00);
			m_chan[ch].page = data << 8 & 0xff0000;
			break;

		case 0x2:
			// no function? always 0
			break;

		case 0x3:
			// unknown, always 0x0400
			break;

		case 0x4:
			// frequency
			m_chan[ch].step = data + 1;
			break;

		case 0x5:
			// lo byte: loop address low
			// hi byte: right output gain (direct)
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0xff00) | (data & 0xff);
			m_chan[ch].output_gain[3] = data >> 8;
			break;

		case 0x6:
			// end address
			m_chan[ch].end_pos = data;
			break;

		case 0x7:
			// lo byte: loop address high
			// hi byte: left output gain (direct)
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0x00ff) | (data << 8 & 0xff00);
			m_chan[ch].output_gain[2] = data >> 8;
			break;

		case 0x8:
			// Filter cutoff (Direct)
			m_chan[ch].output_cutoff_initial = data;
			break;

		case 0x9:
			// no function? always 0
			break;

		case 0xa:
			// volume (Direct)
			m_chan[ch].vol_initial = data;
			break;

		case 0xb:
			// always writes 0
			// this register is read-only
			break;

		case 0xc:
			// IIR lowpass time constant
			m_chan[ch].output_cutoff_target = data;
			break;

		case 0xd:
			// hi byte: DSP channel 1 (chorus) gain
			// lo byte: Filter ramping speed
			m_chan[ch].output_gain[1] = data >> 8;
			m_chan[ch].emphasis_cutoff_initial = expand_reg(data & 0xff);
			break;

		case 0xe:
			// volume (Target)
			m_chan[ch].vol_target = data;
			break;

		case 0xf:
			// hi byte: DSP channel 0 (reverb) gain
			// lo byte: Volume ramping speed
			m_chan[ch].output_gain[0] = data >> 8;
			m_chan[ch].emphasis_cutoff_target = expand_reg(data & 0xff);
			break;

		default:
			break;
	}

	m_chan[ch].v[reg] = data;
}

uint16_t zsg2_device::chan_r(int ch, int reg)
{
	switch (reg)
	{
		case 0xb: // Only later games (taitogn) read this register...
			return m_chan[ch].is_playing << 13;
		default:
			break;
	}

	return m_chan[ch].v[reg];
}

// expand 8-bit reg to 16-bit value. This is used for the emphasis filter
// register. Not sure about how this works, the sound
// CPU uses a lookup table (stored in gdarius sound cpu ROM at 0x6332) to
// calculate this value, for now I'm generating an opproximate inverse.
int16_t zsg2_device::expand_reg(uint8_t val)
{
	static const signed char frac_tab[16] = {8,9,10,11,12,13,14,15,-15,-14,-13,-12,-11,-10,-9,-8};
	static const unsigned char shift_tab[8] = {1, 2, 3, 4, 5, 6, 7, 8};

	return (frac_tab[val&0x0f] << shift_tab[val>>4])>>EMPHASIS_CUTOFF_SHIFT;
}

// ramp registers
// The CPU does not write often enough to make the transitions always sound
// smooth, so the sound chip probably helps by smoothing the changes.
// There are two sets of the volume and filter cutoff registers.
// At key on, the CPU writes to the "direct" registers, after that it will
// write to the "target" register instead.
inline int32_t zsg2_device::ramp(int32_t current, int32_t target)
{
	int32_t difference = abs(target-current);
	difference -= 6;
	
	if(difference < 0)
		return target;
	else if(target < current)
		return target + difference;
	else if(target > current)
		return target - difference;
	
	return target;
}

/******************************************************************************/

void zsg2_device::control_w(int reg, uint16_t data)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02:
		{
			// key on
			int base = (reg & 3) << 4;
			for (int i = 0; i < 16; i++)
			{
				if (data & (1 << i))
				{
					int ch = base | i;
					m_chan[ch].is_playing = true;
					m_chan[ch].cur_pos = m_chan[ch].start_pos;
					m_chan[ch].step_ptr = 0;
					m_chan[ch].emphasis_filter_state = 0;
					m_chan[ch].vol = m_chan[ch].vol_initial;
					m_chan[ch].output_cutoff = m_chan[ch].output_cutoff_initial;
					m_chan[ch].emphasis_cutoff = m_chan[ch].emphasis_cutoff_initial;
					filter_samples(&m_chan[ch]);
				}
			}
			break;
		}

		case 0x04: case 0x05: case 0x06:
		{
			// key off
			int base = (reg & 3) << 4;
			for (int i = 0; i < 16; i++)
			{
				if (data & (1 << i))
				{
					int ch = base | i;
					m_chan[ch].is_playing = false;
				}
			}
			break;
		}

//		case 0x0c: //These registers are sometimes written to by the CPU. Unknown purpose.
//			break;
//		case 0x0d:
//			break;
//		case 0x10:
//			break;

//		case 0x18:
//			break;

		case 0x1c:
			// rom readback address low (low 2 bits always 0)
			if (data & 3) popmessage("ZSG2 address %04X, contact MAMEdev", data);
			m_read_address = (m_read_address & 0x3fffc000) | (data >> 2 & 0x00003fff);
			break;
		case 0x1d:
			// rom readback address high
			m_read_address = (m_read_address & 0x00003fff) | (data << 14 & 0x3fffc000);
			break;

		default:
			logerror("ZSG2 control   %02X = %04X\n", reg, data & 0xffff);
			break;
	}
}

uint16_t zsg2_device::control_r(int reg)
{
	switch (reg)
	{
		case 0x14:
			// memory bus busy?
			// right before reading memory, it polls until low 8 bits are 0
			return 0;

		case 0x1e:
			// rom readback word low
			return read_memory(m_read_address) & 0xffff;
		case 0x1f:
			// rom readback word high
			return read_memory(m_read_address) >> 16;

		default:
			break;
	}

	return 0;
}

/******************************************************************************/

WRITE16_MEMBER(zsg2_device::write)
{
	// we only support full 16-bit accesses
	if (mem_mask != 0xffff)
	{
		popmessage("ZSG2 write mask %04X, contact MAMEdev", mem_mask);
		return;
	}

	m_stream->update();

	if (offset < 0x300)
	{
		int chan = offset >> 4;
		int reg = offset & 0xf;

		chan_w(chan, reg, data);
	}
	else
	{
		control_w(offset - 0x300, data);
	}
}

READ16_MEMBER(zsg2_device::read)
{
	// we only support full 16-bit accesses
	if (mem_mask != 0xffff)
	{
		popmessage("ZSG2 read mask %04X, contact MAMEdev", mem_mask);
		return 0;
	}

	if (offset < 0x300)
	{
		int chan = offset >> 4;
		int reg = offset & 0xf;

		return chan_r(chan, reg);
	}
	else
	{
		return control_r(offset - 0x300);
	}
}
