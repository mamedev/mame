// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    c352.c - Namco C352 custom PCM chip emulation
    v1.2
    By R. Belmont
    Additional code by cync and the hoot development team

    Thanks to Cap of VivaNonno for info and The_Author for preliminary reverse-engineering

    Chip specs:
    32 voices
    Supports 8-bit linear and 8-bit muLaw samples
    Output: digital, 16 bit, 4 channels
    Output sample rate is the input clock / (288 * 2).

    superctr: The clock divider appears to be configurable for each system.
    Below is a list of the divider values followed by the systems that use it.

    * 228: System 11.
    * 288: System 22, Super 22, NB-1/2, ND-1, FL.
    * 296: System 23, Super 23.
    * 332: System 12.
 */

#include "emu.h"
#include "c352.h"

#define VERBOSE (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

// device type definition
const device_type C352 = &device_creator<c352_device>;

// default address map
static ADDRESS_MAP_START( c352, AS_0, 8, c352_device )
	AM_RANGE(0x000000, 0xffffff) AM_ROM
ADDRESS_MAP_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c352_device - constructor
//-------------------------------------------------

c352_device::c352_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C352, "C352", tag, owner, clock, "c352", __FILE__),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("samples", ENDIANNESS_LITTLE, 8, 24, 0, NULL, *ADDRESS_MAP_NAME(c352))
{
}

//-------------------------------------------------
//  static_set_dividder - configuration helper to
//  set the divider setting
//-------------------------------------------------

void c352_device::static_set_divider(device_t &device, int setting)
{
	c352_device &c352 = downcast<c352_device &>(device);
	c352.m_divider = setting;
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *c352_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

// noise generator
int c352_device::get_mseq_bit()
{
	unsigned int mask = (1 << (7 - 1));
	unsigned int reg = m_mseq_reg;
	unsigned int bit = reg & (1 << (17 - 1));

	if (bit)
	{
		reg = ((reg ^ mask) << 1) | 1;
	}
	else
	{
		reg = reg << 1;
	}

	m_mseq_reg = reg;

	return (reg & 1);
}

void c352_device::mix_one_channel(unsigned long ch, long sample_count)
{
	int i;

	signed short sample, nextsample;
	signed short noisebuf;
	UINT16 noisecnt;
	INT32 frequency, delta, offset, cnt, flag;
	UINT32 bank;
	UINT32 pos;

	frequency = m_c352_ch[ch].pitch;
	delta=frequency;

	pos = m_c352_ch[ch].current_addr;   // sample pointer
	offset = m_c352_ch[ch].pos; // 16.16 fixed-point offset into the sample
	flag = m_c352_ch[ch].flag;
	bank = m_c352_ch[ch].bank << 16;

	noisecnt = m_c352_ch[ch].noisecnt;
	noisebuf = m_c352_ch[ch].noisebuf;

	for(i = 0 ; (i < sample_count) && (flag & C352_FLG_BUSY) ; i++)
	{
		offset += delta;
		cnt = (offset>>16)&0x7fff;
		if (cnt)            // if there is a whole sample part, chop it off now that it's been applied
		{
			offset &= 0xffff;
		}

		if (pos > 0x1000000)
		{
			m_c352_ch[ch].flag &= ~C352_FLG_BUSY;
			return;
		}

		sample = (char)m_direct->read_byte(pos);
		nextsample = (char)m_direct->read_byte(pos+cnt);

		// sample is muLaw, not 8-bit linear (Fighting Layer uses this extensively)
		if (flag & C352_FLG_MULAW)
		{
			sample = m_mulaw_table[(unsigned char)sample];
			nextsample = m_mulaw_table[(unsigned char)nextsample];
		}
		else
		{
			sample <<= 8;
			nextsample <<= 8;
		}

		// play noise instead of sample data
		if (flag & C352_FLG_NOISE)
		{
			int noise_level = 0x8000;
			sample = m_c352_ch[ch].noise = (m_c352_ch[ch].noise << 1) | get_mseq_bit();
			sample = (sample & (noise_level - 1)) - (noise_level >> 1);
			if (sample > 0x7f)
			{
				sample = 0x7f;
			}
			else if (sample < 0)
			{
				sample = 0xff;
			}
			sample = m_mulaw_table[(unsigned char)sample];

			if ( (pos+cnt) == pos )
			{
				noisebuf += sample;
				noisecnt++;
				sample = noisebuf / noisecnt;
			}
			else
			{
				if ( noisecnt )
				{
					sample = noisebuf / noisecnt;
				}
				else
				{
					sample = m_mulaw_table[0x7f];       // Nearest sound(s) is here.
				}
				noisebuf = 0;
				noisecnt = ( flag & C352_FLG_FILTER ) ? 0 : 1;
			}
		}

		// apply linear interpolation
		if ( (flag & (C352_FLG_FILTER | C352_FLG_NOISE)) == 0 )
		{
			sample = (short)(sample + ((nextsample-sample) * (((double)(0x0000ffff&offset) )/0x10000)));
		}

		if ( flag & C352_FLG_PHASEFL )
		{
			m_channel_l[i]  += ((-sample * m_c352_ch[ch].vol_l)>>8);
		}
		else
		{
			m_channel_l[i] += ((sample * m_c352_ch[ch].vol_l)>>8);
		}

		if ( flag & C352_FLG_PHASEFR )
		{
			m_channel_r[i]  += ((-sample * m_c352_ch[ch].vol_r)>>8);
		}
		else
		{
			m_channel_r[i] += ((sample * m_c352_ch[ch].vol_r)>>8);
		}

		if ( flag & C352_FLG_PHASERL )
		{
			m_channel_l2[i] += ((-sample * m_c352_ch[ch].vol_l2)>>8);
		}
		else
		{
			m_channel_l2[i] += ((sample * m_c352_ch[ch].vol_l2)>>8);
		}
		m_channel_r2[i] += ((sample * m_c352_ch[ch].vol_r2)>>8);

		if ( (flag & C352_FLG_REVERSE) && (flag & C352_FLG_LOOP) )
		{
			if ( !(flag & C352_FLG_LDIR) )
			{
				pos += cnt;
				if (
					(((pos&0xFFFF) > m_c352_ch[ch].end_addr) && ((pos&0xFFFF) < m_c352_ch[ch].start) && (m_c352_ch[ch].start > m_c352_ch[ch].end_addr) ) ||
					(((pos&0xFFFF) > m_c352_ch[ch].end_addr) && ((pos&0xFFFF) > m_c352_ch[ch].start) && (m_c352_ch[ch].start < m_c352_ch[ch].end_addr) ) ||
					((pos > (bank|0xFFFF)) && (m_c352_ch[ch].end_addr == 0xFFFF))
					)
				{
					m_c352_ch[ch].flag |= C352_FLG_LDIR;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
			}
			else
			{
				pos -= cnt;
				if (
					(((pos&0xFFFF) < m_c352_ch[ch].repeat) && ((pos&0xFFFF) < m_c352_ch[ch].end_addr) && (m_c352_ch[ch].end_addr > m_c352_ch[ch].start) ) ||
					(((pos&0xFFFF) < m_c352_ch[ch].repeat) && ((pos&0xFFFF) > m_c352_ch[ch].end_addr) && (m_c352_ch[ch].end_addr < m_c352_ch[ch].start) ) ||
					((pos < bank) && (m_c352_ch[ch].repeat == 0x0000))
					)
				{
					m_c352_ch[ch].flag &= ~C352_FLG_LDIR;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
			}
		}
		else if ( flag & C352_FLG_REVERSE )
		{
			pos -= cnt;
			if (
				(((pos&0xFFFF) < m_c352_ch[ch].end_addr) && ((pos&0xFFFF) < m_c352_ch[ch].start) && (m_c352_ch[ch].start > m_c352_ch[ch].end_addr) ) ||
				(((pos&0xFFFF) < m_c352_ch[ch].end_addr) && ((pos&0xFFFF) > m_c352_ch[ch].start) && (m_c352_ch[ch].start < m_c352_ch[ch].end_addr) ) ||
				((pos < bank) && (m_c352_ch[ch].end_addr == 0x0000))
				)
			{
				if ( (flag & C352_FLG_LINK) && (flag & C352_FLG_LOOP) )
				{
					m_c352_ch[ch].bank = m_c352_ch[ch].start_addr & 0xFF;
					m_c352_ch[ch].start_addr = m_c352_ch[ch].repeat_addr;
					m_c352_ch[ch].start = m_c352_ch[ch].start_addr;
					m_c352_ch[ch].repeat = m_c352_ch[ch].repeat_addr;
					pos = (m_c352_ch[ch].bank<<16) + m_c352_ch[ch].start_addr;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else if (flag & C352_FLG_LOOP)
				{
					pos = (pos & 0xFF0000) + m_c352_ch[ch].repeat;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else
				{
					m_c352_ch[ch].flag |= C352_FLG_KEYOFF;
					m_c352_ch[ch].flag &= ~C352_FLG_BUSY;
					return;
				}
			}
		} else {
			pos += cnt;
			if (
				(((pos&0xFFFF) > m_c352_ch[ch].end_addr) && ((pos&0xFFFF) < m_c352_ch[ch].start) && (m_c352_ch[ch].start > m_c352_ch[ch].end_addr) ) ||
				(((pos&0xFFFF) > m_c352_ch[ch].end_addr) && ((pos&0xFFFF) > m_c352_ch[ch].start) && (m_c352_ch[ch].start < m_c352_ch[ch].end_addr) ) ||
				((pos > (bank|0xFFFF)) && (m_c352_ch[ch].end_addr == 0xFFFF))
				)
			{
				if ( (flag & C352_FLG_LINK) && (flag & C352_FLG_LOOP) )
				{
					m_c352_ch[ch].bank = m_c352_ch[ch].start_addr & 0xFF;
					m_c352_ch[ch].start_addr = m_c352_ch[ch].repeat_addr;
					m_c352_ch[ch].start = m_c352_ch[ch].start_addr;
					m_c352_ch[ch].repeat = m_c352_ch[ch].repeat_addr;
					pos = (m_c352_ch[ch].bank<<16) + m_c352_ch[ch].start_addr;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else if (flag & C352_FLG_LOOP)
				{
					pos = (pos & 0xFF0000) + m_c352_ch[ch].repeat;
					m_c352_ch[ch].flag |= C352_FLG_LOOPHIST;
				}
				else
				{
					m_c352_ch[ch].flag |= C352_FLG_KEYOFF;
					m_c352_ch[ch].flag &= ~C352_FLG_BUSY;
					return;
				}
			}
		}
	}

	m_c352_ch[ch].noisecnt = noisecnt;
	m_c352_ch[ch].noisebuf = noisebuf;
	m_c352_ch[ch].pos = offset;
	m_c352_ch[ch].current_addr = pos;
}


void c352_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i, j;
	stream_sample_t *bufferl = outputs[0];
	stream_sample_t *bufferr = outputs[1];
	stream_sample_t *bufferl2 = outputs[2];
	stream_sample_t *bufferr2 = outputs[3];

	for(i = 0 ; i < samples ; i++)
	{
			m_channel_l[i] = m_channel_r[i] = m_channel_l2[i] = m_channel_r2[i] = 0;
	}

	for (j = 0 ; j < 32 ; j++)
	{
		mix_one_channel(j, samples);
	}

	for(i = 0 ; i < samples ; i++)
	{
		*bufferl++ = (short) (m_channel_l[i] >>3);
		*bufferr++ = (short) (m_channel_r[i] >>3);
		*bufferl2++ = (short) (m_channel_l2[i] >>3);
		*bufferr2++ = (short) (m_channel_r2[i] >>3);
	}
}

unsigned short c352_device::read_reg16(unsigned long address)
{
	unsigned long   chan;
	unsigned short  val;

	m_stream->update();

	chan = (address >> 4) & 0xfff;
	if (chan > 31)
	{
		val = 0;
	}
	else
	{
		if ((address & 0xf) == 6)
		{
			val = m_c352_ch[chan].flag;
		}
		else
		{
			val = 0;
		}
	}
	return val;
}

void c352_device::write_reg16(unsigned long address, unsigned short val)
{
	unsigned long   chan;
	int i;

	m_stream->update();

	chan = (address >> 4) & 0xfff;

	if ( address >= 0x400 )
	{
		switch(address)
		{
			case 0x404: // execute key-ons/offs
				for ( i = 0 ; i <= 31 ; i++ )
				{
					if ( m_c352_ch[i].flag & C352_FLG_KEYON )
					{
						if (m_c352_ch[i].start_addr != m_c352_ch[i].end_addr)
						{
							m_c352_ch[i].current_addr = (m_c352_ch[i].bank << 16) + m_c352_ch[i].start_addr;
							m_c352_ch[i].start = m_c352_ch[i].start_addr;
							m_c352_ch[i].repeat = m_c352_ch[i].repeat_addr;
							m_c352_ch[i].noisebuf = 0;
							m_c352_ch[i].noisecnt = 0;
							m_c352_ch[i].flag &= ~(C352_FLG_KEYON | C352_FLG_LOOPHIST);
							m_c352_ch[i].flag |= C352_FLG_BUSY;
						}
					}
					else if ( m_c352_ch[i].flag & C352_FLG_KEYOFF )
					{
						m_c352_ch[i].flag &= ~C352_FLG_BUSY;
						m_c352_ch[i].flag &= ~(C352_FLG_KEYOFF);
					}
				}
				break;
			default:
				break;
		}
		return;
	}

	if (chan > 31)
	{
		LOG(("C352 CTRL %08lx %04x\n", address, val));
		return;
	}
	switch(address & 0xf)
	{
	case 0x0:
		// volumes (output 1)
		LOG(("CH %02ld LVOL %02x RVOL %02x\n", chan, val & 0xff, val >> 8));
		m_c352_ch[chan].vol_l = val & 0xff;
		m_c352_ch[chan].vol_r = val >> 8;
		break;

	case 0x2:
		// volumes (output 2)
		LOG(("CH %02ld RLVOL %02x RRVOL %02x\n", chan, val & 0xff, val >> 8));
		m_c352_ch[chan].vol_l2 = val & 0xff;
		m_c352_ch[chan].vol_r2 = val >> 8;
		break;

	case 0x4:
		// pitch
		LOG(("CH %02ld PITCH %04x\n", chan, val));
		m_c352_ch[chan].pitch = val;
		break;

	case 0x6:
		// flags
		LOG(("CH %02ld FLAG %02x\n", chan, val));
		m_c352_ch[chan].flag = val;
		break;

	case 0x8:
		// bank (bits 16-31 of address);
		m_c352_ch[chan].bank = val & 0xff;
		LOG(("CH %02ld BANK %02x", chan, m_c352_ch[chan].bank));
		break;

	case 0xa:
		// start address
		LOG(("CH %02ld SADDR %04x\n", chan, val));
		m_c352_ch[chan].start_addr = val;
		break;

	case 0xc:
		// end address
		LOG(("CH %02ld EADDR %04x\n", chan, val));
		m_c352_ch[chan].end_addr = val;
		break;

	case 0xe:
		// loop address
		LOG(("CH %02ld LADDR %04x\n", chan, val));
		m_c352_ch[chan].repeat_addr = val;
		break;

	default:
		LOG(("CH %02ld UNKN %01lx %04x", chan, address & 0xf, val));
		break;
	}
}

void c352_device::device_start()
{
	int i;
	double x_max = 32752.0;
	double y_max = 127.0;
	double u = 10.0;

	// find our direct access
	m_direct = &space().direct();

	m_sample_rate_base = clock() / m_divider;

	m_stream = machine().sound().stream_alloc(*this, 0, 4, m_sample_rate_base);

	// generate mulaw table for mulaw format samples
	for (i = 0; i < 256; i++)
	{
			double y = (double) (i & 0x7f);
			double x = (exp (y / y_max * log (1.0 + u)) - 1.0) * x_max / u;

			if (i & 0x80)
			{
			x = -x;
			}
			m_mulaw_table[i] = (short)x;
	}

	// register save state info
	for (i = 0; i < 32; i++)
	{
		save_item(NAME(m_c352_ch[i].vol_l), i);
		save_item(NAME(m_c352_ch[i].vol_r), i);
		save_item(NAME(m_c352_ch[i].vol_l2), i);
		save_item(NAME(m_c352_ch[i].vol_r2), i);
		save_item(NAME(m_c352_ch[i].bank), i);
		save_item(NAME(m_c352_ch[i].noise), i);
		save_item(NAME(m_c352_ch[i].noisebuf), i);
		save_item(NAME(m_c352_ch[i].noisecnt), i);
		save_item(NAME(m_c352_ch[i].pitch), i);
		save_item(NAME(m_c352_ch[i].start_addr), i);
		save_item(NAME(m_c352_ch[i].end_addr), i);
		save_item(NAME(m_c352_ch[i].repeat_addr), i);
		save_item(NAME(m_c352_ch[i].flag), i);
		save_item(NAME(m_c352_ch[i].start), i);
		save_item(NAME(m_c352_ch[i].repeat), i);
		save_item(NAME(m_c352_ch[i].current_addr), i);
		save_item(NAME(m_c352_ch[i].pos), i);
	}
}

void c352_device::device_reset()
{
	// clear all channels states
	memset(m_c352_ch, 0, sizeof(c352_ch_t)*32);

	// init noise generator
	m_mseq_reg = 0x12345678;
}

READ16_MEMBER( c352_device::read )
{
	return(read_reg16(offset*2));
}

WRITE16_MEMBER( c352_device::write )
{
	if (mem_mask == 0xffff)
	{
		write_reg16(offset*2, data);
	}
	else
	{
		logerror("C352: byte-wide write unsupported at this time!\n");
	}
}
