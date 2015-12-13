// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    ZOOM ZSG-2 custom wavetable synthesizer

    Written by Olivier Galibert
    MAME conversion by R. Belmont
    Working emulation by The Talentuous Hands Of The Popularious hap
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
- volume/panning is linear? volume slides are too steep
- most music sounds tinny, probably due to missing DSP?
- what is reg 0xa/0xc? seems related to volume
- identify sample flags
  * bassdrum in shikigam level 1 music is a good hint: it should be one octave
    lower, indicating possible stereo sample, or base octave(like in ymf278)
- memory reads out of range sometimes

*/

#include "emu.h"
#include "zsg2.h"


// device type definition
const device_type ZSG2 = &device_creator<zsg2_device>;

//-------------------------------------------------
//  zsg2_device - constructor
//-------------------------------------------------

zsg2_device::zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ZSG2, "ZSG-2", tag, owner, clock, "zsg2", __FILE__),
		device_sound_interface(mconfig, *this),
		m_mem_base(*this, DEVICE_SELF),
		m_read_address(0),
		m_ext_read_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zsg2_device::device_start()
{
	m_ext_read_handler.resolve();

	memset(&m_chan, 0, sizeof(m_chan));

	m_stream = stream_alloc(0, 2, clock() / 768);

	m_mem_blocks = m_mem_base.length();
	m_mem_copy = auto_alloc_array_clear(machine(), UINT32, m_mem_blocks);
	m_full_samples = auto_alloc_array_clear(machine(), INT16, m_mem_blocks * 4 + 4); // +4 is for empty block

	// register for savestates
	save_pointer(NAME(m_mem_copy), m_mem_blocks / sizeof(UINT32));
	save_pointer(NAME(m_full_samples), (m_mem_blocks * 4 + 4) / sizeof(INT16));
	save_item(NAME(m_read_address));

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
		save_item(NAME(m_chan[ch].flags), ch);
		save_item(NAME(m_chan[ch].panl), ch);
		save_item(NAME(m_chan[ch].panr), ch);
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

UINT32 zsg2_device::read_memory(UINT32 offset)
{
	if (offset >= m_mem_blocks)
		return 0;

	if (m_ext_read_handler.isnull())
		return m_mem_base[offset];

	return m_ext_read_handler(offset);
}

INT16 *zsg2_device::prepare_samples(UINT32 offset)
{
	UINT32 block = read_memory(offset);

	if (block == 0)
		return &m_full_samples[m_mem_blocks]; // overflow or 0

	if (block == m_mem_copy[offset])
		return &m_full_samples[offset * 4]; // cached

	m_mem_copy[offset] = block;
	offset *= 4;

	// decompress 32 byte block to 4 16-bit samples
	// 42222222 51111111 60000000 ssss3333
	m_full_samples[offset|0] = block >> 8 & 0x7f;
	m_full_samples[offset|1] = block >> 16 & 0x7f;
	m_full_samples[offset|2] = block >> 24 & 0x7f;
	m_full_samples[offset|3] = (block >> (8+1) & 0x40) | (block >> (16+2) & 0x20) | (block >> (24+3) & 0x10) | (block & 0xf);

	// sign-extend and shift
	UINT8 shift = block >> 4 & 0xf;
	for (int i = offset; i < (offset + 4); i++)
	{
		m_full_samples[i] <<= 9;
		m_full_samples[i] >>= shift;
	}

	return &m_full_samples[offset];
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void zsg2_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		INT32 mix_l = 0;
		INT32 mix_r = 0;

		// loop over all channels
		for (auto & elem : m_chan)
		{
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
				elem.samples = prepare_samples(elem.page | elem.cur_pos);
			}

			INT32 sample = (elem.samples[elem.step_ptr >> 14 & 3] * elem.vol) >> 16;

			mix_l += (sample * elem.panl + sample * (0x1f - elem.panr)) >> 5;
			mix_r += (sample * elem.panr + sample * (0x1f - elem.panl)) >> 5;
		}

		outputs[0][i] = mix_l;
		outputs[1][i] = mix_r;
	}
}


/******************************************************************************/

void zsg2_device::chan_w(int ch, int reg, UINT16 data)
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
			// hi byte: right panning (high bits always 0)
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0xff00) | (data & 0xff);
			m_chan[ch].panr = data >> 8 & 0x1f;
			break;

		case 0x6:
			// end address
			m_chan[ch].end_pos = data;
			break;

		case 0x7:
			// lo byte: loop address high
			// hi byte: left panning (high bits always 0)
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0x00ff) | (data << 8 & 0xff00);
			m_chan[ch].panl = data >> 8 & 0x1f;
			break;

		case 0x9:
			// no function? always 0
			break;

		case 0xb:
			// always writes 0
			// this register is read-only
			break;

		case 0xe:
			// volume
			m_chan[ch].vol = data;
			break;

		case 0xf:
			// flags
			m_chan[ch].flags = data;
			break;

		default:
			break;
	}

	m_chan[ch].v[reg] = data;
}

UINT16 zsg2_device::chan_r(int ch, int reg)
{
	switch (reg)
	{
		case 0xb:
			// ?
			return 0;

		default:
			break;
	}

	return m_chan[ch].v[reg];
}


/******************************************************************************/

void zsg2_device::control_w(int reg, UINT16 data)
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
					m_chan[ch].samples = prepare_samples(m_chan[ch].page | m_chan[ch].cur_pos);
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

		case 0x18:
			break;

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
			break;
	}
}

UINT16 zsg2_device::control_r(int reg)
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
