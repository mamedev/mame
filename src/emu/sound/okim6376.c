// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni, James Wallace
/**********************************************************************************************
 *
 *   OKI MSM6376 ADPCM
 *   by Mirko Buffoni, J. Wallace
 *
 *   TODO:
 *     add BEEP tone generator
 *     confirm echo
 *     sample divisor in ROM table not implemented (no documentation)
 *     modernise
 **********************************************************************************************/


#include "emu.h"
#include "okim6376.h"

#define MAX_SAMPLE_CHUNK    10000
//#define MAX_WORDS           111

#define OKIVERBOSE 0
#define MSM6376LOG(x) do { if (OKIVERBOSE) logerror x; } while (0)

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* volume lookup table. Upon configuration, the number of ST pulses determine how much
   attenuation to apply to the sound signal. However, this only applies to the second
   channel*/
static const int volume_table[3] =
{
	0x20,   //   0 dB
	0x10,   //  -6.0 dB
	0x08,   // -12.0 dB
};

/* divisor lookup table. When an individual word is selected, it can be assigned one of three different 'rates'.
   These are implemented as clock divisors, and are looked up in the ROM header. More often than not, this value is 0,
   relating to a division by 8, or nominally 8KHz sampling (based on the datasheet example of a 64KHz clock).*/
static const int divisor_table[3] =
{
	8,
	10,
	16,
};

/* tables computed? */
static int tables_computed = 0;

/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}

	tables_computed = 1;
}



/**********************************************************************************************

     reset_adpcm -- reset the ADPCM stream

***********************************************************************************************/

static void reset_adpcm(struct ADPCMVoice *voice)
{
	/* make sure we have our tables */
	if (!tables_computed)
		compute_tables();

	/* reset the signal/step */
	voice->signal = -2;
	voice->step = 0;
}


const device_type OKIM6376 = &device_creator<okim6376_device>;

okim6376_device::okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, OKIM6376, "OKI6376", tag, owner, clock, "okim6376", __FILE__),
		device_sound_interface(mconfig, *this),
		m_region_base(*this, DEVICE_SELF),
		//m_command[OKIM6376_VOICES],
		m_latch(0),
		//m_stage[OKIM6376_VOICES],
		m_stream(NULL),
		m_master_clock(0),
		m_divisor(0),
		m_channel(0),
		m_nar(0),
		m_nartimer(0),
		m_busy(0),
		m_ch2(0),
		m_st(0),
		m_st_pulses(0),
		m_ch2_update(0),
		m_st_update(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void okim6376_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim6376_device::device_start()
{
	int voice;
	compute_tables();

	m_command[0] = -1;
	m_command[1] = -1;
	m_stage[0] = 0;
	m_stage[1] = 0;
	m_latch = 0;
	m_master_clock = clock();
	m_divisor = divisor_table[0];
	m_nar = 1;
	m_nartimer = 0;
	m_busy = 1;
	m_st = 1;
	m_ch2 = 1;
	m_st_update = 0;
	m_ch2_update = 0;
	m_st_pulses = 0;
	/* generate the name and create the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock() / m_divisor);

	/* initialize the voices */
	for (voice = 0; voice < OKIM6376_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		m_voice[voice].volume = 0;
		reset_adpcm(&m_voice[voice]);
	}

	okim6376_state_save_register();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim6376_device::device_reset()
{
	int i;

	m_stream->update();
	for (i = 0; i < OKIM6376_VOICES; i++)
		m_voice[i].playing = 0;
}


/**********************************************************************************************

     clock_adpcm -- clock the next ADPCM byte

***********************************************************************************************/

static INT16 clock_adpcm(struct ADPCMVoice *voice, UINT8 nibble)
{
	voice->signal += diff_lookup[voice->step * 16 + (nibble & 15)];

	/* clamp to the maximum 12bit */
	if (voice->signal > 2047)
		voice->signal = 2047;
	else if (voice->signal < -2048)
		voice->signal = -2048;

	/* adjust the step size and clamp */
	voice->step += index_shift[nibble & 7];
	if (voice->step > 48)
		voice->step = 48;
	else if (voice->step < 0)
		voice->step = 0;

	/* return the signal */
	return voice->signal;
}


void okim6376_device::oki_process(int channel, int command)
{
	/* if a command is pending, process the second half */
	if ((command != -1) && (command != 0)) //process silence separately
	{
		int start;
		unsigned char *base/*, *base_end*/;
		/* update the stream */
		m_stream->update();

		/* determine which voice(s) (voice is set by the state of 2CH) */
		{
			struct ADPCMVoice *voice = &m_voice[channel];

			/* determine the start position, max address space is 16Mbit */
			base = &m_region_base[m_command[channel] * 4];
			//base_end = &m_region_base[(MAX_WORDS+1) * 4];
			start = ((base[0] << 16) + (base[1] << 8) + base[2]) & 0x1fffff;

				if (start == 0)
				{
					voice->playing = 0;
				}
				else
				{
					/* set up the voice to play this sample */
					if (!voice->playing)
					{
						voice->playing = 1;
						voice->base_offset = start;
						voice->sample = 0;
						voice->count = 0;

						/* also reset the ADPCM parameters */
						reset_adpcm(voice);
						if (channel == 0)
						{
							/* We set channel 2's audio separately */
							voice->volume = volume_table[0];
						}
					}
					else
					{
						if (((m_nar)&&(channel == 0))||(channel == 1))//Store the request, for later processing (channel 2 ignores NAR)
						{
							m_stage[channel] = 1;
						}
					}
				}
			}
		}
		/* otherwise, see if this is a silence command */
		else
		{
			/* update the stream, then turn it off */
			m_stream->update();

			if (command ==0)
			{
				int i;
				for (i = 0; i < OKIM6376_VOICES; i++)
				{
					struct ADPCMVoice *voice = &m_voice[i];
					voice->playing = 0;
			}
		}
	}
}


/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

void okim6376_device::generate_adpcm(struct ADPCMVoice *voice, INT16 *buffer, int samples,int channel)
{
	/* if this voice is active */
	if (voice->playing)
	{
		UINT8 *base = m_region_base + voice->base_offset;
		int sample = voice->sample;
		int count = voice->count;

		/* loop while we still have samples to generate */
		while (samples)
		{
			int nibble;

			if (count == 0)
			{
				/* get the number of samples to play */
				count = (base[sample / 2] & 0x7f) << 1;

				/* end of voice marker */
				if (count == 0)
				{
					voice->playing = 0;
					break;
				}
				else
				{
					/* step past the count byte */
					sample += 2;
				}
			}

			/* compute the new amplitude and update the current step */
			nibble = base[sample / 2] >> (((sample & 1) << 2) ^ 4);

			/* output to the buffer, scaling by the volume */
			/* signal in range -4096..4095, volume in range 2..16 => signal * volume / 2 in range -32768..32767 */
			*buffer++ = clock_adpcm(voice, nibble) * voice->volume / 2;

			++sample;
			--count;
			--samples;
		}

		/* update the parameters */
		voice->sample = sample;
		voice->count = count;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;

	if ((!voice->playing)&&(m_stage[channel]))//end of samples, load anything staged in
	{
		m_stage[channel] = 0;
		oki_process(channel,m_command[channel]);
	}
}


/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

void okim6376_device::postload()
{
	set_frequency(m_master_clock);
}

void okim6376_device::adpcm_state_save_register(struct ADPCMVoice *voice, int index)
{
	save_item(NAME(voice->playing), index);
	save_item(NAME(voice->sample), index);
	save_item(NAME(voice->count), index);
	save_item(NAME(voice->signal), index);
	save_item(NAME(voice->step), index);
	save_item(NAME(voice->volume), index);
	save_item(NAME(voice->base_offset), index);
}

void okim6376_device::okim6376_state_save_register()
{
	int j;
	for (j = 0; j < OKIM6376_VOICES; j++)
	{
		adpcm_state_save_register(&m_voice[j], j);
	}
		machine().save().register_postload(save_prepost_delegate(FUNC(okim6376_device::postload), this));
		save_item(NAME(m_command[0]));
		save_item(NAME(m_command[1]));
		save_item(NAME(m_stage[0]));
		save_item(NAME(m_stage[1]));
		save_item(NAME(m_latch));
		save_item(NAME(m_divisor));
		save_item(NAME(m_nar));
		save_item(NAME(m_nartimer));
		save_item(NAME(m_busy));
		save_item(NAME(m_st));
		save_item(NAME(m_st_pulses));
		save_item(NAME(m_st_update));
		save_item(NAME(m_ch2));
		save_item(NAME(m_ch2_update));
		save_item(NAME(m_master_clock));
}

void okim6376_device::set_frequency(int frequency)
{
	m_master_clock = frequency;
	m_stream->set_sample_rate(m_master_clock / m_divisor);
}


/**********************************************************************************************

     okim6376_status_r -- read the status port of an OKIM6376-compatible chip

***********************************************************************************************/

READ_LINE_MEMBER( okim6376_device::busy_r )
{
	struct ADPCMVoice *voice0 = &m_voice[0];
	struct ADPCMVoice *voice1 = &m_voice[1];

	/* set the bit low if it's playing */
	if ((voice0->playing)||(voice1->playing))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

READ_LINE_MEMBER( okim6376_device::nar_r )
{
	MSM6376LOG(("OKIM6376:'%s' NAR %x\n",tag(),m_nar));
	return m_nar;
}

WRITE_LINE_MEMBER( okim6376_device::ch2_w )
{
	m_ch2_update = 0;//Clear flag
	MSM6376LOG(("OKIM6376:'%s' CH2 %x\n",tag(),state));

	if (m_ch2 != state)
	{
		m_ch2 = state;
		m_ch2_update = 1;
	}
	if((!m_ch2)&&(m_ch2_update))
	{
		m_st_pulses = 0;
		struct ADPCMVoice *voice0 = &m_voice[0];
		struct ADPCMVoice *voice1 = &m_voice[1];
		// We set to channel 2
		MSM6376LOG(("OKIM6376:'%s' Channel 1\n",tag()));
		m_channel = 1;

		if ((voice0->playing)&&(m_st))
		{
			//Echo functions when Channel 1 is playing, and ST is still high
			m_command[1] = m_command[0];//copy sample over
			voice1->volume = volume_table[1]; //echo is 6dB attenuated
		}
	}

	if((m_ch2)&&(m_ch2_update))
	{
		m_stage[1]=0;
		oki_process(1, m_command[1]);
		MSM6376LOG(("OKIM6376:'%s' Channel 0\n",tag()));
		m_channel = 0;
	}
}


WRITE_LINE_MEMBER( okim6376_device::st_w )
{
	//As in STart, presumably, this triggers everything

	m_st_update = 0;//Clear flag
	MSM6376LOG(("OKIM6376:'%s' ST %x\n",tag(),state));

	if (m_st != state)
	{
		m_st = state;
		m_st_update = 1;

		if ((m_channel == 1) & !m_st)//ST acts as attenuation for Channel 2 when low, and stays at that level until the channel is reset
		{
			struct ADPCMVoice *voice = &m_voice[m_channel];
			{
				m_st_pulses ++;
				MSM6376LOG(("OKIM6376:'%s' ST pulses %x\n",tag(),m_st_pulses));
				if (m_st_pulses > 3)
				{
					m_st_pulses = 3; //undocumented behaviour beyond 3 pulses
				}

				voice->volume = volume_table[m_st_pulses - 1];
			}
		}
		if (m_st && m_st_update)
		{
			m_command[m_channel] = m_latch;
			if (m_channel ==0 && m_nar)
			{
				m_stage[m_channel]=0;
				oki_process(0, m_command[0]);
				m_nar = 0;
				m_nartimer = 4;
				/*According to datasheet, NAR timing is ~375 us at 8KHz, and is inversely proportional to sample rate, effectively 6 stream updates. */
			}
		}
	}
}

/**********************************************************************************************

     okim6376_data_w -- write to the data port of an OKIM6376-compatible chip

***********************************************************************************************/

WRITE8_MEMBER( okim6376_device::write )
{
	// The data port is purely used to set the latch, everything else is started by an ST pulse

	m_latch = data & 0x7f;
	// FIX: maximum adpcm words are 111, there are other 8 commands to generate BEEP tone (0x70 to 0x77),
	// and others for internal testing, that manual explicitly says not to use (0x78 to 0x7f)
	// We aren't doing anything with the BEEP at the moment, as we'd need to mix the ADPCM stream with beep.c
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void okim6376_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	for (i = 0; i < OKIM6376_VOICES; i++)
	{
		struct ADPCMVoice *voice = &m_voice[i];
		stream_sample_t *buffer = outputs[0];
		INT16 sample_data[MAX_SAMPLE_CHUNK];
		int remaining = samples;
		if (i == 0) //channel 1 is the only channel to affect NAR
		{
			if (m_nartimer > 0)
			{
				m_nartimer--;
				if (!m_nartimer)
				{
					m_nar =1;
				}
			}
		}

		/* loop while we have samples remaining */
		while (remaining)
		{
			int samples = (remaining > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : remaining;
			int samp;

			generate_adpcm(voice, sample_data, samples,i);
			for (samp = 0; samp < samples; samp++)
				*buffer++ += sample_data[samp];

			remaining -= samples;
		}
	}
}
