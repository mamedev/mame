// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*

c140.cpp

Simulator based on AMUSE sources.
The C140 sound chip is used by Namco System 2 and System 21
The 219 ASIC (which incorporates a modified C140) is used by Namco NA-1 and NA-2
This chip controls 24 channels (C140) or 16 (219) of PCM.
16 bytes are associated with each channel.
Channels can be 8 bit compressed PCM, or 12 bit signed PCM.

2000.06.26: CAB fixed compressed pcm playback
2002.07.20: R. Belmont added support for multiple banking types
2006.01.08: R. Belmont added support for NA-1/2 "219" derivative
2020.05.06: cam900 implemented some features from QuattroPlay sources, by superctr

TODO:
- What does the INT0 pin do? Normally Namco tied it to VOL0 (with VOL1 = VCC).
- Acknowledge A9 bit (9th address bit) of host interface
- Verify data bus bits of C219
- Verify C219 LFSR algorithm (same as c352.cpp?)
- Verify unknown mode bits (0x40 for C140, 0x02 for C219)

--------------

ASIC "219" notes

On the 219 ASIC used on NA-1 and NA-2, the high registers have the following
meaning instead:
0x1f7: bank for voices 0-3
0x1f1: bank for voices 4-7
0x1f3: bank for voices 8-11
0x1f5: bank for voices 12-15

Some games (bkrtmaq, xday2) write to 0x1fd for voices 12-15 instead.  Probably the bank registers
mirror at 1f8, in which case 1ff is also 0-3, 1f9 is also 4-7, 1fb is also 8-11, and 1fd is also 12-15.

Each bank is 0x20000 (128k), and the voice addresses on the 219 are all multiplied by 2.
Additionally, the 219's base pitch is the same as the C352's (42667).  But these changes
are IMO not sufficient to make this a separate file - all the other registers are
fully compatible.

Finally, the 219 only has 16 voices.

*/

#include "emu.h"
#include "c140.h"

#include <algorithm>

struct voice_registers
{
	u8 volume_right;
	u8 volume_left;
	u8 frequency_msb;
	u8 frequency_lsb;
	u8 bank;
	u8 mode;
	u8 start_msb;
	u8 start_lsb;
	u8 end_msb;
	u8 end_lsb;
	u8 loop_msb;
	u8 loop_lsb;
	u8 reserved[4];
};


// device type definition
DEFINE_DEVICE_TYPE(C140, c140_device, "c140", "Namco C140")
DEFINE_DEVICE_TYPE(C219, c219_device, "c219", "Namco C219")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c140_device - constructor
//-------------------------------------------------

c140_device::c140_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c140_device(mconfig, C140, tag, owner, clock)
{
}

c140_device::c140_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_int1_callback(*this)
	, m_sample_rate(0)
	, m_stream(nullptr)
	, m_mixer_buffer_left(nullptr)
	, m_mixer_buffer_right(nullptr)
	, m_baserate(0)
{
	std::fill(std::begin(m_REG), std::end(m_REG), 0);
	std::fill(std::begin(m_pcmtbl), std::end(m_pcmtbl), 0);
}

c219_device::c219_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c140_device(mconfig, C219, tag, owner, clock)
{
	// TODO: unknown address bus bits
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c140_device::device_start()
{
	m_sample_rate = m_baserate = clock();

	m_int1_timer = timer_alloc(FUNC(c140_device::int1_on), this);

	m_stream = stream_alloc(0, 2, m_sample_rate);

	// make decompress pcm table (Verified from Wii Virtual Console Arcade Starblade)
	for (int i = 0; i < 256; i++)
	{
		int j = (s8)i;
		s8 s1 = j & 7;
		s8 s2 = abs(j >> 3) & 31;

		m_pcmtbl[i] = 0x80 << s1 & 0xff00;
		m_pcmtbl[i] += s2 << (s1 ? s1 + 3 : 4);

		if (j < 0)
			m_pcmtbl[i] = -m_pcmtbl[i];
	}

	std::fill(std::begin(m_REG), std::end(m_REG), 0);

	for (int i = 0; i < MAX_VOICE; i++)
	{
		init_voice(&m_voi[i]);
	}

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	m_mixer_buffer_left = std::make_unique<s16[]>(m_sample_rate);
	m_mixer_buffer_right = std::make_unique<s16[]>(m_sample_rate);

	save_item(NAME(m_REG));

	save_item(STRUCT_MEMBER(m_voi, ptoffset));
	save_item(STRUCT_MEMBER(m_voi, pos));
	save_item(STRUCT_MEMBER(m_voi, key));
	save_item(STRUCT_MEMBER(m_voi, lastdt));
	save_item(STRUCT_MEMBER(m_voi, prevdt));
	save_item(STRUCT_MEMBER(m_voi, dltdt));
	save_item(STRUCT_MEMBER(m_voi, rvol));
	save_item(STRUCT_MEMBER(m_voi, lvol));
	save_item(STRUCT_MEMBER(m_voi, frequency));
	save_item(STRUCT_MEMBER(m_voi, bank));
	save_item(STRUCT_MEMBER(m_voi, mode));
	save_item(STRUCT_MEMBER(m_voi, sample_start));
	save_item(STRUCT_MEMBER(m_voi, sample_end));
	save_item(STRUCT_MEMBER(m_voi, sample_loop));
}

void c219_device::device_start()
{
	c140_device::device_start();

	// generate mulaw table (Verified from Wii Virtual Console Arcade Knuckle Heads)
	// same as c352.cpp
	int j = 0;
	for (int i = 0; i < 128; i++)
	{
		m_pcmtbl[i] = j << 5;
		if (i < 16)
			j += 1;
		else if (i < 24)
			j += 2;
		else if (i < 48)
			j += 4;
		else if (i < 100)
			j += 8;
		else
			j += 16;
	}
	for (int i = 0; i < 128; i++)
		m_pcmtbl[i + 128] = (~m_pcmtbl[i]) & 0xffe0;

	m_lfsr = 0x1234;

	save_item(NAME(m_lfsr));
}

void c140_device::device_clock_changed()
{
	m_sample_rate = m_baserate = clock();

	m_stream->set_sample_rate(m_sample_rate);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	m_mixer_buffer_left = std::make_unique<s16[]>(m_sample_rate);
	m_mixer_buffer_right = std::make_unique<s16[]>(m_sample_rate);
}


void c140_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void c140_device::sound_stream_update(sound_stream &stream)
{
	float pbase = (float)m_baserate * 2.0f / (float)m_sample_rate;
	s16 *lmix, *rmix;

	int samples = stream.samples();
	if (samples > m_sample_rate) samples = m_sample_rate;

	/* zap the contents of the mixer buffer */
	std::fill_n(&m_mixer_buffer_left[0], samples, 0);
	std::fill_n(&m_mixer_buffer_right[0], samples, 0);

	//--- audio update
	for (int i = 0; i < 24; i++)
	{
		C140_VOICE *v = &m_voi[i];
		const struct voice_registers *vreg = (struct voice_registers *)&m_REG[i * 16];

		if (v->key)
		{
			const u16 frequency = (vreg->frequency_msb << 8) | vreg->frequency_lsb;

			/* Abort voice if no frequency value set */
			if (frequency == 0) continue;

			/* Delta =  frequency * ((8MHz/374)*2 / sample rate) */
			const int delta = (int)((float)frequency * pbase);

			/* Calculate left/right channel volumes */
			const int lvol = (vreg->volume_left * 32) / MAX_VOICE; //32ch -> 24ch
			const int rvol = (vreg->volume_right * 32) / MAX_VOICE;

			/* Set mixer outputs base pointers */
			lmix = m_mixer_buffer_left.get();
			rmix = m_mixer_buffer_right.get();

			/* Retrieve sample start/end and calculate size */
			const int st = v->sample_start;
			const int ed = v->sample_end;
			const int sz = ed - st;

			/* Retrieve base pointer to the sample data */
			const int sampleData = find_sample(st, v->bank, i);

			/* Fetch back previous data pointers */
			int offset = v->ptoffset;
			int pos = v->pos;
			s32 lastdt = v->lastdt;
			s32 prevdt = v->prevdt;
			s32 dltdt = v->dltdt;

			/* linear or compressed 12bit signed PCM */
			for (int j = 0; j < samples; j++)
			{
				offset += delta;
				const int cnt = (offset >> 16) & 0x7fff;
				offset &= 0xffff;
				pos += cnt;
				/* Check for the end of the sample */
				if (pos >= sz)
				{
					/* Check if its a looping sample, either stop or loop */
					if (ch_looped(v))
					{
						pos = (v->sample_loop - st);
					}
					else
					{
						v->key = 0;
						break;
					}
				}

				if (cnt)
				{
					const u16 sample = read_word((sampleData + pos) << 1) & 0xfff0; // 12bit
					prevdt = lastdt;
					lastdt = ((ch_mulaw(v)) ? m_pcmtbl[(sample >> 8) & 0xff] : s16(sample)) >> 4;
					dltdt = (lastdt - prevdt);
				}

				/* Caclulate the sample value */
				s32 dt = ((dltdt * offset) >> 16) + prevdt;

				/* Write the data to the sample buffers */
				*lmix++ += (dt * lvol) >> (5 + 4);
				*rmix++ += (dt * rvol) >> (5 + 4);
			}

			/* Save positional data for next callback */
			v->ptoffset = offset;
			v->pos = pos;
			v->lastdt = lastdt;
			v->prevdt = prevdt;
			v->dltdt = dltdt;
		}
	}

	/* render to MAME's stream buffer */
	lmix = m_mixer_buffer_left.get();
	rmix = m_mixer_buffer_right.get();
	{
		for (int i = 0; i < samples; i++)
		{
			stream.put_int_clamp(0, i, *lmix++, 32768 / 8);
			stream.put_int_clamp(1, i, *rmix++, 32768 / 8);
		}
	}
}

void c219_device::sound_stream_update(sound_stream &stream)
{
	float pbase = (float)m_baserate * 2.0f / (float)m_sample_rate;
	s16 *lmix, *rmix;

	int samples = stream.samples();
	if (samples > m_sample_rate) samples = m_sample_rate;

	/* zap the contents of the mixer buffer */
	std::fill_n(&m_mixer_buffer_left[0], samples, 0);
	std::fill_n(&m_mixer_buffer_right[0], samples, 0);

	//--- audio update
	for (int i = 0; i < 16; i++)
	{
		C140_VOICE *v = &m_voi[i];
		const struct voice_registers *vreg = (struct voice_registers *)&m_REG[i * 16];

		if (v->key)
		{
			const u16 frequency = (vreg->frequency_msb << 8) | vreg->frequency_lsb;

			/* Abort voice if no frequency value set */
			if (frequency == 0) continue;

			/* Delta =  frequency * ((8MHz/374)*2 / sample rate) */
			const int delta = (int)((float)frequency * pbase);

			/* Calculate left/right channel volumes */
			const int lvol = (vreg->volume_left * 32) / MAX_VOICE; //32ch -> 24ch
			const int rvol = (vreg->volume_right * 32) / MAX_VOICE;

			/* Set mixer outputs base pointers */
			lmix = m_mixer_buffer_left.get();
			rmix = m_mixer_buffer_right.get();

			/* Retrieve sample start/end and calculate size */
			const int st = v->sample_start;
			const int ed = v->sample_end;
			const int sz = ed - st;

			/* Retrieve base pointer to the sample data */
			const int sampleData = find_sample(st, v->bank, i);

			/* Fetch back previous data pointers */
			int offset = v->ptoffset;
			int pos = v->pos;
			s32 lastdt = v->lastdt;
			s32 prevdt = v->prevdt;
			s32 dltdt = v->dltdt;

			/* linear or compressed 8bit signed PCM */
			for (int j = 0; j < samples; j++)
			{
				offset += delta;
				const int cnt = (offset >> 16) & 0x7fff;
				offset &= 0xffff;
				pos += cnt;
				/* Check for the end of the sample */
				if (pos >= sz)
				{
					/* Check if its a looping sample, either stop or loop */
					if (ch_looped(v) || ch_noise(v))
					{
						pos = (v->sample_loop - st);
					}
					else
					{
						v->key = 0;
						break;
					}
				}

				const int shift = ch_noise(v) ? 8 : 3;
				if (cnt)
				{
					prevdt = lastdt;

					if (ch_noise(v)) // noise
					{
						m_lfsr = (m_lfsr >> 1) ^ ((-(m_lfsr & 1)) & 0xfff6);
						lastdt = s16(m_lfsr);
					}
					else
					{
						lastdt = s8(read_byte(sampleData + pos));
						// 11 bit mulaw
						if (ch_mulaw(v))
							lastdt = m_pcmtbl[lastdt & 0xff] >> 5;
						else
							lastdt <<= 3; // scale as 11bit
					}

					// Sign flip
					if (ch_inv_sign(v))
						lastdt = -lastdt;

					dltdt = (lastdt - prevdt);
				}

				/* Caclulate the sample value */
				s32 dt = ((dltdt * offset) >> 16) + prevdt;

				/* Write the data to the sample buffers */
				*lmix++ += ((ch_inv_lout(v)) ? -(dt * lvol) : (dt * lvol)) >> (5 + shift);
				*rmix++ += (dt * rvol) >> (5 + shift);
			}

			/* Save positional data for next callback */
			v->ptoffset = offset;
			v->pos = pos;
			v->lastdt = lastdt;
			v->prevdt = prevdt;
			v->dltdt = dltdt;
		}
	}

	/* render to MAME's stream buffer */
	lmix = m_mixer_buffer_left.get();
	rmix = m_mixer_buffer_right.get();
	{
		for (int i = 0; i < samples; i++)
		{
			stream.put_int_clamp(0, i, *lmix++, 32768 / 8);
			stream.put_int_clamp(1, i, *rmix++, 32768 / 8);
		}
	}
}

inline u8 c140_device::keyon_status_read(u16 offset)
{
	m_stream->update();
	C140_VOICE const &v = m_voi[offset >> 4];

	// suzuka 8 hours and final lap games read from here, expecting bit 6 to be an in-progress sample flag.
	// four trax also expects bit 4 high for some specific channels to make engine noises to work properly
	// (sounds kinda bogus when player crashes in an object and jump spin, needs real HW verification)
	return (v.key ? 0x40 : 0x00) | (m_REG[offset] & 0x3f);
}


u8 c140_device::c140_r(offs_t offset)
{
	offset &= 0x1ff;
	u8 data = m_REG[offset];

	if ((offset & 0xf) == 0x5 && offset < 0x180)
	{
		data = keyon_status_read(offset);
	}
	else if (offset == 0x1f8)
	{
		// timer reload value = written reg data + 1
		data++;
	}

	return data;
}


void c140_device::c140_w(offs_t offset, u8 data)
{
	m_stream->update();

	offset &= 0x1ff;

	m_REG[offset] = data;
	if (offset < 0x180)
	{
		const u8 ch = offset >> 4;
		C140_VOICE *v = &m_voi[ch];

		if ((offset & 0xf) == 0x5)
		{
			if (data & 0x80)
			{
				const struct voice_registers *vreg = (struct voice_registers *) &m_REG[offset & 0x1f0];
				v->key = 1;
				v->ptoffset = 0;
				v->pos = 0;
				v->lastdt = 0;
				v->prevdt = 0;
				v->dltdt = 0;
				v->bank = vreg->bank;
				v->mode = data;

				const u32 loop = (vreg->loop_msb << 8) + vreg->loop_lsb;
				const u32 start = (vreg->start_msb << 8) + vreg->start_lsb;
				const u32 end = (vreg->end_msb << 8) + vreg->end_lsb;
				v->sample_loop = loop;
				v->sample_start = start;
				v->sample_end = end;
			}
			else
			{
				v->key = 0;
			}
		}
	}

	else switch (offset)
	{
		// timer reload value
		case 0x1f8:
			break;

		// set INT1 timer
		case 0x1fa:
			m_int1_callback(CLEAR_LINE);

			if (BIT(m_REG[0x1fe], 0))
				m_int1_timer->adjust(attotime::from_ticks((m_REG[0x1f8] + 1) * 2, m_baserate));

			break;

		// enable INT1 timer
		case 0x1fe:
			if (BIT(data, 0))
			{
				// kyukaidk and marvlandj want the first interrupt to happen immediately
				if (m_int1_timer->expire().is_never())
					m_int1_callback(ASSERT_LINE);
			}
			else
			{
				m_int1_callback(CLEAR_LINE);
				m_int1_timer->adjust(attotime::never);
			}
			break;

		default:
			break;
	}
}


u8 c219_device::c219_r(offs_t offset)
{
	offset &= 0x1ff;

	// TODO: what happens here on reading unmapped voice regs?
	u8 data = m_REG[offset];

	if ((offset & 0xf) == 0x5 && offset < 0x100)
	{
		// assume same as c140
		data = keyon_status_read(offset);
	}

	return data;
}


void c219_device::c219_w(offs_t offset, u8 data)
{
	m_stream->update();

	offset &= 0x1ff;

	// mirror the bank registers on the 219, fixes bkrtmaq (and probably xday2 based on notes in the HLE)
	if ((offset >= 0x1f8) && BIT(offset, 0))
	{
		offset -= 8;
	}

	m_REG[offset] = data;
	if (offset < 0x100) // only 16 voices
	{
		const u8 ch = offset >> 4;
		C140_VOICE *v = &m_voi[ch];

		if ((offset & 0xf) == 0x5)
		{
			if (data & 0x80)
			{
				const struct voice_registers *vreg = (struct voice_registers *) &m_REG[offset & 0x1f0];
				v->key = 1;
				v->ptoffset = 0;
				v->pos = 0;
				v->lastdt = 0;
				v->prevdt = 0;
				v->dltdt = 0;
				v->bank = vreg->bank;
				v->mode = data;

				const u32 loop = (vreg->loop_msb << 8) + vreg->loop_lsb;
				const u32 start = (vreg->start_msb << 8) + vreg->start_lsb;
				const u32 end = (vreg->end_msb << 8) + vreg->end_lsb;
				// on the 219 asic, addresses are in words
				v->sample_loop = loop << 1;
				v->sample_start = start << 1;
				v->sample_end = end << 1;

				#if 0
				logerror("219: play v %d mode %02x start %x loop %x end %x\n",
					ch, v->mode,
					find_sample(v->sample_start, v->bank, ch),
					find_sample(v->sample_loop, v->bank, ch),
					find_sample(v->sample_end, v->bank, ch));
				#endif
			}
			else
			{
				v->key = 0;
			}
		}
	}
	// TODO: No interrupt/timers?
}

TIMER_CALLBACK_MEMBER(c140_device::int1_on)
{
	m_int1_callback(ASSERT_LINE);
}


void c140_device::init_voice(C140_VOICE *v)
{
	v->key = 0;
	v->ptoffset = 0;
	v->rvol = 0;
	v->lvol = 0;
	v->frequency = 0;
	v->bank = 0;
	v->mode = 0;
	v->sample_start = 0;
	v->sample_end = 0;
	v->sample_loop = 0;
}


/*
   find_sample: compute the actual address of a sample given it's
   address and banking registers, as well as the chip type.
 */
int c140_device::find_sample(int adrs, int bank, int voice)
{
	adrs = (bank << 16) + adrs;

	return adrs;
}

int c219_device::find_sample(int adrs, int bank, int voice)
{
	static const s16 asic219banks[4] = { 0x1f7, 0x1f1, 0x1f3, 0x1f5 };

	adrs = (bank << 16) + adrs;

	// ASIC219's banking is fairly simple
	return ((m_REG[asic219banks[voice / 4]] & 0x3) * 0x20000) + adrs;
}
