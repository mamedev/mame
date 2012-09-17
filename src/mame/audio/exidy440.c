/***************************************************************************

    Exidy 440 sound system

    Special thanks to Zonn Moore and Neil Bradley for letting me hack
    their Retrocade CVSD decoder into the sound system here.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "audio/exidy440.h"


#define	SOUND_LOG		0
#define	FADE_TO_ZERO	1


#define EXIDY440_AUDIO_CLOCK	(XTAL_12_9792MHz / 16)
#define EXIDY440_MC3418_CLOCK	(EXIDY440_AUDIO_CLOCK / 16)
#define EXIDY440_MC3417_CLOCK	(EXIDY440_AUDIO_CLOCK / 32)


/* internal caching */
#define	MAX_CACHE_ENTRIES		1024				/* maximum separate samples we expect to ever see */
#define	SAMPLE_BUFFER_LENGTH	1024				/* size of temporary decode buffer on the stack */

/* FIR digital filter parameters */
#define	FIR_HISTORY_LENGTH		57					/* number of FIR coefficients */

/* CVSD decoding parameters */
#define	INTEGRATOR_LEAK_TC		(10e3 * 0.1e-6)
#define	FILTER_DECAY_TC			((18e3 + 3.3e3) * 0.33e-6)
#define	FILTER_CHARGE_TC		(18e3 * 0.33e-6)
#define	FILTER_MIN				0.0416
#define	FILTER_MAX				1.0954
#define	SAMPLE_GAIN				10000.0


/* channel_data structure holds info about each 6844 DMA channel */
struct m6844_channel_data
{
	int active;
	int address;
	int counter;
	UINT8 control;
	int start_address;
	int start_counter;
};


/* channel_data structure holds info about each active sound channel */
struct sound_channel_data
{
	INT16 *base;
	int offset;
	int remaining;
};


/* sound_cache_entry structure contains info on each decoded sample */
struct sound_cache_entry
{
	struct sound_cache_entry *next;
	int address;
	int length;
	int bits;
	int frequency;
	INT16 data[1];
};



struct exidy440_audio_state
{
	UINT8 sound_command;
	UINT8 sound_command_ack;

	UINT8 sound_banks[4];
	UINT8 m6844_data[0x20];
	UINT8 sound_volume[0x10];
	INT32 *mixer_buffer_left;
	INT32 *mixer_buffer_right;
	sound_cache_entry *sound_cache;
	sound_cache_entry *sound_cache_end;
	sound_cache_entry *sound_cache_max;

	/* 6844 description */
	m6844_channel_data m6844_channel[4];
	UINT8 m6844_priority;
	UINT8 m6844_interrupt;
	UINT8 m6844_chain;

	/* sound interface parameters */
	sound_stream *stream;
	sound_channel_data sound_channel[4];

	/* debugging */
	FILE *debuglog;

	/* channel frequency is configurable */
	int channel_frequency[4];
};

/* constant channel parameters */
static const int channel_bits[4] =
{
	4, 4,									/* channels 0 and 1 are MC3418s, 4-bit CVSD */
	3, 3									/* channels 2 and 3 are MC3417s, 3-bit CVSD */
};


/* function prototypes */
static STREAM_UPDATE( channel_update );
static void m6844_finished(m6844_channel_data *channel);
static void play_cvsd(device_t *device, int ch);
static void stop_cvsd(device_t *device, int ch);

static void reset_sound_cache(device_t *device);
static INT16 *add_to_sound_cache(device_t *device, UINT8 *input, int address, int length, int bits, int frequency);
static INT16 *find_or_add_to_sound_cache(device_t *device, int address, int length, int bits, int frequency);

static void decode_and_filter_cvsd(device_t *device, UINT8 *data, int bytes, int maskbits, int frequency, INT16 *dest);
static void fir_filter(device_t *device, INT32 *input, INT16 *output, int count);


class exidy440_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	exidy440_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~exidy440_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type EXIDY440;


/*************************************
 *
 *  Initialize the sound system
 *
 *************************************/

INLINE exidy440_audio_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
        assert(device->type() == EXIDY440);

        return (exidy440_audio_state *)downcast<exidy440_sound_device *>(device)->token();
}

static DEVICE_START( exidy440_sound )
{
	exidy440_audio_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	int i, length;

	/* reset the system */
	state->sound_command = 0;
	state->sound_command_ack = 1;
	state_save_register_global(machine, state->sound_command);
	state_save_register_global(machine, state->sound_command_ack);

	/* reset the 6844 */
	for (i = 0; i < 4; i++)
	{
		state->m6844_channel[i].active = 0;
		state->m6844_channel[i].control = 0x00;
	}
	state->m6844_priority = 0x00;
	state->m6844_interrupt = 0x00;
	state->m6844_chain = 0x00;

	state_save_register_global(machine, state->m6844_priority);
	state_save_register_global(machine, state->m6844_interrupt);
	state_save_register_global(machine, state->m6844_chain);

	state->channel_frequency[0] = device->clock();   /* channels 0 and 1 are run by FCLK */
	state->channel_frequency[1] = device->clock();
	state->channel_frequency[2] = device->clock()/2; /* channels 2 and 3 are run by SCLK */
	state->channel_frequency[3] = device->clock()/2;

	/* get stream channels */
	state->stream = device->machine().sound().stream_alloc(*device, 0, 2, device->clock(), NULL, channel_update);

	/* allocate the sample cache */
	length = machine.root_device().memregion("cvsd")->bytes() * 16 + MAX_CACHE_ENTRIES * sizeof(sound_cache_entry);
	state->sound_cache = (sound_cache_entry *)auto_alloc_array(machine, UINT8, length);

	/* determine the hard end of the cache and reset */
	state->sound_cache_max = (sound_cache_entry *)((UINT8 *)state->sound_cache + length);
	reset_sound_cache(device);

	/* allocate the mixer buffer */
	state->mixer_buffer_left = auto_alloc_array(machine, INT32, 2 * device->clock());
	state->mixer_buffer_right = state->mixer_buffer_left + device->clock();

	if (SOUND_LOG)
		state->debuglog = fopen("sound.log", "w");
}



/*************************************
 *
 *  Tear down the sound system
 *
 *************************************/

static DEVICE_STOP( exidy440_sound )
{
	exidy440_audio_state *state = get_safe_token(device);
	if (SOUND_LOG && state->debuglog)
		fclose(state->debuglog);
}



/*************************************
 *
 *  Add a bunch of samples to the mix
 *
 *************************************/

static void add_and_scale_samples(device_t *device, int ch, INT32 *dest, int samples, int volume)
{
	exidy440_audio_state *state = get_safe_token(device);
	sound_channel_data *channel = &state->sound_channel[ch];
	INT16 *srcdata;
	int i;

	/* channels 2 and 3 are half-rate samples */
	if (ch & 2)
	{
		srcdata = &channel->base[channel->offset >> 1];

		/* handle the edge case */
		if (channel->offset & 1)
		{
			*dest++ += *srcdata++ * volume / 256;
			samples--;
		}

		/* copy 1 for 2 to the destination */
		for (i = 0; i < samples; i += 2)
		{
			INT16 sample = *srcdata++ * volume / 256;
			*dest++ += sample;
			*dest++ += sample;
		}
	}

	/* channels 0 and 1 are full-rate samples */
	else
	{
		srcdata = &channel->base[channel->offset];
		for (i = 0; i < samples; i++)
			*dest++ += *srcdata++ * volume / 256;
	}
}



/*************************************
 *
 *  Mix the result to 16 bits
 *
 *************************************/

static void mix_to_16(device_t *device, int length, stream_sample_t *dest_left, stream_sample_t *dest_right)
{
	exidy440_audio_state *state = get_safe_token(device);
	INT32 *mixer_left = state->mixer_buffer_left;
	INT32 *mixer_right = state->mixer_buffer_right;
	int i, clippers = 0;

	for (i = 0; i < length; i++)
	{
		INT32 sample_left = *mixer_left++;
		INT32 sample_right = *mixer_right++;

		if (sample_left < -32768) { sample_left = -32768; clippers++; }
		else if (sample_left > 32767) { sample_left = 32767; clippers++; }
		if (sample_right < -32768) { sample_right = -32768; clippers++; }
		else if (sample_right > 32767) { sample_right = 32767; clippers++; }

		*dest_left++ = sample_left;
		*dest_right++ = sample_right;
	}
}



/*************************************
 *
 *  Stream callback
 *
 *************************************/

static STREAM_UPDATE( channel_update )
{
	exidy440_audio_state *state = get_safe_token(device);
	int ch;

	/* reset the mixer buffers */
	memset(state->mixer_buffer_left, 0, samples * sizeof(INT32));
	memset(state->mixer_buffer_right, 0, samples * sizeof(INT32));

	/* loop over channels */
	for (ch = 0; ch < 4; ch++)
	{
		sound_channel_data *channel = &state->sound_channel[ch];
		int length, volume, left = samples;
		int effective_offset;

		/* if we're not active, bail */
		if (channel->remaining <= 0)
			continue;

		/* see how many samples to copy */
		length = (left > channel->remaining) ? channel->remaining : left;

		/* get a pointer to the sample data and copy to the left */
		volume = state->sound_volume[2 * ch + 0];
		if (volume)
			add_and_scale_samples(device, ch, state->mixer_buffer_left, length, volume);

		/* get a pointer to the sample data and copy to the left */
		volume = state->sound_volume[2 * ch + 1];
		if (volume)
			add_and_scale_samples(device, ch, state->mixer_buffer_right, length, volume);

		/* update our counters */
		channel->offset += length;
		channel->remaining -= length;
		left -= length;

		/* update the MC6844 */
		effective_offset = (ch & 2) ? channel->offset / 2 : channel->offset;
		state->m6844_channel[ch].address = state->m6844_channel[ch].start_address + effective_offset / 8;
		state->m6844_channel[ch].counter = state->m6844_channel[ch].start_counter - effective_offset / 8;
		if (state->m6844_channel[ch].counter <= 0)
		{
			if (SOUND_LOG && state->debuglog)
				fprintf(state->debuglog, "Channel %d finished\n", ch);
			m6844_finished(&state->m6844_channel[ch]);
		}
	}

	/* all done, time to mix it */
	mix_to_16(device, samples, outputs[0], outputs[1]);
}



/*************************************
 *
 *  Sound command register
 *
 *************************************/

static READ8_DEVICE_HANDLER( sound_command_r )
{
	exidy440_audio_state *state = get_safe_token(device);
	/* clear the FIRQ that got us here and acknowledge the read to the main CPU */
	space.machine().device("audiocpu")->execute().set_input_line(1, CLEAR_LINE);
	state->sound_command_ack = 1;

	return state->sound_command;
}


void exidy440_sound_command(device_t *device, UINT8 param)
{
	exidy440_audio_state *state = get_safe_token(device);
	state->sound_command = param;
	state->sound_command_ack = 0;
	device->machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}


UINT8 exidy440_sound_command_ack(device_t *device)
{
	exidy440_audio_state *state = get_safe_token(device);
	return state->sound_command_ack;
}



/*************************************
 *
 *  Sound volume registers
 *
 *************************************/

static READ8_DEVICE_HANDLER( sound_volume_r )
{
	exidy440_audio_state *state = get_safe_token(device);
	return state->sound_volume[offset];
}

static WRITE8_DEVICE_HANDLER( sound_volume_w )
{
	exidy440_audio_state *state = get_safe_token(device);
	if (SOUND_LOG && state->debuglog)
		fprintf(state->debuglog, "Volume %02X=%02X\n", offset, data);

	/* update the stream */
	state->stream->update();

	/* set the new volume */
	state->sound_volume[offset] = ~data;
}



/*************************************
 *
 *  Sound interrupt handling
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( sound_interrupt_clear_w )
{
	space.machine().device("audiocpu")->execute().set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  MC6844 DMA controller interface
 *
 *************************************/

static void m6844_update(device_t *device)
{
	exidy440_audio_state *state = get_safe_token(device);
	/* update the stream */
	state->stream->update();
}


static void m6844_finished(m6844_channel_data *channel)
{
	/* mark us inactive */
	channel->active = 0;

	/* set the final address and counter */
	channel->counter = 0;
	channel->address = channel->start_address + channel->start_counter;

	/* clear the DMA busy bit and set the DMA end bit */
	channel->control &= ~0x40;
	channel->control |= 0x80;
}



/*************************************
 *
 *  MC6844 DMA controller I/O
 *
 *************************************/

static READ8_DEVICE_HANDLER( m6844_r )
{
	exidy440_audio_state *state = get_safe_token(device);
	m6844_channel_data *m6844_channel = state->m6844_channel;
	int result = 0;

	/* first update the current state of the DMA transfers */
	m6844_update(device);

	/* switch off the offset we were given */
	switch (offset)
	{
		/* upper byte of address */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			result = m6844_channel[offset / 4].address >> 8;
			break;

		/* lower byte of address */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			result = m6844_channel[offset / 4].address & 0xff;
			break;

		/* upper byte of counter */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			result = m6844_channel[offset / 4].counter >> 8;
			break;

		/* lower byte of counter */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			result = m6844_channel[offset / 4].counter & 0xff;
			break;

		/* channel control */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			result = m6844_channel[offset - 0x10].control;

			/* a read here clears the DMA end flag */
			m6844_channel[offset - 0x10].control &= ~0x80;
			break;

		/* priority control */
		case 0x14:
			result = state->m6844_priority;
			break;

		/* interrupt control */
		case 0x15:

			/* update the global DMA end flag */
			state->m6844_interrupt &= ~0x80;
			state->m6844_interrupt |= (m6844_channel[0].control & 0x80) |
			                   (m6844_channel[1].control & 0x80) |
			                   (m6844_channel[2].control & 0x80) |
			                   (m6844_channel[3].control & 0x80);

			result = state->m6844_interrupt;
			break;

		/* chaining control */
		case 0x16:
			result = state->m6844_chain;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}

	return result;
}


static WRITE8_DEVICE_HANDLER( m6844_w )
{
	exidy440_audio_state *state = get_safe_token(device);
	m6844_channel_data *m6844_channel = state->m6844_channel;
	int i;

	/* first update the current state of the DMA transfers */
	m6844_update(device);

	/* switch off the offset we were given */
	switch (offset)
	{
		/* upper byte of address */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			m6844_channel[offset / 4].address = (m6844_channel[offset / 4].address & 0xff) | (data << 8);
			break;

		/* lower byte of address */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			m6844_channel[offset / 4].address = (m6844_channel[offset / 4].address & 0xff00) | (data & 0xff);
			break;

		/* upper byte of counter */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			m6844_channel[offset / 4].counter = (m6844_channel[offset / 4].counter & 0xff) | (data << 8);
			break;

		/* lower byte of counter */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			m6844_channel[offset / 4].counter = (m6844_channel[offset / 4].counter & 0xff00) | (data & 0xff);
			break;

		/* channel control */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			m6844_channel[offset - 0x10].control = (m6844_channel[offset - 0x10].control & 0xc0) | (data & 0x3f);
			break;

		/* priority control */
		case 0x14:
			state->m6844_priority = data;

			/* update the sound playback on each channel */
			for (i = 0; i < 4; i++)
			{
				/* if we're going active... */
				if (!m6844_channel[i].active && (data & (1 << i)))
				{
					/* mark us active */
					m6844_channel[i].active = 1;

					/* set the DMA busy bit and clear the DMA end bit */
					m6844_channel[i].control |= 0x40;
					m6844_channel[i].control &= ~0x80;

					/* set the starting address, counter, and time */
					m6844_channel[i].start_address = m6844_channel[i].address;
					m6844_channel[i].start_counter = m6844_channel[i].counter;

					/* generate and play the sample */
					play_cvsd(device, i);
				}

				/* if we're going inactive... */
				else if (m6844_channel[i].active && !(data & (1 << i)))
				{
					/* mark us inactive */
					m6844_channel[i].active = 0;

					/* stop playing the sample */
					stop_cvsd(device, i);
				}
			}
			break;

		/* interrupt control */
		case 0x15:
			state->m6844_interrupt = (state->m6844_interrupt & 0x80) | (data & 0x7f);
			break;

		/* chaining control */
		case 0x16:
			state->m6844_chain = data;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}
}



/*************************************
 *
 *  Sound cache management
 *
 *************************************/

static void reset_sound_cache(device_t *device)
{
	exidy440_audio_state *state = get_safe_token(device);
	state->sound_cache_end = state->sound_cache;
}


static INT16 *add_to_sound_cache(device_t *device, UINT8 *input, int address, int length, int bits, int frequency)
{
	exidy440_audio_state *state = get_safe_token(device);
	sound_cache_entry *current = state->sound_cache_end;

	/* compute where the end will be once we add this entry */
	state->sound_cache_end = (sound_cache_entry *)((UINT8 *)current + sizeof(sound_cache_entry) + length * 16);

	/* if this will overflow the cache, reset and re-add */
	if (state->sound_cache_end > state->sound_cache_max)
	{
		reset_sound_cache(device);
		return add_to_sound_cache(device, input, address, length, bits, frequency);
	}

	/* fill in this entry */
	current->next = state->sound_cache_end;
	current->address = address;
	current->length = length;
	current->bits = bits;
	current->frequency = frequency;

	/* decode the data into the cache */
	decode_and_filter_cvsd(device, input, length, bits, frequency, current->data);
	return current->data;
}


static INT16 *find_or_add_to_sound_cache(device_t *device, int address, int length, int bits, int frequency)
{
	exidy440_audio_state *state = get_safe_token(device);
	sound_cache_entry *current;

	for (current = state->sound_cache; current < state->sound_cache_end; current = current->next)
		if (current->address == address && current->length == length && current->bits == bits && current->frequency == frequency)
			return current->data;

	return add_to_sound_cache(device, &device->machine().root_device().memregion("cvsd")->base()[address], address, length, bits, frequency);
}



/*************************************
 *
 *  Internal CVSD decoder and player
 *
 *************************************/

static void play_cvsd(device_t *device, int ch)
{
	exidy440_audio_state *state = get_safe_token(device);
	sound_channel_data *channel = &state->sound_channel[ch];
	int address = state->m6844_channel[ch].address;
	int length = state->m6844_channel[ch].counter;
	INT16 *base;

	/* add the bank number to the address */
	if (state->sound_banks[ch] & 1)
		address += 0x00000;
	else if (state->sound_banks[ch] & 2)
		address += 0x08000;
	else if (state->sound_banks[ch] & 4)
		address += 0x10000;
	else if (state->sound_banks[ch] & 8)
		address += 0x18000;

	/* compute the base address in the converted samples array */
	base = find_or_add_to_sound_cache(device, address, length, channel_bits[ch], state->channel_frequency[ch]);
	if (!base)
		return;

	/* if the length is 0 or 1, just do an immediate end */
	if (length <= 3)
	{
		channel->base = base;
		channel->offset = length;
		channel->remaining = 0;
		m6844_finished(&state->m6844_channel[ch]);
		return;
	}

	if (SOUND_LOG && state->debuglog)
		fprintf(state->debuglog, "Sound channel %d play at %02X,%04X, length = %04X, volume = %02X/%02X\n",
				ch, state->sound_banks[ch],
				state->m6844_channel[ch].address, state->m6844_channel[ch].counter,
				state->sound_volume[ch * 2], state->sound_volume[ch * 2 + 1]);

	/* set the pointer and count */
	channel->base = base;
	channel->offset = 0;
	channel->remaining = length * 8;

	/* channels 2 and 3 play twice as slow, so we need to count twice as many samples */
	if (ch & 2) channel->remaining *= 2;
}


static void stop_cvsd(device_t *device, int ch)
{
	exidy440_audio_state *state = get_safe_token(device);
	/* the DMA channel is marked inactive; that will kill the audio */
	state->sound_channel[ch].remaining = 0;
	state->stream->update();

	if (SOUND_LOG && state->debuglog)
		fprintf(state->debuglog, "Channel %d stop\n", ch);
}



/*************************************
 *
 *  FIR digital filter
 *
 *************************************/

static void fir_filter(device_t *device, INT32 *input, INT16 *output, int count)
{
	while (count--)
	{
		INT32 result = (input[-1] - input[-8] - input[-48] + input[-55]) << 2;
		result += (input[0] + input[-18] + input[-38] + input[-56]) << 3;
		result += (-input[-2] - input[-4] + input[-5] + input[-51] - input[-52] - input[-54]) << 4;
		result += (-input[-3] - input[-11] - input[-45] - input[-53]) << 5;
		result += (input[-6] + input[-7] - input[-9] - input[-15] - input[-41] - input[-47] + input[-49] + input[-50]) << 6;
		result += (-input[-10] + input[-12] + input[-13] + input[-14] + input[-21] + input[-35] + input[-42] + input[-43] + input[-44] - input[-46]) << 7;
		result += (-input[-16] - input[-17] + input[-19] + input[-37] - input[-39] - input[-40]) << 8;
		result += (input[-20] - input[-22] - input[-24] + input[-25] + input[-31] - input[-32] - input[-34] + input[-36]) << 9;
		result += (-input[-23] - input[-33]) << 10;
		result += (input[-26] + input[-30]) << 11;
		result += (input[-27] + input[-28] + input[-29]) << 12;
		result >>= 14;

		if (result < -32768)
			result = -32768;
		else if (result > 32767)
			result = 32767;

		*output++ = result;
		input++;
	}
}



/*************************************
 *
 *  CVSD decoder
 *
 *************************************/

static void decode_and_filter_cvsd(device_t *device, UINT8 *input, int bytes, int maskbits, int frequency, INT16 *output)
{
	INT32 buffer[SAMPLE_BUFFER_LENGTH + FIR_HISTORY_LENGTH];
	int total_samples = bytes * 8;
	int mask = (1 << maskbits) - 1;
	double filter, integrator, leak;
	double charge, decay, gain;
	int steps;
	int chunk_start;

	/* compute the charge, decay, and leak constants */
	charge = pow(exp(-1.0), 1.0 / (FILTER_CHARGE_TC * (double)frequency));
	decay = pow(exp(-1.0), 1.0 / (FILTER_DECAY_TC * (double)frequency));
	leak = pow(exp(-1.0), 1.0 / (INTEGRATOR_LEAK_TC * (double)frequency));

	/* compute the gain */
	gain = SAMPLE_GAIN;

	/* clear the history words for a start */
	memset(&buffer[0], 0, FIR_HISTORY_LENGTH * sizeof(INT32));

	/* initialize the CVSD decoder */
	steps = 0xaa;
	filter = FILTER_MIN;
	integrator = 0.0;

	/* loop over chunks */
	for (chunk_start = 0; chunk_start < total_samples; chunk_start += SAMPLE_BUFFER_LENGTH)
	{
		INT32 *bufptr = &buffer[FIR_HISTORY_LENGTH];
		int chunk_bytes;
		int ind;

		/* how many samples do we generate in this chunk? */
		if (chunk_start + SAMPLE_BUFFER_LENGTH > total_samples)
			chunk_bytes = (total_samples - chunk_start) / 8;
		else
			chunk_bytes = SAMPLE_BUFFER_LENGTH / 8;

		/* loop over samples */
		for (ind = 0; ind < chunk_bytes; ind++)
		{
			double temp;
			int databyte = *input++;
			int bit;
			int sample;

			/* loop over bits in the byte, low to high */
			for (bit = 0; bit < 8; bit++)
			{
				/* move the estimator up or down a step based on the bit */
				if (databyte & (1 << bit))
				{
					integrator += filter;
					steps = (steps << 1) | 1;
				}
				else
				{
					integrator -= filter;
					steps <<= 1;
				}

				/* keep track of the last n bits */
				steps &= mask;

				/* simulate leakage */
				integrator *= leak;

				/* if we got all 0's or all 1's in the last n bits, bump the step up */
				if (steps == 0 || steps == mask)
				{
					filter = FILTER_MAX - ((FILTER_MAX - filter) * charge);
					if (filter > FILTER_MAX)
						filter = FILTER_MAX;
				}

				/* simulate decay */
				else
				{
					filter *= decay;
					if (filter < FILTER_MIN)
						filter = FILTER_MIN;
				}

				/* compute the sample as a 32-bit word */
				temp = integrator * gain;

				/* compress the sample range to fit better in a 16-bit word */
				if (temp < 0)
					sample = (int)(temp / (-temp * (1.0 / 32768.0) + 1.0));
				else
					sample = (int)(temp / (temp * (1.0 / 32768.0) + 1.0));

				/* store the result to our temporary buffer */
				*bufptr++ = sample;
			}
		}

		/* all done with this chunk, run the filter on it */
		fir_filter(device, &buffer[FIR_HISTORY_LENGTH], &output[chunk_start], chunk_bytes * 8);

		/* copy the last few input samples down to the start for a new history */
		memcpy(&buffer[0], &buffer[SAMPLE_BUFFER_LENGTH], FIR_HISTORY_LENGTH * sizeof(INT32));
	}

	/* make sure the volume goes smoothly to 0 over the last 512 samples */
	if (FADE_TO_ZERO)
	{
		INT16 *data;

		chunk_start = (total_samples > 512) ? total_samples - 512 : 0;
		data = output + chunk_start;
		for ( ; chunk_start < total_samples; chunk_start++)
		{
			*data = (*data * ((total_samples - chunk_start) >> 9));
			data++;
		}
	}
}


static WRITE8_DEVICE_HANDLER( sound_banks_w )
{
	exidy440_audio_state *state = get_safe_token(device);
	state->sound_banks[offset] = data;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( exidy440_audio_map, AS_PROGRAM, 8, driver_device )
	AM_RANGE(0x0000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x801f) AM_MIRROR(0x03e0) AM_DEVREADWRITE_LEGACY("custom", m6844_r, m6844_w)
	AM_RANGE(0x8400, 0x840f) AM_MIRROR(0x03f0) AM_DEVREADWRITE_LEGACY("custom", sound_volume_r, sound_volume_w)
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_DEVREAD_LEGACY("custom", sound_command_r) AM_WRITENOP
	AM_RANGE(0x8c00, 0x93ff) AM_NOP
	AM_RANGE(0x9400, 0x9403) AM_MIRROR(0x03fc) AM_READNOP AM_DEVWRITE_LEGACY("custom", sound_banks_w)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READNOP AM_DEVWRITE_LEGACY("custom", sound_interrupt_clear_w)
	AM_RANGE(0x9c00, 0x9fff) AM_NOP
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xdfff) AM_NOP
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Audio definitions
 *
 *************************************/

const device_type EXIDY440 = &device_creator<exidy440_sound_device>;

exidy440_sound_device::exidy440_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EXIDY440, "Exidy 440 CVSD", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(exidy440_audio_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void exidy440_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy440_sound_device::device_start()
{
	DEVICE_START_NAME( exidy440_sound )(this);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void exidy440_sound_device::device_stop()
{
	DEVICE_STOP_NAME( exidy440_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy440_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}





/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( exidy440_audio )

	MCFG_CPU_ADD("audiocpu", M6809, EXIDY440_AUDIO_CLOCK)
	MCFG_CPU_PROGRAM_MAP(exidy440_audio_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_assert)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("custom", EXIDY440, EXIDY440_AUDIO_CLOCK/16)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

//  MCFG_SOUND_ADD("cvsd1", MC3418, EXIDY440_MC3418_CLOCK)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

//  MCFG_SOUND_ADD("cvsd2", MC3418, EXIDY440_MC3418_CLOCK)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

//  MCFG_SOUND_ADD("cvsd3", MC3417, EXIDY440_MC3417_CLOCK)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

//  MCFG_SOUND_ADD("cvsd4", MC3417, EXIDY440_MC3417_CLOCK)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END
