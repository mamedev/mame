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

//#define VERBOSE 1
#include "logmacro.h"

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

static void compute_tables()
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

void okim6376_device::ADPCMVoice::reset()
{
	/* make sure we have our tables */
	if (!tables_computed)
		compute_tables();

	/* reset the signal/step */
	signal = -2;
	step = 0;
}


DEFINE_DEVICE_TYPE(OKIM6376, okim6376_device, "okim6376", "OKI MSM6376 ADPCM")
DEFINE_DEVICE_TYPE(OKIM6650, okim6650_device, "okim6650", "OKI MSM6650 ADPCM")

okim6376_device::okim6376_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbits)
	: device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		device_rom_interface(mconfig, *this),
		//m_command[OKIM6376_VOICES],
		m_latch(0),
		//m_stage[OKIM6376_VOICES],
		m_stream(nullptr),
		m_divisor(8),
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
	override_address_width(addrbits);
}

okim6376_device::okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: okim6376_device(mconfig, OKIM6376, tag, owner, clock, 21)
{
}

okim6650_device::okim6650_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: okim6376_device(mconfig, OKIM6650, tag, owner, clock, 23)
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
	m_stream = stream_alloc(0, 1, get_sample_rate());

	/* initialize the voices */
	for (voice = 0; voice < OKIM6376_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		m_voice[voice].volume = 0;
		m_voice[voice].reset();
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


void okim6376_device::rom_bank_pre_change()
{
	m_stream->update();
}


/**********************************************************************************************

     clock_adpcm -- clock the next ADPCM byte

***********************************************************************************************/

int16_t okim6376_device::ADPCMVoice::clock(uint8_t nibble)
{
	signal += diff_lookup[step * 16 + (nibble & 15)];

	/* clamp to the maximum 12bit */
	if (signal > 2047)
		signal = 2047;
	else if (signal < -2048)
		signal = -2048;

	/* adjust the step size and clamp */
	step += index_shift[nibble & 7];
	if (step > 48)
		step = 48;
	else if (step < 0)
		step = 0;

	/* return the signal */
	return signal;
}


offs_t okim6376_device::get_start_position(int channel)
{
	offs_t base = m_command[channel] * 4;

	// max address space is 16Mbit
	return (read_byte(base+0) << 16 | read_byte(base+1) << 8 | read_byte(base+2)) & 0x1fffff;
}


offs_t okim6650_device::get_start_position(int channel)
{
	offs_t base = 0x000800 + m_command[channel] * 4;

	// determine sampling frequency for phrase
	uint8_t data = read_byte(base);
	m_divisor = ((data & 3) == 2 ? 5 : 8 - (data & 3) * 2) * (BIT(data, 2) ? 2 : 1);
	notify_clock_changed();

	// max address space is 64Mbit
	return (read_byte(base+1) << 16 | read_byte(base+2) << 8 | read_byte(base+3)) & 0x7fffff;
}


u32 okim6376_device::get_sample_rate()
{
	return clock() / m_divisor;
}

u32 okim6650_device::get_sample_rate()
{
	return clock() / 64 / m_divisor;
}



void okim6376_device::oki_process(int channel, int command)
{
	/* if a command is pending, process the second half */
	if ((command != -1) && (command != 0)) //process silence separately
	{
		/* update the stream */
		m_stream->update();

		/* determine which voice(s) (voice is set by the state of 2CH) */
		{
			struct ADPCMVoice *voice = &m_voice[channel];

			// determine the start position
			offs_t start = get_start_position(channel);

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
					voice->reset();
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

void okim6376_device::generate_adpcm(struct ADPCMVoice *voice, int16_t *buffer, int samples,int channel)
{
	/* if this voice is active */
	if (voice->playing)
	{
		offs_t base = voice->base_offset;
		int sample = voice->sample;
		int count = voice->count;

		/* loop while we still have samples to generate */
		while (samples)
		{
			int nibble;

			if (count == 0)
			{
				/* get the number of samples to play */
				count = (read_byte(base + sample / 2) & 0x7f) << 1;

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
			nibble = read_byte(base + sample / 2) >> (((sample & 1) << 2) ^ 4);

			/* output to the buffer, scaling by the volume */
			/* signal in range -4096..4095, volume in range 2..16 => signal * volume / 2 in range -32768..32767 */
			*buffer++ = voice->clock(nibble) * voice->volume / 2;

			++sample;
			--count;
			--samples;
		}

		/* update the parameters */
		voice->sample = sample;
		voice->count = count;
	}

	if ((!voice->playing)&&(m_stage[channel]))//end of samples, load anything staged in
	{
		m_stage[channel] = 0;
		oki_process(channel,m_command[channel]);
	}
}


/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

void okim6376_device::device_post_load()
{
	notify_clock_changed();
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
	for (int j = 0; j < OKIM6376_VOICES; j++)
	{
		adpcm_state_save_register(&m_voice[j], j);
	}

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
}

void okim6376_device::device_clock_changed()
{
	m_stream->set_sample_rate(get_sample_rate());
}


/**********************************************************************************************

     okim6376_status_r -- read the status port of an OKIM6376-compatible chip

***********************************************************************************************/

int okim6376_device::busy_r()
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

int okim6376_device::nar_r()
{
	LOG("OKIM6376: NAR %x\n",m_nar);
	return m_nar;
}

void okim6376_device::ch2_w(int state)
{
	m_ch2_update = 0;//Clear flag
	LOG("OKIM6376: CH2 %x\n",state);

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
		LOG("OKIM6376: Channel 1\n");
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
		LOG("OKIM6376: Channel 0\n");
		m_channel = 0;
	}
}


void okim6376_device::st_w(int state)
{
	//As in STart, presumably, this triggers everything

	m_st_update = 0;//Clear flag
	LOG("OKIM6376: ST %x\n",state);

	if (m_st != state)
	{
		m_st = state;
		m_st_update = 1;

		if ((m_channel == 1) & !m_st)//ST acts as attenuation for Channel 2 when low, and stays at that level until the channel is reset
		{
			struct ADPCMVoice *voice = &m_voice[m_channel];
			{
				m_st_pulses ++;
				LOG("OKIM6376: ST pulses %x\n",m_st_pulses);
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


void okim6650_device::cmd_w(int state)
{
	// TODO
}

/**********************************************************************************************

     okim6376_data_w -- write to the data port of an OKIM6376-compatible chip

***********************************************************************************************/

void okim6376_device::write(uint8_t data)
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

void okim6376_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < OKIM6376_VOICES; i++)
	{
		struct ADPCMVoice *voice = &m_voice[i];
		int16_t sample_data[MAX_SAMPLE_CHUNK];
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
		for (int sampindex = 0; sampindex < stream.samples(); )
		{
			int remaining = stream.samples() - sampindex;
			int samples = (remaining > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : remaining;

			generate_adpcm(voice, sample_data, samples,i);
			for (int samp = 0; samp < samples; samp++)
				stream.add_int(0, sampindex + samp, sample_data[samp], 32768);

			sampindex += samples;
		}
	}
}
