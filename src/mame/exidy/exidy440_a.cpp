// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Exidy 440 sound system

    Special thanks to Zonn Moore and Neil Bradley for letting me hack
    their Retrocade CVSD decoder into the sound system here.

***************************************************************************/

#include "emu.h"
#include "exidy440_a.h"

#include "cpu/m6809/m6809.h"


#define SOUND_LOG       0
#define FADE_TO_ZERO    1


/* internal caching */
#define SAMPLE_BUFFER_LENGTH    1024                /* size of temporary decode buffer on the stack */

/* FIR digital filter parameters */
#define FIR_HISTORY_LENGTH      57                  /* number of FIR coefficients */

/* CVSD decoding parameters */
#define INTEGRATOR_LEAK_TC      (10e3 * 0.1e-6)
#define FILTER_DECAY_TC         ((18e3 + 3.3e3) * 0.33e-6)
#define FILTER_CHARGE_TC        (18e3 * 0.33e-6)
#define FILTER_MIN              0.0416
#define FILTER_MAX              1.0954
#define SAMPLE_GAIN             10000.0

/* constant channel parameters */
static const int channel_bits[4] =
{
	4, 4,                                   /* channels 0 and 1 are MC3418s, 4-bit CVSD */
	3, 3                                    /* channels 2 and 3 are MC3417s, 3-bit CVSD */
};


/*************************************
 *
 *  Audio CPU memory map
 *
 *************************************/

void exidy440_sound_device::exidy440_audio_map(address_map &map)
{
	map(0x0000, 0x7fff).noprw();
	map(0x8000, 0x801f).mirror(0x03e0).rw(FUNC(exidy440_sound_device::m6844_r), FUNC(exidy440_sound_device::m6844_w));
	map(0x8400, 0x840f).mirror(0x03f0).rw(FUNC(exidy440_sound_device::sound_volume_r), FUNC(exidy440_sound_device::sound_volume_w));
	map(0x8800, 0x8800).mirror(0x03ff).r(FUNC(exidy440_sound_device::sound_command_r)).nopw();
	map(0x8c00, 0x93ff).noprw();
	map(0x9400, 0x9403).mirror(0x03fc).nopr().w(FUNC(exidy440_sound_device::sound_banks_w));
	map(0x9800, 0x9800).mirror(0x03ff).nopr().w(FUNC(exidy440_sound_device::sound_interrupt_clear_w));
	map(0x9c00, 0x9fff).noprw();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).noprw();
	map(0xe000, 0xffff).rom().region("audiocpu", 0);
}


DEFINE_DEVICE_TYPE(EXIDY440, exidy440_sound_device, "exidy440_sound", "Exidy 440 CVSD")

exidy440_sound_device::exidy440_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EXIDY440, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_audiocpu(*this, "audiocpu"),
	m_samples(*this, "samples")
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void exidy440_sound_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_audiocpu, EXIDY440_AUDIO_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &exidy440_sound_device::exidy440_audio_map);

//  MC3418(config, "cvsd1", EXIDY440_MC3418_CLOCK).add_route(ALL_OUTPUTS, "speaker", 1.0);
//  MC3418(config, "cvsd2", EXIDY440_MC3418_CLOCK).add_route(ALL_OUTPUTS, "speaker", 1.0);
//  MC3417(config, "cvsd3", EXIDY440_MC3417_CLOCK).add_route(ALL_OUTPUTS, "speaker", 1.0);
//  MC3417(config, "cvsd4", EXIDY440_MC3417_CLOCK).add_route(ALL_OUTPUTS, "speaker", 1.0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy440_sound_device::device_start()
{
	/* reset the system */
	m_sound_command = 0;
	m_sound_command_ack = 1;
	save_item(NAME(m_sound_command));
	save_item(NAME(m_sound_command_ack));

	for (int i = 0; i < 4; i++)
	{
		m_sound_banks[i] = 0;
		m_sound_channel[i].base = nullptr;
		m_sound_channel[i].offset = 0;
		m_sound_channel[i].remaining = 0;
	}

	memset(m_sound_volume, 0, sizeof(m_sound_volume));
	save_item(NAME(m_sound_volume));
	save_item(NAME(m_sound_banks));

	/* reset the 6844 */
	for (int i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].address = 0;
		m_m6844_channel[i].counter = 0;
		m_m6844_channel[i].control = 0;
		m_m6844_channel[i].start_address = 0;
		m_m6844_channel[i].start_counter = 0;
	}

	m_m6844_priority = 0;
	m_m6844_interrupt = 0;
	m_m6844_chain = 0;
	save_item(NAME(m_m6844_priority));
	save_item(NAME(m_m6844_interrupt));
	save_item(NAME(m_m6844_chain));

	m_channel_frequency[0] = clock();   /* channels 0 and 1 are run by FCLK */
	m_channel_frequency[1] = clock();
	m_channel_frequency[2] = clock()/2; /* channels 2 and 3 are run by SCLK */
	m_channel_frequency[3] = clock()/2;

	/* get stream channels */
	m_stream = stream_alloc(0, 2, clock());

	/* allocate the mixer buffer */
	m_mixer_buffer_left.resize(clock());
	m_mixer_buffer_right.resize(clock());

	if (SOUND_LOG)
		m_debuglog = fopen("sound.log", "w");
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void exidy440_sound_device::device_stop()
{
	if (SOUND_LOG && m_debuglog)
		fclose(m_debuglog);
}

/*************************************
 *
 *  Add a bunch of samples to the mix
 *
 *************************************/

void exidy440_sound_device::add_and_scale_samples(int ch, int32_t *dest, int samples, int volume)
{
	sound_channel_data *channel = &m_sound_channel[ch];
	int16_t *srcdata;

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
		for (int i = 0; i < samples; i += 2)
		{
			int16_t sample = *srcdata++ * volume / 256;
			*dest++ += sample;
			*dest++ += sample;
		}
	}

	/* channels 0 and 1 are full-rate samples */
	else
	{
		srcdata = &channel->base[channel->offset];
		for (int i = 0; i < samples; i++)
			*dest++ += *srcdata++ * volume / 256;
	}
}



/*************************************
 *
 *  Mix the result to 16 bits
 *
 *************************************/

void exidy440_sound_device::mix_to_16(sound_stream &stream)
{
	int32_t *mixer_left = &m_mixer_buffer_left[0];
	int32_t *mixer_right = &m_mixer_buffer_right[0];

	for (int i = 0; i < stream.samples(); i++)
	{
		stream.put_int_clamp(0, i, *mixer_left++, 32768);
		stream.put_int_clamp(1, i, *mixer_right++, 32768);
	}
}

/*************************************
 *
 *  Sound command register
 *
 *************************************/

uint8_t exidy440_sound_device::sound_command_r()
{
	/* clear the FIRQ that got us here and acknowledge the read to the main CPU */
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	m_sound_command_ack = 1;

	return m_sound_command;
}


void exidy440_sound_device::exidy440_sound_command(uint8_t param)
{
	m_sound_command = param;
	m_sound_command_ack = 0;
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


uint8_t exidy440_sound_device::exidy440_sound_command_ack()
{
	return m_sound_command_ack;
}



/*************************************
 *
 *  Sound volume registers
 *
 *************************************/

uint8_t exidy440_sound_device::sound_volume_r(offs_t offset)
{
	return m_sound_volume[offset];
}

void exidy440_sound_device::sound_volume_w(offs_t offset, uint8_t data)
{
	if (SOUND_LOG && m_debuglog)
		fprintf(m_debuglog, "Volume %02X=%02X\n", offset, data);

	/* update the stream */
	m_stream->update();

	/* set the new volume */
	m_sound_volume[offset] = ~data;
}



/*************************************
 *
 *  Sound interrupt handling
 *
 *************************************/

void exidy440_sound_device::sound_interrupt_w(int state)
{
	if (state)
		m_audiocpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

void exidy440_sound_device::sound_interrupt_clear_w(uint8_t data)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void exidy440_sound_device::sound_reset_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state);
}



/*************************************
 *
 *  MC6844 DMA controller interface
 *
 *************************************/

void exidy440_sound_device::m6844_update()
{
	/* update the stream */
	m_stream->update();
}


void exidy440_sound_device::m6844_finished(m6844_channel_data *channel)
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

uint8_t exidy440_sound_device::m6844_r(offs_t offset)
{
	m6844_channel_data *m6844_channel = m_m6844_channel;
	int result = 0;

	/* first update the current state of the DMA transfers */
	m6844_update();

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
			result = m_m6844_priority;
			break;

		/* interrupt control */
		case 0x15:

			/* update the global DMA end flag */
			m_m6844_interrupt &= ~0x80;
			m_m6844_interrupt |= (m6844_channel[0].control & 0x80) |
								(m6844_channel[1].control & 0x80) |
								(m6844_channel[2].control & 0x80) |
								(m6844_channel[3].control & 0x80);

			result = m_m6844_interrupt;
			break;

		/* chaining control */
		case 0x16:
			result = m_m6844_chain;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}

	return result;
}


void exidy440_sound_device::m6844_w(offs_t offset, uint8_t data)
{
	m6844_channel_data *m6844_channel = m_m6844_channel;

	/* first update the current state of the DMA transfers */
	m6844_update();

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
			m_m6844_priority = data;

			/* update the sound playback on each channel */
			for (int i = 0; i < 4; i++)
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
					play_cvsd(i);
				}

				/* if we're going inactive... */
				else if (m6844_channel[i].active && !(data & (1 << i)))
				{
					/* mark us inactive */
					m6844_channel[i].active = 0;

					/* stop playing the sample */
					stop_cvsd(i);
				}
			}
			break;

		/* interrupt control */
		case 0x15:
			m_m6844_interrupt = (m_m6844_interrupt & 0x80) | (data & 0x7f);
			break;

		/* chaining control */
		case 0x16:
			m_m6844_chain = data;
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

int16_t *exidy440_sound_device::add_to_sound_cache(uint8_t *input, int address, int length, int bits, int frequency)
{
	/* fill in this entry */
	auto &current = m_sound_cache.emplace_back();
	current.address = address;
	current.length = length;
	current.bits = bits;
	current.frequency = frequency;

	/* decode the data into the cache */
	decode_and_filter_cvsd(input, length, bits, frequency, current.data);
	return &current.data[0];
}


int16_t *exidy440_sound_device::find_or_add_to_sound_cache(int address, int length, int bits, int frequency)
{
	for (auto &current : m_sound_cache)
		if (current.address == address && current.length == length && current.bits == bits && current.frequency == frequency)
			return &current.data[0];

	return length ? add_to_sound_cache(&m_samples[address], address, length, bits, frequency) : nullptr;
}



/*************************************
 *
 *  Internal CVSD decoder and player
 *
 *************************************/

void exidy440_sound_device::play_cvsd(int ch)
{
	sound_channel_data *channel = &m_sound_channel[ch];
	int address = m_m6844_channel[ch].address;
	int length = m_m6844_channel[ch].counter;
	int16_t *base;

	/* add the bank number to the address */
	if (m_sound_banks[ch] & 1)
		address += 0x00000;
	else if (m_sound_banks[ch] & 2)
		address += 0x08000;
	else if (m_sound_banks[ch] & 4)
		address += 0x10000;
	else if (m_sound_banks[ch] & 8)
		address += 0x18000;

	/* compute the base address in the converted samples array */
	base = find_or_add_to_sound_cache(address, length, channel_bits[ch], m_channel_frequency[ch]);

	/* if the length is 0 or 1, just do an immediate end */
	if (length <= 3)
	{
		channel->base = base;
		channel->offset = length;
		channel->remaining = 0;
		m6844_finished(&m_m6844_channel[ch]);
		return;
	}

	if (SOUND_LOG && m_debuglog)
		fprintf(m_debuglog, "Sound channel %d play at %02X,%04X, length = %04X, volume = %02X/%02X\n",
				ch, m_sound_banks[ch],
				m_m6844_channel[ch].address, m_m6844_channel[ch].counter,
				m_sound_volume[ch * 2], m_sound_volume[ch * 2 + 1]);

	/* set the pointer and count */
	channel->base = base;
	channel->offset = 0;
	channel->remaining = length * 8;

	/* channels 2 and 3 play twice as slow, so we need to count twice as many samples */
	if (ch & 2) channel->remaining *= 2;
}


void exidy440_sound_device::stop_cvsd(int ch)
{
	/* the DMA channel is marked inactive; that will kill the audio */
	m_sound_channel[ch].remaining = 0;
	m_stream->update();

	if (SOUND_LOG && m_debuglog)
		fprintf(m_debuglog, "Channel %d stop\n", ch);
}



/*************************************
 *
 *  FIR digital filter
 *
 *************************************/

void exidy440_sound_device::fir_filter(int32_t *input, int16_t *output, int count)
{
	while (count--)
	{
		int32_t result = (input[-1] - input[-8] - input[-48] + input[-55]) << 2;
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

void exidy440_sound_device::decode_and_filter_cvsd(uint8_t *input, int bytes, int maskbits, int frequency, std::vector<int16_t> &output)
{
	int32_t buffer[SAMPLE_BUFFER_LENGTH + FIR_HISTORY_LENGTH];
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
	memset(&buffer[0], 0, FIR_HISTORY_LENGTH * sizeof(int32_t));

	/* initialize the CVSD decoder */
	steps = 0xaa;
	filter = FILTER_MIN;
	integrator = 0.0;

	/* loop over chunks */
	output.resize(total_samples);
	for (chunk_start = 0; chunk_start < total_samples; chunk_start += SAMPLE_BUFFER_LENGTH)
	{
		int32_t *bufptr = &buffer[FIR_HISTORY_LENGTH];
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
		fir_filter(&buffer[FIR_HISTORY_LENGTH], &output[chunk_start], chunk_bytes * 8);

		/* copy the last few input samples down to the start for a new history */
		memcpy(&buffer[0], &buffer[SAMPLE_BUFFER_LENGTH], FIR_HISTORY_LENGTH * sizeof(int32_t));
	}

	/* make sure the volume goes smoothly to 0 over the last 512 samples */
	if (FADE_TO_ZERO)
	{
		int16_t *data;

		chunk_start = (total_samples > 512) ? total_samples - 512 : 0;
		data = &output[chunk_start];
		for ( ; chunk_start < total_samples; chunk_start++)
		{
			*data = (*data * ((total_samples - chunk_start) >> 9));
			data++;
		}
	}
}


void exidy440_sound_device::sound_banks_w(offs_t offset, uint8_t data)
{
	m_sound_banks[offset] = data;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy440_sound_device::sound_stream_update(sound_stream &stream)
{
	/* reset the mixer buffers */
	std::fill_n(&m_mixer_buffer_left[0], stream.samples(), 0);
	std::fill_n(&m_mixer_buffer_right[0], stream.samples(), 0);

	/* loop over channels */
	for (int ch = 0; ch < 4; ch++)
	{
		sound_channel_data *channel = &m_sound_channel[ch];
		int length, volume, left = stream.samples();
		int effective_offset;

		/* if we're not active, bail */
		if (channel->remaining <= 0)
			continue;

		/* see how many samples to copy */
		length = (left > channel->remaining) ? channel->remaining : left;

		/* get a pointer to the sample data and copy to the left */
		volume = m_sound_volume[2 * ch + 0];
		if (volume)
			add_and_scale_samples(ch, &m_mixer_buffer_left[0], length, volume);

		/* get a pointer to the sample data and copy to the left */
		volume = m_sound_volume[2 * ch + 1];
		if (volume)
			add_and_scale_samples(ch, &m_mixer_buffer_right[0], length, volume);

		/* update our counters */
		channel->offset += length;
		channel->remaining -= length;
		left -= length;

		/* update the MC6844 */
		effective_offset = (ch & 2) ? channel->offset / 2 : channel->offset;
		m_m6844_channel[ch].address = m_m6844_channel[ch].start_address + effective_offset / 8;
		m_m6844_channel[ch].counter = m_m6844_channel[ch].start_counter - effective_offset / 8;
		if (m_m6844_channel[ch].counter <= 0)
		{
			if (SOUND_LOG && m_debuglog)
				fprintf(m_debuglog, "Channel %d finished\n", ch);
			m6844_finished(&m_m6844_channel[ch]);
		}
	}

	/* all done, time to mix it */
	mix_to_16(stream);
}
