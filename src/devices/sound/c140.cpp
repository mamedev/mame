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

TODO:
- What does the INT0 pin do? Normally Namco tied it to VOL0 (with VOL1 = VCC).
- Acknowledge A9 bit (9th address bit) of host interface
- Verify data bus bits of C219

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
/*
    2000.06.26  CAB     fixed compressed pcm playback
    2002.07.20  R. Belmont   added support for multiple banking types
    2006.01.08  R. Belmont   added support for NA-1/2 "219" derivative
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


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

static inline int limit(s32 in)
{
	return std::max(-0x7fff, std::min(0x8000, in));
}


//-------------------------------------------------
//  c140_device - constructor
//-------------------------------------------------

c140_device::c140_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, C140, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 25, ENDIANNESS_BIG, 16) // Verified from schematics (24 bit address, 12(16? for C219) bit data)
	, m_int1_callback(*this)
	, m_sample_rate(0)
	, m_stream(nullptr)
	, m_banking_type(C140_TYPE::LINEAR)
	, m_mixer_buffer_left(nullptr)
	, m_mixer_buffer_right(nullptr)
	, m_baserate(0)
{
	std::fill(std::begin(m_REG), std::end(m_REG), 0);
	std::fill(std::begin(m_pcmtbl), std::end(m_pcmtbl), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c140_device::device_start()
{
	m_sample_rate = m_baserate = clock();

	m_int1_callback.resolve_safe();
	m_int1_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(c140_device::int1_on), this));

	m_stream = stream_alloc(0, 2, m_sample_rate);

	/* make decompress pcm table */     //2000.06.26 CAB
	s32 segbase = 0;
	for (int i = 0; i < 8; i++)
	{
		m_pcmtbl[i] = segbase;    //segment base value
		segbase += 16 << i;
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


void c140_device::device_clock_changed()
{
	m_sample_rate = m_baserate = clock();

	m_stream->set_sample_rate(m_sample_rate);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	m_mixer_buffer_left = std::make_unique<s16[]>(m_sample_rate);
	m_mixer_buffer_right = std::make_unique<s16[]>(m_sample_rate);;
}


void c140_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void c140_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	s32   dt;

	float  pbase = (float)m_baserate * 2.0f / (float)m_sample_rate;

	s16   *lmix, *rmix;

	if (samples > m_sample_rate) samples = m_sample_rate;

	/* zap the contents of the mixer buffer */
	std::fill_n(&m_mixer_buffer_left[0], samples, 0);
	std::fill_n(&m_mixer_buffer_right[0], samples, 0);

	/* get the number of voices to update */
	const int voicecnt = (m_banking_type == C140_TYPE::ASIC219) ? 16 : 24;

	//--- audio update
	for (int i = 0; i < voicecnt; i++)
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

			/* Switch on data type - compressed PCM is only for C140 */
			if ((v->mode & 8) && (m_banking_type != C140_TYPE::ASIC219))
			{
				//compressed PCM (maybe correct...)
				/* Loop for enough to fill sample buffer as requested */
				for (int j = 0; j < samples; j++)
				{
					offset += delta;
					const int cnt = (offset >> 16) & 0x7fff;
					offset &= 0xffff;
					pos += cnt;
					//for (; cnt > 0; cnt--)
					{
						/* Check for the end of the sample */
						if (pos >= sz)
						{
							/* Check if its a looping sample, either stop or loop */
							if (v->mode & 0x10)
							{
								pos = (v->sample_loop - st);
							}
							else
							{
								v->key = 0;
								break;
							}
						}

						/* Read the chosen sample byte */
						dt = s8(read_byte((sampleData + pos) << 1));

						/* decompress to 13bit range */     //2000.06.26 CAB
						s32 sdt = dt >> 3;              //signed
						if (sdt < 0)   sdt = (sdt << (dt & 7)) - m_pcmtbl[dt & 7];
						else           sdt = (sdt << (dt & 7)) + m_pcmtbl[dt & 7];

						prevdt = lastdt;
						lastdt = sdt;
						dltdt = (lastdt - prevdt);
					}

					/* Caclulate the sample value */
					dt = ((dltdt * offset) >> 16) + prevdt;

					/* Write the data to the sample buffers */
					*lmix++ += (dt * lvol) >> (5 + 5);
					*rmix++ += (dt * rvol) >> (5 + 5);
				}
			}
			else
			{
				/* linear 12bit(8bit for C219) signed PCM */
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
						if (v->mode & 0x10)
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
						prevdt = lastdt;

						if (m_banking_type == C140_TYPE::ASIC219)
						{
							lastdt = s8(read_byte(sampleData + pos));

							// Sign + magnitude format
							if ((v->mode & 0x01) && (lastdt & 0x80))
								lastdt = -(lastdt & 0x7f);

							// Sign flip
							if (v->mode & 0x40)
								lastdt = -lastdt;

							lastdt <<= 8;
						}
						else
						{
							lastdt = s16(read_word((sampleData + pos) << 1) & 0xfff0); // 12bit
						}

						dltdt = (lastdt - prevdt);
					}

					/* Caclulate the sample value */
					dt = ((dltdt * offset) >> 16) + prevdt;

					/* Write the data to the sample buffers */
					*lmix++ += (dt * lvol) >> (5 + 8);
					*rmix++ += (dt * rvol) >> (5 + 8);
				}
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
		stream_sample_t *dest1 = outputs[0];
		stream_sample_t *dest2 = outputs[1];
		for (int i = 0; i < samples; i++)
		{
			s32 val;

			val = 8 * (*lmix++);
			*dest1++ = limit(val);
			val = 8 * (*rmix++);
			*dest2++ = limit(val);
		}
	}
}


u8 c140_device::c140_r(offs_t offset)
{
	offset &= 0x1ff;
	return m_REG[offset];
}


void c140_device::c140_w(offs_t offset, u8 data)
{
	m_stream->update();

	offset &= 0x1ff;

	// mirror the bank registers on the 219, fixes bkrtmaq (and probably xday2 based on notes in the HLE)
	if ((offset >= 0x1f8) && BIT(offset, 0) && (m_banking_type == C140_TYPE::ASIC219))
	{
		offset -= 8;
	}

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
				// on the 219 asic, addresses are in words
				if (m_banking_type == C140_TYPE::ASIC219)
				{
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
					v->sample_loop = loop;
					v->sample_start = start;
					v->sample_end = end;
				}
			}
			else
			{
				v->key = 0;
			}
		}
	}
	else if (offset == 0x1fa)
	{
		m_int1_callback(CLEAR_LINE);

		// timing not verified
		unsigned div = m_REG[0x1f8] != 0 ? m_REG[0x1f8] : 256;
		attotime interval = attotime::from_ticks(div * 2, m_baserate);
		if (BIT(m_REG[0x1fe], 0))
			m_int1_timer->adjust(interval);
	}
	else if (offset == 0x1fe)
	{
		if (BIT(data, 0))
		{
			// kyukaidk and marvlandj want the first interrupt to happen immediately
			if (!m_int1_timer->enabled())
				m_int1_callback(ASSERT_LINE);
		}
		else
		{
			m_int1_callback(CLEAR_LINE);
			m_int1_timer->enable(false);
		}
	}
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
   address and banking registers, as well as the board type.

   I suspect in "real life" this works like the Sega MultiPCM where the banking
   is done by a small PAL or GAL external to the sound chip, which can be switched
   per-game or at least per-PCB revision as addressing range needs grow.
 */
int c140_device::find_sample(int adrs, int bank, int voice)
{
	int newadr = 0;

	static const s16 asic219banks[4] = { 0x1f7, 0x1f1, 0x1f3, 0x1f5 };

	adrs = (bank << 16) + adrs;

	switch (m_banking_type)
	{
		case C140_TYPE::SYSTEM2:
			// System 2 banking
			/*
			Verified from schematics:
			MD0-MD3 : Connected in 3N "voice0" D0-D3 or D4-D7, Nibble changeable with 74LS157
			MD4-MD11 : Connected in 3M "voice1" or 3L "voice2" D0-D7
			MA0-MA18 : Connected in Address bus of ROMs
			MA19 : Connected in 74LS157 Select Pin
			MA20 : Connected in 74LS157 Strobe Pin
			MA21 : ROM select in MD4-MD11 area
			*/
			newadr = ((adrs & 0x200000) >> 2) | (adrs & 0x7ffff);
			break;

		case C140_TYPE::SYSTEM21:
			// System 21 banking.
			// similar to System 2's.
			// TODO: verify from schematics
			newadr = ((adrs & 0x300000) >> 1) | (adrs & 0x7ffff);
			break;

		case C140_TYPE::ASIC219:
			// ASIC219's banking is fairly simple
			newadr = ((m_REG[asic219banks[voice / 4]] & 0x3) * 0x20000) + adrs;
			break;
		default:
			// linear addressing, or banked with address map
			return adrs;
	}

	return newadr;
}
