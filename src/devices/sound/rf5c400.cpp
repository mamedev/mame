// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Ricoh RF5C400 emulator

    Written by Ville Linde
    Improvements by the hoot development team

    history -
    2007-02-08 hoot development team
        looping
        stereo panning
        8-bit sample support

    2007-02-16 hoot development team
        envelope
        fixed volume table
*/

#include "emu.h"
#include "rf5c400.h"

static int volume_table[256];
static double pan_table[0x64];

/* envelope parameter (experimental) */
#define ENV_AR_SPEED        0.1
#define ENV_MIN_AR          0x02
#define ENV_MAX_AR          0x80
#define ENV_DR_SPEED        2.0
#define ENV_MIN_DR          0x20
#define ENV_MAX_DR          0x73
#define ENV_RR_SPEED        0.7
#define ENV_MIN_RR          0x20
#define ENV_MAX_RR          0x54

/* PCM type */
enum
{
	TYPE_MASK       = 0x00C0,
	TYPE_16         = 0x0000,
	TYPE_8LOW       = 0x0040,
	TYPE_8HIGH      = 0x0080
};

/* envelope phase */
enum
{
	PHASE_NONE      = 0,
	PHASE_ATTACK,
	PHASE_DECAY,
	PHASE_RELEASE
};


// device type definition
const device_type RF5C400 = &device_creator<rf5c400_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rf5c400_device - constructor
//-------------------------------------------------

rf5c400_device::rf5c400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RF5C400, "RF5C400", tag, owner, clock, "rf5c400", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_stream(nullptr)
{
	memset(m_env_ar_table, 0, sizeof(double)*0x9f);
	memset(m_env_dr_table, 0, sizeof(double)*0x9f);
	memset(m_env_rr_table, 0, sizeof(double)*0x9f);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rf5c400_device::device_start()
{
	rf5c400_init_chip();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rf5c400_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i, ch;
	INT16 *rom = m_rom;
	UINT32 end, loop;
	UINT64 pos;
	UINT8 vol, lvol, rvol, type;
	UINT8 env_phase;
	double env_level, env_step, env_rstep;

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	for (ch=0; ch < 32; ch++)
	{
		rf5c400_channel *channel = &m_channels[ch];
		stream_sample_t *buf0 = outputs[0];
		stream_sample_t *buf1 = outputs[1];

//      start = ((channel->startH & 0xFF00) << 8) | channel->startL;
		end = ((channel->endHloopH & 0xFF) << 16) | channel->endL;
		loop = ((channel->endHloopH & 0xFF00) << 8) | channel->loopL;
		pos = channel->pos;
		vol = channel->volume & 0xFF;
		lvol = channel->pan & 0xFF;
		rvol = channel->pan >> 8;
		type = (channel->volume >> 8) & TYPE_MASK;

		env_phase = channel->env_phase;
		env_level = channel->env_level;
		env_step = channel->env_step;
		env_rstep = env_step * channel->env_scale;

		for (i=0; i < samples; i++)
		{
			INT16 tmp;
			INT32 sample;

			if (env_phase == PHASE_NONE) break;

			tmp = rom[(pos>>16) & m_rommask];
			switch ( type )
			{
				case TYPE_16:
					sample = tmp;
					break;
				case TYPE_8LOW:
					sample = (INT16)(tmp << 8);
					break;
				case TYPE_8HIGH:
					sample = (INT16)(tmp & 0xFF00);
					break;
				default:
					sample = 0;
					break;
			}

			if ( sample & 0x8000 )
			{
				sample ^= 0x7FFF;
			}

			env_level += env_rstep;
			switch (env_phase)
			{
			case PHASE_ATTACK:
				if (env_level >= 1.0)
				{
					env_phase = PHASE_DECAY;
					env_level = 1.0;
					if (channel->decay & 0x0080)
					{
						env_step = 0.0;
					}
					else
					{
						env_step =
							m_env_dr_table[decode80(channel->decay >> 8)];
					}
					env_rstep = env_step * channel->env_scale;
				}
				break;
			case PHASE_DECAY:
				if (env_level <= 0.0)
				{
					env_phase = PHASE_NONE;
					env_level = 0.0;
					env_step = 0.0;
					env_rstep = 0.0;
				}
				break;
			case PHASE_RELEASE:
				if (env_level <= 0.0)
				{
					env_phase = PHASE_NONE;
					env_level = 0.0;
					env_step = 0.0;
					env_rstep = 0.0;
				}
				break;
			}

			sample *= volume_table[vol];
			sample = (sample >> 9) * env_level;
			*buf0++ += sample * pan_table[lvol];
			*buf1++ += sample * pan_table[rvol];

			pos += channel->step;
			if ( (pos>>16) > m_rom.length() || (pos>>16) > end)
			{
				pos -= loop<<16;
				pos &= U64(0xFFFFFF0000);
			}

		}
		channel->pos = pos;

		channel->env_phase = env_phase;
		channel->env_level = env_level;
		channel->env_step = env_step;
	}
}


/*****************************************************************************/

UINT8 rf5c400_device::decode80(UINT8 val)
{
	if (val & 0x80)
	{
		val = (val & 0x7f) + 0x1f;
	}

	return val;
}

void rf5c400_device::rf5c400_init_chip()
{
	int i;

	// init volume table
	{
		double max=255.0;
		for (i = 0; i < 256; i++) {
			volume_table[i]=(UINT16)max;
			max /= pow(10.0,(double)((4.5/(256.0/16.0))/20));
		}
		for(i = 0; i < 0x48; i++) {
			pan_table[i] = sqrt( (double)(0x47 - i) ) / sqrt( (double)0x47 );
		}
		for(i = 0x48; i < 0x64; i++) {
			pan_table[i] = 0.0;
		}
	}

	// init envelope table
	{
		double r;

		// attack
		r = 1.0 / (ENV_AR_SPEED * (clock() / 384));
		for (i = 0; i < ENV_MIN_AR; i++)
		{
			m_env_ar_table[i] = 1.0;
		}
		for (i = ENV_MIN_AR; i < ENV_MAX_AR; i++)
		{
			m_env_ar_table[i] =
				r * (ENV_MAX_AR - i) / (ENV_MAX_AR - ENV_MIN_AR);
		}
		for (i = ENV_MAX_AR; i < 0x9f; i++)
		{
			m_env_ar_table[i] = 0.0;
		}

		// decay
		r = -5.0 / (ENV_DR_SPEED * (clock() / 384));
		for (i = 0; i < ENV_MIN_DR; i++)
		{
			m_env_dr_table[i] = r;
		}
		for (i = ENV_MIN_DR; i < ENV_MAX_DR; i++)
		{
			m_env_dr_table[i] =
				r * (ENV_MAX_DR - i) / (ENV_MAX_DR - ENV_MIN_DR);
		}
		for (i = ENV_MAX_DR; i < 0x9f; i++)
		{
			m_env_dr_table[i] = 0.0;
		}

		// release
		r = -5.0 / (ENV_RR_SPEED * (clock() / 384));
		for (i = 0; i < ENV_MIN_RR; i++)
		{
			m_env_rr_table[i] = r;
		}
		for (i = ENV_MIN_RR; i < ENV_MAX_RR; i++)
		{
			m_env_rr_table[i] =
				r * (ENV_MAX_RR - i) / (ENV_MAX_RR - ENV_MIN_RR);
		}
		for (i = ENV_MAX_RR; i < 0x9f; i++)
		{
			m_env_rr_table[i] = 0.0;
		}
	}

	// init channel info
	for (i = 0; i < 32; i++)
	{
		m_channels[i].env_phase = PHASE_NONE;
		m_channels[i].env_level = 0.0;
		m_channels[i].env_step  = 0.0;
		m_channels[i].env_scale  = 1.0;
	}

	for (i = 0; i < ARRAY_LENGTH(m_channels); i++)
	{
		save_item(NAME(m_channels[i].startH), i);
		save_item(NAME(m_channels[i].startL), i);
		save_item(NAME(m_channels[i].freq), i);
		save_item(NAME(m_channels[i].endL), i);
		save_item(NAME(m_channels[i].endHloopH), i);
		save_item(NAME(m_channels[i].loopL), i);
		save_item(NAME(m_channels[i].pan), i);
		save_item(NAME(m_channels[i].effect), i);
		save_item(NAME(m_channels[i].volume), i);
		save_item(NAME(m_channels[i].attack), i);
		save_item(NAME(m_channels[i].decay), i);
		save_item(NAME(m_channels[i].release), i);
		save_item(NAME(m_channels[i].cutoff), i);
		save_item(NAME(m_channels[i].pos), i);
		save_item(NAME(m_channels[i].step), i);
		save_item(NAME(m_channels[i].keyon), i);
		save_item(NAME(m_channels[i].env_phase), i);
		save_item(NAME(m_channels[i].env_level), i);
		save_item(NAME(m_channels[i].env_step), i);
		save_item(NAME(m_channels[i].env_scale), i);
	}

	m_stream = stream_alloc(0, 2, clock()/384);

	m_rommask = m_rom.length() - 1;
}


/*****************************************************************************/

static UINT16 rf5c400_status = 0;
READ16_MEMBER( rf5c400_device::rf5c400_r )
{
	switch(offset)
	{
		case 0x00:
		{
			return rf5c400_status;
		}

		case 0x04:
		{
			return 0;
		}
	}

	return 0;
}

WRITE16_MEMBER( rf5c400_device::rf5c400_w )
{
	if (offset < 0x400)
	{
		switch(offset)
		{
			case 0x00:
			{
				rf5c400_status = data;
				break;
			}

			case 0x01:      // channel control
			{
				int ch = data & 0x1f;
				switch ( data & 0x60 )
				{
					case 0x60:
						m_channels[ch].pos =
							((m_channels[ch].startH & 0xFF00) << 8) | m_channels[ch].startL;
						m_channels[ch].pos <<= 16;

						m_channels[ch].env_phase = PHASE_ATTACK;
						m_channels[ch].env_level = 0.0;
						m_channels[ch].env_step  =
							m_env_ar_table[decode80(m_channels[ch].attack >> 8)];
						break;
					case 0x40:
						if (m_channels[ch].env_phase != PHASE_NONE)
						{
							m_channels[ch].env_phase = PHASE_RELEASE;
							if (m_channels[ch].release & 0x0080)
							{
								m_channels[ch].env_step = 0.0;
							}
							else
							{
								m_channels[ch].env_step =
									m_env_rr_table[decode80(m_channels[ch].release >> 8)];
							}
						}
						break;
					default:
						m_channels[ch].env_phase = PHASE_NONE;
						m_channels[ch].env_level = 0.0;
						m_channels[ch].env_step  = 0.0;
						break;
				}
				break;
			}

			case 0x08:      // relative to env attack (channel no)
			case 0x09:      // relative to env attack (0x0c00/ 0x1c00)

			case 0x11:      // ? counter for 0x13?
			{
				break;
			}
			case 0x13:      // ? bujutsu writes sample data here
			{
				break;
			}

			case 0x14:      // ? related to 0x11/0x13 ?
				break;

			case 0x21:      // reverb(character).w
			case 0x32:      // reverb(pre-lpf).w
			case 0x2B:      // reverb(level).w
			case 0x20:      // ???.b : reverb(time).b

			case 0x2C:      // chorus(level).w
			case 0x30:      // chorus(rate).w
			case 0x22:      // chorus(macro).w
			case 0x23:      // chorus(depth).w
			case 0x24:      // chorus(macro).w
			case 0x2F:      // chorus(depth).w
			case 0x27:      // chorus(send level to reverb).w

			default:
			{
				//osd_printf_debug("%s:rf5c400_w: %08X, %08X, %08X\n", machine().describe_context(), data, offset, mem_mask);
				break;
			}
		}
		//osd_printf_debug("%s:rf5c400_w: %08X, %08X, %08X at %08X\n", machine().describe_context(), data, offset, mem_mask);
	}
	else
	{
		// channel registers
		int ch = (offset >> 5) & 0x1f;
		int reg = (offset & 0x1f);

		rf5c400_channel *channel = &m_channels[ch];

		switch (reg)
		{
			case 0x00:      // sample start address, bits 23 - 16
			{
				channel->startH = data;
				break;
			}
			case 0x01:      // sample start address, bits 15 - 0
			{
				channel->startL = data;
				break;
			}
			case 0x02:      // sample playing frequency
			{
				channel->step = ((data & 0x1fff) << (data >> 13)) * 4;
				channel->freq = data;
				break;
			}
			case 0x03:      // sample end address, bits 15 - 0
			{
				channel->endL = data;
				break;
			}
			case 0x04:      // sample end address, bits 23 - 16 , sample loop 23 - 16
			{
				channel->endHloopH = data;
				break;
			}
			case 0x05:      // sample loop offset, bits 15 - 0
			{
				channel->loopL = data;
				break;
			}
			case 0x06:      // channel volume
			{
				channel->pan = data;
				break;
			}
			case 0x07:      // effect depth
			{
				// 0xCCRR: CC = chorus send depth, RR = reverb send depth
				channel->effect = data;
				break;
			}
			case 0x08:      // volume, flag
			{
				channel->volume = data;
				break;
			}
			case 0x09:      // env attack
			{
				// 0x0100: max speed                  (in case of attack <= 0x40)
				// 0xXX40: XX = attack-0x3f (encoded) (in case of attack > 0x40)
				//
				channel->attack = data;
				break;
			}
			case 0x0A:      // relative to env attack ?
			{
				// always 0x0100
				break;
			}
			case 0x0B:      // relative to env decay ?
			{
				// always 0x0100
				break;
			}
			case 0x0C:      // env decay
			{
				// 0xXX70: XX = decay (encoded) (in case of decay > 0x71)
				// 0xXX80: XX = decay (encoded) (in case of decay <= 0x71)
				channel->decay = data;
				break;
			}
			case 0x0D:      // relative to env release ?
			{
				// always 0x0100
				break;
			}
			case 0x0E:      // env release
			{
				// 0xXX70: XX = release-0x1f (encoded) (0x01 if release <= 0x20)
				channel->release = data;
				break;
			}
			case 0x10:      // resonance, cutoff freq.
			{
				// bit 15-12: resonance
				// bit 11-0 : cutoff frequency
				channel->cutoff = data;
				break;
			}
		}
	}
}
