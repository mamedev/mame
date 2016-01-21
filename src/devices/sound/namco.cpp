// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/***************************************************************************

    NAMCO sound driver.

    This driver handles the four known types of NAMCO wavetable sounds:

        - 3-voice mono (PROM-based design: Pac-Man, Pengo, Dig Dug, etc)
        - 8-voice quadrophonic (Pole Position 1, Pole Position 2)
        - 8-voice mono (custom 15XX: Mappy, Dig Dug 2, etc)
        - 8-voice stereo (System 1)

***************************************************************************/

#include "emu.h"
#include "namco.h"


/* quality parameter: internal sample rate is 192 KHz, output is 48 KHz */
#define INTERNAL_RATE   192000

/* 16 bits: sample bits of the stream buffer    */
/* 4 bits:  volume                  */
/* 4 bits:  prom sample bits            */
#define MIXLEVEL    (1 << (16 - 4 - 4))

/* stream output level */
#define OUTPUT_LEVEL(n)     ((n) * MIXLEVEL / m_voices)

/* a position of waveform sample */
#define WAVEFORM_POSITION(n)    (((n) >> m_f_fracbits) & 0x1f)

const device_type NAMCO = &device_creator<namco_device>;
const device_type NAMCO_15XX = &device_creator<namco_15xx_device>;
const device_type NAMCO_CUS30 = &device_creator<namco_cus30_device>;

namco_audio_device::namco_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
		device_sound_interface(mconfig, *this),
		m_last_channel(nullptr),
		m_soundregs(nullptr),
		m_wavedata(nullptr),
		m_wave_size(0),
		m_sound_enable(0),
		m_stream(nullptr),
		m_namco_clock(0),
		m_sample_rate(0),
		m_f_fracbits(0),
		m_voices(0),
		m_stereo(0)
{
}

namco_device::namco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: namco_audio_device(mconfig, NAMCO, "Namco", tag, owner, clock, "namco", __FILE__)
{
}

namco_15xx_device::namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:namco_audio_device(mconfig, NAMCO_15XX, "Namco 15XX", tag, owner, clock, "namco_15xx", __FILE__)
{
}

namco_cus30_device::namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: namco_audio_device(mconfig, NAMCO_CUS30, "Namco CUS30", tag, owner, clock, "namco_cus30", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_audio_device::device_start()
{
	sound_channel *voice;
	int clock_multiple;

	/* extract globals from the interface */
	m_last_channel = m_channel_list + m_voices;

	m_soundregs = auto_alloc_array_clear(machine(), UINT8, 0x400);

	/* adjust internal clock */
	m_namco_clock = clock();
	for (clock_multiple = 0; m_namco_clock < INTERNAL_RATE; clock_multiple++)
		m_namco_clock *= 2;

	m_f_fracbits = clock_multiple + 15;

	/* adjust output clock */
	m_sample_rate = m_namco_clock;

	logerror("Namco: freq fractional bits = %d: internal freq = %d, output freq = %d\n", m_f_fracbits, m_namco_clock, m_sample_rate);

	/* build the waveform table */
	build_decoded_waveform(region()->base());

	/* get stream channels */
	if (m_stereo)
		m_stream = machine().sound().stream_alloc(*this, 0, 2, m_sample_rate);
	else
		m_stream = machine().sound().stream_alloc(*this, 0, 1, m_sample_rate);

	/* start with sound enabled, many games don't have a sound enable register */
	m_sound_enable = 1;

	/* register with the save state system */
	save_pointer(NAME(m_soundregs), 0x400);

	if (region() == nullptr)
		save_pointer(NAME(m_wavedata), 0x400);

	save_item(NAME(m_voices));
	save_item(NAME(m_sound_enable));
	save_pointer(NAME(m_waveform[0]), MAX_VOLUME * 32 * 8 * (1+m_wave_size));

	/* reset all the voices */
	for (voice = m_channel_list; voice < m_last_channel; voice++)
	{
		int voicenum = voice - m_channel_list;

		voice->frequency = 0;
		voice->volume[0] = voice->volume[1] = 0;
		voice->waveform_select = 0;
		voice->counter = 0;
		voice->noise_sw = 0;
		voice->noise_state = 0;
		voice->noise_seed = 1;
		voice->noise_counter = 0;
		voice->noise_hold = 0;

		/* register with the save state system */
		save_item(NAME(voice->frequency), voicenum);
		save_item(NAME(voice->counter), voicenum);
		save_item(NAME(voice->volume), voicenum);
		save_item(NAME(voice->noise_sw), voicenum);
		save_item(NAME(voice->noise_state), voicenum);
		save_item(NAME(voice->noise_seed), voicenum);
		save_item(NAME(voice->noise_hold), voicenum);
		save_item(NAME(voice->noise_counter), voicenum);
		save_item(NAME(voice->waveform_select), voicenum);
	}
}



/* update the decoded waveform data */
void namco_audio_device::update_namco_waveform(int offset, UINT8 data)
{
	if (m_wave_size == 1)
	{
		INT16 wdata;
		int v;

		/* use full byte, first 4 high bits, then low 4 bits */
		for (v = 0; v < MAX_VOLUME; v++)
		{
			wdata = ((data >> 4) & 0x0f) - 8;
			m_waveform[v][offset * 2] = OUTPUT_LEVEL(wdata * v);
			wdata = (data & 0x0f) - 8;
			m_waveform[v][offset * 2 + 1] = OUTPUT_LEVEL(wdata * v);
		}
	}
	else
	{
		int v;

		/* use only low 4 bits */
		for (v = 0; v < MAX_VOLUME; v++)
			m_waveform[v][offset] = OUTPUT_LEVEL(((data & 0x0f) - 8) * v);
	}
}


/* build the decoded waveform table */
void namco_audio_device::build_decoded_waveform(UINT8 *rgnbase)
{
	INT16 *p;
	int size;
	int offset;
	int v;

	m_wavedata = (rgnbase != nullptr) ? rgnbase : auto_alloc_array_clear(machine(), UINT8, 0x400);

	/* 20pacgal has waves in RAM but old sound system */
	if (rgnbase == nullptr && m_voices != 3)
	{
		m_wave_size = 1;
		size = 32 * 16;     /* 32 samples, 16 waveforms */
	}
	else
	{
		m_wave_size = 0;
		size = 32 * 8;      /* 32 samples, 8 waveforms */
	}

	p = auto_alloc_array(machine(), INT16, size * MAX_VOLUME);

	for (v = 0; v < MAX_VOLUME; v++)
	{
		m_waveform[v] = p;
		p += size;
	}

	/* We need waveform data. It fails if region is not specified. */
	if (m_wavedata)
	{
		for (offset = 0; offset < 256; offset++)
			update_namco_waveform(offset, m_wavedata[offset]);
	}
}


/* generate sound by oversampling */
UINT32 namco_audio_device::namco_update_one(stream_sample_t *buffer, int length, const INT16 *wave, UINT32 counter, UINT32 freq)
{
	while (length-- > 0)
	{
		*buffer++ += wave[WAVEFORM_POSITION(counter)];
		counter += freq;
	}

	return counter;
}


/********************************************************************************/

/* pacman register map
    0x05:       ch 0    waveform select
    0x0a:       ch 1    waveform select
    0x0f:       ch 2    waveform select

    0x10:       ch 0    the first voice has extra frequency bits
    0x11-0x14:  ch 0    frequency
    0x15:       ch 0    volume

    0x16-0x19:  ch 1    frequency
    0x1a:       ch 1    volume

    0x1b-0x1e:  ch 2    frequency
    0x1f:       ch 2    volume
*/

WRITE8_MEMBER( namco_device::pacman_sound_enable_w )
{
	m_sound_enable = data;
}

WRITE8_MEMBER( namco_device::pacman_sound_w )
{
	sound_channel *voice;
	int ch;

	data &= 0x0f;
	if (m_soundregs[offset] == data)
		return;

	/* update the streams */
	m_stream->update();

	/* set the register */
	m_soundregs[offset] = data;

	if (offset < 0x10)
		ch = (offset - 5) / 5;
	else if (offset == 0x10)
		ch = 0;
	else
		ch = (offset - 0x11) / 5;

	if (ch >= m_voices)
		return;

	/* recompute the voice parameters */
	voice = m_channel_list + ch;
	switch (offset - ch * 5)
	{
	case 0x05:
		voice->waveform_select = data & 7;
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
		/* the frequency has 20 bits */
		/* the first voice has extra frequency bits */
		voice->frequency = (ch == 0) ? m_soundregs[0x10] : 0;
		voice->frequency += (m_soundregs[ch * 5 + 0x11] << 4);
		voice->frequency += (m_soundregs[ch * 5 + 0x12] << 8);
		voice->frequency += (m_soundregs[ch * 5 + 0x13] << 12);
		voice->frequency += (m_soundregs[ch * 5 + 0x14] << 16); /* always 0 */
		break;

	case 0x15:
		voice->volume[0] = data;
		break;
	}
}

WRITE8_MEMBER( namco_cus30_device::pacman_sound_w )
{
	sound_channel *voice;
	int ch;

	data &= 0x0f;
	if (m_soundregs[offset] == data)
		return;

	/* update the streams */
	m_stream->update();

	/* set the register */
	m_soundregs[offset] = data;

	if (offset < 0x10)
		ch = (offset - 5) / 5;
	else if (offset == 0x10)
		ch = 0;
	else
		ch = (offset - 0x11) / 5;

	if (ch >= m_voices)
		return;

	/* recompute the voice parameters */
	voice = m_channel_list + ch;
	switch (offset - ch * 5)
	{
	case 0x05:
		voice->waveform_select = data & 7;
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
		/* the frequency has 20 bits */
		/* the first voice has extra frequency bits */
		voice->frequency = (ch == 0) ? m_soundregs[0x10] : 0;
		voice->frequency += (m_soundregs[ch * 5 + 0x11] << 4);
		voice->frequency += (m_soundregs[ch * 5 + 0x12] << 8);
		voice->frequency += (m_soundregs[ch * 5 + 0x13] << 12);
		voice->frequency += (m_soundregs[ch * 5 + 0x14] << 16); /* always 0 */
		break;

	case 0x15:
		voice->volume[0] = data;
		break;
	}
}

/********************************************************************************/

/* polepos register map
Note: even if there are 8 voices, the game doesn't use the first 2 because
it select the 54XX/52XX outputs on those channels

    0x00-0x01   ch 0    frequency
    0x02        ch 0    xxxx---- GAIN 2 volume
    0x03        ch 0    xxxx---- GAIN 3 volume
                        ----xxxx GAIN 4 volume

    0x04-0x07   ch 1

    .
    .
    .

    0x1c-0x1f   ch 7

    0x23        ch 0    xxxx---- GAIN 1 volume
                        -----xxx waveform select
                        ----x-xx channel output select
                                 0-7 (all the same, shared with waveform select) = wave
                                 8 = CHANL1 (54XX pins 17-20)
                                 9 = CHANL2 (54XX pins 8-11)
                                 A = CHANL3 (54XX pins 4-7)
                                 B = CHANL4 (52XX)
    0x27        ch 1
    0x2b        ch 2
    0x2f        ch 3
    0x33        ch 4
    0x37        ch 5
    0x3b        ch 6
    0x3f        ch 7
*/

void namco_device::polepos_sound_enable(int enable)
{
	m_sound_enable = enable;
}

READ8_MEMBER( namco_device::polepos_sound_r )
{
	return m_soundregs[offset];
}

WRITE8_MEMBER( namco_device::polepos_sound_w )
{
	sound_channel *voice;
	int ch;

	if (m_soundregs[offset] == data)
		return;

	/* update the streams */
	m_stream->update();

	/* set the register */
	m_soundregs[offset] = data;

	ch = (offset & 0x1f) / 4;

	/* recompute the voice parameters */
	voice = m_channel_list + ch;
	switch (offset & 0x23)
	{
	case 0x00:
	case 0x01:
		/* the frequency has 16 bits */
		voice->frequency = m_soundregs[ch * 4 + 0x00];
		voice->frequency += m_soundregs[ch * 4 + 0x01] << 8;
		break;

	case 0x23:
		voice->waveform_select = data & 7;
		/* fall through */
	case 0x02:
	case 0x03:
		voice->volume[0] = voice->volume[1] = 0;
		// front speakers ?
		voice->volume[0] += m_soundregs[ch * 4 + 0x03] >> 4;
		voice->volume[1] += m_soundregs[ch * 4 + 0x03] & 0x0f;
		// rear speakers ?
		voice->volume[0] += m_soundregs[ch * 4 + 0x23] >> 4;
		voice->volume[1] += m_soundregs[ch * 4 + 0x02] >> 4;

		voice->volume[0] /= 2;
		voice->volume[1] /= 2;

		/* if 54XX or 52XX selected, silence this voice */
		if (m_soundregs[ch * 4 + 0x23] & 8)
			voice->volume[0] = voice->volume[1] = 0;
		break;
	}
}


/********************************************************************************/

/* 15XX register map
    0x03        ch 0    volume
    0x04-0x05   ch 0    frequency
    0x06        ch 0    waveform select & frequency

    0x0b        ch 1    volume
    0x0c-0x0d   ch 1    frequency
    0x0e        ch 1    waveform select & frequency

    .
    .
    .

    0x3b        ch 7    volume
    0x3c-0x3d   ch 7    frequency
    0x3e        ch 7    waveform select & frequency
*/

void namco_15xx_device::mappy_sound_enable(int enable)
{
	m_sound_enable = enable;
}

WRITE8_MEMBER(namco_15xx_device::namco_15xx_w)
{
	sound_channel *voice;
	int ch;

	if (m_soundregs[offset] == data)
		return;

	/* update the streams */
	m_stream->update();

	/* set the register */
	m_soundregs[offset] = data;

	ch = offset / 8;
	if (ch >= m_voices)
		return;

	/* recompute the voice parameters */
	voice = m_channel_list + ch;
	switch (offset - ch * 8)
	{
	case 0x03:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x06:
		voice->waveform_select = (data >> 4) & 7;
	case 0x04:
	case 0x05:
		/* the frequency has 20 bits */
		voice->frequency = m_soundregs[ch * 8 + 0x04];
		voice->frequency += m_soundregs[ch * 8 + 0x05] << 8;
		voice->frequency += (m_soundregs[ch * 8 + 0x06] & 15) << 16;    /* high bits are from here */
		break;
	}
}


/********************************************************************************/

/* namcos1 register map
    0x00        ch 0    left volume
    0x01        ch 0    waveform select & frequency
    0x02-0x03   ch 0    frequency
    0x04        ch 0    right volume AND
    0x04        ch 1    noise sw

    0x08        ch 1    left volume
    0x09        ch 1    waveform select & frequency
    0x0a-0x0b   ch 1    frequency
    0x0c        ch 1    right volume AND
    0x0c        ch 2    noise sw

    .
    .
    .

    0x38        ch 7    left volume
    0x39        ch 7    waveform select & frequency
    0x3a-0x3b   ch 7    frequency
    0x3c        ch 7    right volume AND
    0x3c        ch 0    noise sw
*/

	WRITE8_MEMBER(namco_cus30_device::namcos1_sound_w)
{
	sound_channel *voice;
	int ch;
	int nssw;


	/* verify the offset */
	if (offset > 63)
	{
		logerror("NAMCOS1 sound: Attempting to write past the 64 registers segment\n");
		return;
	}

	m_soundregs = m_wavedata + 0x100;

	if (m_soundregs[offset] == data)
		return;

	/* update the streams */
	m_stream->update();

	/* set the register */
	m_soundregs[offset] = data;

	ch = offset / 8;
	if (ch >= m_voices)
		return;

	/* recompute the voice parameters */
	voice = m_channel_list + ch;
	switch (offset - ch * 8)
	{
	case 0x00:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x01:
		voice->waveform_select = (data >> 4) & 15;
	case 0x02:
	case 0x03:
		/* the frequency has 20 bits */
		voice->frequency = (m_soundregs[ch * 8 + 0x01] & 15) << 16; /* high bits are from here */
		voice->frequency += m_soundregs[ch * 8 + 0x02] << 8;
		voice->frequency += m_soundregs[ch * 8 + 0x03];
		break;

	case 0x04:
		voice->volume[1] = data & 0x0f;

		nssw = ((data & 0x80) >> 7);
		if (++voice == m_last_channel)
			voice = m_channel_list;
		voice->noise_sw = nssw;
		break;
	}
}

WRITE8_MEMBER( namco_cus30_device::namcos1_cus30_w )
{
	if (offset < 0x100)
	{
		if (m_wavedata[offset] != data)
		{
			/* update the streams */
			m_stream->update();

			m_wavedata[offset] = data;

			/* update the decoded waveform table */
			update_namco_waveform(offset, data);
		}
	}
	else if (offset < 0x140)
		namcos1_sound_w(space, offset - 0x100,data);
	else
		m_wavedata[offset] = data;
}

READ8_MEMBER( namco_cus30_device::namcos1_cus30_r )
{
	return m_wavedata[offset];
}

READ8_MEMBER( namco_15xx_device::sharedram_r )
{
	return m_soundregs[offset];
}

WRITE8_MEMBER( namco_15xx_device::sharedram_w )
{
	if (offset < 0x40)
		namco_15xx_w(space, offset, data);
	else
	{
		m_soundregs[offset] = data;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void namco_audio_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	if (m_stereo)
	{
		sound_channel *voice;

		/* zap the contents of the buffers */
		memset(outputs[0], 0, samples * sizeof(*outputs[0]));
		memset(outputs[1], 0, samples * sizeof(*outputs[1]));

		/* if no sound, we're done */
		if (m_sound_enable == 0)
			return;

		/* loop over each voice and add its contribution */
		for (voice = m_channel_list; voice < m_last_channel; voice++)
		{
			stream_sample_t *lmix = outputs[0];
			stream_sample_t *rmix = outputs[1];
			int lv = voice->volume[0];
			int rv = voice->volume[1];

			if (voice->noise_sw)
			{
				int f = voice->frequency & 0xff;

				/* only update if we have non-zero volume and frequency */
				if ((lv || rv) && f)
				{
					int hold_time = 1 << (m_f_fracbits - 16);
					int hold = voice->noise_hold;
					UINT32 delta = f << 4;
					UINT32 c = voice->noise_counter;
					INT16 l_noise_data = OUTPUT_LEVEL(0x07 * (lv >> 1));
					INT16 r_noise_data = OUTPUT_LEVEL(0x07 * (rv >> 1));
					int i;

					/* add our contribution */
					for (i = 0; i < samples; i++)
					{
						int cnt;

						if (voice->noise_state)
						{
							*lmix++ += l_noise_data;
							*rmix++ += r_noise_data;
						}
						else
						{
							*lmix++ -= l_noise_data;
							*rmix++ -= r_noise_data;
						}

						if (hold)
						{
							hold--;
							continue;
						}

						hold =  hold_time;

						c += delta;
						cnt = (c >> 12);
						c &= (1 << 12) - 1;
						for( ;cnt > 0; cnt--)
						{
							if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
							if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
							voice->noise_seed >>= 1;
						}
					}

					/* update the counter and hold time for this voice */
					voice->noise_counter = c;
					voice->noise_hold = hold;
				}
			}
			else
			{
				/* only update if we have non-zero frequency */
				if (voice->frequency)
				{
					/* save the counter for this voice */
					UINT32 c = voice->counter;

					/* only update if we have non-zero left volume */
					if (lv)
					{
						const INT16 *lw = &m_waveform[lv][voice->waveform_select * 32];

						/* generate sound into the buffer */
						c = namco_update_one(lmix, samples, lw, voice->counter, voice->frequency);
					}

					/* only update if we have non-zero right volume */
					if (rv)
					{
						const INT16 *rw = &m_waveform[rv][voice->waveform_select * 32];

						/* generate sound into the buffer */
						c = namco_update_one(rmix, samples, rw, voice->counter, voice->frequency);
					}

					/* update the counter for this voice */
					voice->counter = c;
				}
			}
		}
	}
	else
	{
		sound_channel *voice;

		stream_sample_t *buffer = outputs[0];
		/* zap the contents of the buffer */
		memset(buffer, 0, samples * sizeof(*buffer));

		/* if no sound, we're done */

		if (m_sound_enable == 0)
		return;

		/* loop over each voice and add its contribution */
		for (voice = m_channel_list; voice < m_last_channel; voice++)
		{
			stream_sample_t *mix = buffer;
			int v = voice->volume[0];
				if (voice->noise_sw)
				{
				int f = voice->frequency & 0xff;
					/* only update if we have non-zero volume and frequency */
				if (v && f)
				{
						int hold_time = 1 << (m_f_fracbits - 16);
						int hold = voice->noise_hold;
						UINT32 delta = f << 4;
						UINT32 c = voice->noise_counter;
						INT16 noise_data = OUTPUT_LEVEL(0x07 * (v >> 1));
						int i;

						/* add our contribution */
						for (i = 0; i < samples; i++)
						{
							int cnt;

							if (voice->noise_state)
								*mix++ += noise_data;
							else
								*mix++ -= noise_data;

							if (hold)
							{
								hold--;
								continue;
							}

							hold =  hold_time;

							c += delta;
							cnt = (c >> 12);
							c &= (1 << 12) - 1;
							for( ;cnt > 0; cnt--)
							{
								if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
								if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
								voice->noise_seed >>= 1;
							}
						}

						/* update the counter and hold time for this voice */
						voice->noise_counter = c;
						voice->noise_hold = hold;
					}
				}
				else
				{
				/* only update if we have non-zero volume and frequency */
				if (v && voice->frequency)
				{
					const INT16 *w = &m_waveform[v][voice->waveform_select * 32];

					/* generate sound into buffer and update the counter for this voice */
					voice->counter = namco_update_one(mix, samples, w, voice->counter, voice->frequency);
				}
			}
		}
	}
}

void namco_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	namco_audio_device::sound_stream_update(stream, inputs, outputs, samples);
}

void namco_15xx_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	namco_audio_device::sound_stream_update(stream, inputs, outputs, samples);
}

void namco_cus30_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	namco_audio_device::sound_stream_update(stream, inputs, outputs, samples);
}
