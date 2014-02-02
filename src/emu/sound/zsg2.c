/*
    ZOOM ZSG-2 custom wavetable synthesizer

    Written by Olivier Galibert
    MAME conversion by R. Belmont

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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
    To compute the final 16bits value just shift left by (9-s).
    Yes, that simple.

*/

#include "emu.h"
#include "zsg2.h"


// device type definition
const device_type ZSG2 = &device_creator<zsg2_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zsg2_device - constructor
//-------------------------------------------------

zsg2_device::zsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ZSG2, "ZSG-2", tag, owner, clock, "zsg2", __FILE__),
		device_sound_interface(mconfig, *this),
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

	m_stream = stream_alloc(0, 2, clock() / 128);

	m_mem_base = *region();
	m_mem_size = region()->bytes();
	m_mem_blocks = m_mem_size / 4;
	
	m_mem_copy = auto_alloc_array_clear(machine(), UINT32, m_mem_blocks);
	m_full_samples = auto_alloc_array_clear(machine(), INT16, m_mem_blocks * 4 + 4);
}

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
	int shift = 9 - (block >> 4 & 0xf);
	for (int i = offset; i < (offset + 4); i++)
	{
		if (m_full_samples[i] & 0x40)
			m_full_samples[i] = (INT8)(m_full_samples[i] | 0x80);
		if (shift > 0)
			m_full_samples[i] <<= shift;
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
		for (int ch = 0; ch < 48; ch++)
		{
			if (!m_chan[ch].is_playing)
				continue;
			
			m_chan[ch].step_ptr += m_chan[ch].step;
			if (m_chan[ch].step_ptr & 0x80000)
			{
				m_chan[ch].step_ptr &= 0xffff;
				if (++m_chan[ch].cur_pos >= m_chan[ch].end_pos)
				{
					m_chan[ch].is_playing = false;
					//..
					//m_chan[ch].cur_pos = m_chan[ch].loop_pos;
				}
				m_chan[ch].samples = prepare_samples(m_chan[ch].page | m_chan[ch].cur_pos);
			}
			
			INT16 sample = m_chan[ch].samples[m_chan[ch].step_ptr >> 16 & 3];
			
			
			
			mix_l += sample;
			mix_r += sample;
		}

		outputs[0][i] = mix_l / 48;
		outputs[1][i] = mix_r / 48;
	}
}



void zsg2_device::chan_w(int ch, int reg, UINT16 data)
{
	m_chan[ch].v[reg] = data;
	
	switch (reg)
	{
		case 0x0:
			// lo byte: ?
			// hi byte: start address low
			m_chan[ch].start_pos = (m_chan[ch].start_pos & 0xff00) | (data >> 8 & 0xff);
			break;

		case 0x1:
			// lo byte: start address high
			// hi byte: address page
			m_chan[ch].start_pos = (m_chan[ch].start_pos & 0x00ff) | (data << 8 & 0xff00);
			m_chan[ch].page = data << 8 & 0xff0000;
			break;
		
		case 0x4:
			// frequency?
			m_chan[ch].step = data;
			break;

		case 0x5:
			// lo byte: loop address low
			// hi byte: ?
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0xff00) | (data & 0xff);
			break;
		
		case 0x6:
			// end address
			m_chan[ch].end_pos = data;
			break;

		case 0x7:
			// lo byte: loop address high
			// hi byte: ?
			m_chan[ch].loop_pos = (m_chan[ch].loop_pos & 0x00ff) | (data << 8 & 0xff00);
			break;
		
		default:
			break;
			
	}
}

UINT16 zsg2_device::chan_r(int ch, int reg)
{
	return m_chan[ch].v[reg];
}




void zsg2_device::control_w(int reg, UINT16 data)
{
	switch(reg)
	{
		case 0x00: case 0x01: case 0x02:
		{
			// key on?
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
					


#if 0
	printf("keyon %02x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
		ch,
		m_chan[ch].v[0x0], m_chan[ch].v[0x1], m_chan[ch].v[0x2], m_chan[ch].v[0x3],
		m_chan[ch].v[0x4], m_chan[ch].v[0x5], m_chan[ch].v[0x6], m_chan[ch].v[0x7],
		m_chan[ch].v[0x8], m_chan[ch].v[0x9], m_chan[ch].v[0xa], m_chan[ch].v[0xb],
		m_chan[ch].v[0xc], m_chan[ch].v[0xd], m_chan[ch].v[0xe], m_chan[ch].v[0xf]);

/*
r 0020 0010 - 0000ee50 00003b94
r 0020 0014 - 00019be0 000066f8
r 0020 0018 - 00018eac 000063ab
r 0020 001c - 0000003c 0000000f
keyon 15

9400 083b   00 01   start addr  083b94
0000 0400   02 03
2cec        04
1cab        05      ab = loop addr low
66f8        06      end addr
1c63        07      63 = loop addr high
f1a0 0000 0000 0000 f1a0 1b78 089e 1523



*/

#endif


				}
			}
			break;
		}

		case 0x04: case 0x05: case 0x06:
		{
			// key off?
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
	switch(reg)
	{
		case 0x14:
			return 0xff00;

		case 0x1e:
			// rom readback word low
			if (m_read_address >= m_mem_blocks) return 0;
			return m_mem_base[m_read_address] & 0xffff;
		case 0x1f:
			// rom readback word high
			if (m_read_address >= m_mem_blocks) return 0;
			return m_mem_base[m_read_address] >> 16;
	}

	return 0xffff;
}


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

	return 0;
}
