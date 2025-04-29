// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/***************************************************************************

    NAMCO sound driver.

    This driver handles the four known types of NAMCO wavetable sounds:
    - 3-voice mono (PROM-based design: Pac-Man, Pengo, Dig Dug, etc)
    - 8-voice quadrophonic (Pole Position 1, Pole Position 2)
    - 8-voice mono (custom 15XX: Mappy, Dig Dug 2, etc)
    - 8-voice stereo (System 1)

    The 15XX custom does not have a DAC of its own; instead, it streams
    the 4-bit PROM data directly into the 99XX custom DAC. Most pre-99XX
    (and pre-15XX) Namco games use a LS273 latch (cleared when sound is
    disabled), a 4.7K/2.2K/1K/470 resistor-weighted DAC, and a 4066 and
    second group of resistors (10K/22K/47K/100K) for volume control.
    Pole Position does more complicated sound mixing: a 4051 multiplexes
    wavetable sound with four signals derived from the 52XX and 54XX, the
    selected signal is distributed to four volume control sections, and
    finally the engine noise is mixed into all four channels. The later
    CUS30 also uses the 99XX DAC, or two 99XX in the optional 16-channel
    stereo configuration, but it uses no PROM and delivers its own samples.

    The CUS30 has been decapped and verified to be a ULA.

***************************************************************************/

#include "emu.h"
#include "namco.h"


// quality parameter: internal sample rate is 192 KHz, output is 48 KHz
#define INTERNAL_RATE   192000

// 16 bits: sample bits of the stream buffer
// 4 bits:  volume
// 4 bits:  prom sample bits
#define MIXLEVEL    (1 << (16 - 4 - 4))

// stream output level
#define OUTPUT_LEVEL(n)     ((n) * MIXLEVEL / m_voices)

// a position of waveform sample
#define WAVEFORM_POSITION(n)    (((n) >> m_f_fracbits) & 0x1f)

DEFINE_DEVICE_TYPE(NAMCO,       namco_device,       "namco",       "Namco")
DEFINE_DEVICE_TYPE(NAMCO_15XX,  namco_15xx_device,  "namco_15xx",  "Namco 15xx")
DEFINE_DEVICE_TYPE(NAMCO_CUS30, namco_cus30_device, "namco_cus30", "Namco CUS30")

namco_audio_device::namco_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_wave_ptr(*this, DEVICE_SELF)
	, m_last_channel(nullptr)
	, m_wavedata(nullptr)
	, m_wave_size(0)
	, m_sound_enable(false)
	, m_stream(nullptr)
	, m_namco_clock(0)
	, m_sample_rate(0)
	, m_f_fracbits(0)
	, m_voices(0)
	, m_stereo(false)
{
}

namco_device::namco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device(mconfig, NAMCO, tag, owner, clock)
	, m_soundregs(nullptr)
{
}

namco_15xx_device::namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device(mconfig, NAMCO_15XX, tag, owner, clock)
	, m_soundregs(nullptr)
{
}

namco_cus30_device::namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device(mconfig, NAMCO_CUS30, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_audio_device::device_start()
{
	// extract globals from the interface
	m_last_channel = m_channel_list + m_voices;

	// build the waveform table
	build_decoded_waveform(m_wave_ptr);

	// get stream channels
	if (m_stereo)
		m_stream = stream_alloc(0, 2, 192000);
	else
		m_stream = stream_alloc(0, 1, 192000);

	// start with sound enabled, many games don't have a sound enable register
	m_sound_enable = true;

	// register with the save state system
	if (m_wave_ptr == nullptr)
		save_pointer(NAME(m_wavedata), 0x400);

	save_item(NAME(m_sound_enable));
	for (int v = 0; v < MAX_VOLUME; v++)
		save_pointer(NAME(m_waveform[v]), 32 * 8 * (1+m_wave_size), v);

	// reset all the voices
	for (sound_channel *voice = m_channel_list; voice < m_last_channel; voice++)
	{
		voice->frequency = 0;
		voice->volume[0] = voice->volume[1] = 0;
		voice->waveform_select = 0;
		voice->counter = 0;
		voice->noise_sw = 0;
		voice->noise_state = 0;
		voice->noise_seed = 1;
		voice->noise_counter = 0;
		voice->noise_hold = 0;
	}

	// register with the save state system
	save_pointer(STRUCT_MEMBER(m_channel_list, frequency), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, counter), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, volume), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, noise_sw), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, noise_state), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, noise_seed), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, noise_hold), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, noise_counter), m_voices);
	save_pointer(STRUCT_MEMBER(m_channel_list, waveform_select), m_voices);
}


void namco_device::device_start()
{
	namco_audio_device::device_start();

	m_soundregs = make_unique_clear<uint8_t[]>(0x400);
	save_pointer(NAME(m_soundregs), 0x400);
}


void namco_15xx_device::device_start()
{
	namco_audio_device::device_start();

	m_soundregs = make_unique_clear<uint8_t[]>(0x400);
	save_pointer(NAME(m_soundregs), 0x400);
}


void namco_audio_device::device_clock_changed()
{
	int clock_multiple;

	// adjust internal clock
	m_namco_clock = clock();
	for (clock_multiple = 0; m_namco_clock < INTERNAL_RATE; clock_multiple++)
		m_namco_clock *= 2;

	m_f_fracbits = clock_multiple + 15;

	// adjust output clock
	m_sample_rate = m_namco_clock;

	logerror("Namco: freq fractional bits = %d: internal freq = %d, output freq = %d\n", m_f_fracbits, m_namco_clock, m_sample_rate);

	m_stream->set_sample_rate(m_sample_rate);
}


// update the decoded waveform data
void namco_audio_device::update_namco_waveform(int offset, uint8_t data)
{
	if (m_wave_size == 1)
	{
		// use full byte, first 4 high bits, then low 4 bits
		for (int v = 0; v < MAX_VOLUME; v++)
		{
			int16_t wdata = ((data >> 4) & 0x0f) - 8;
			m_waveform[v][offset * 2] = OUTPUT_LEVEL(wdata * v);
			wdata = (data & 0x0f) - 8;
			m_waveform[v][offset * 2 + 1] = OUTPUT_LEVEL(wdata * v);
		}
	}
	else
	{
		// use only low 4 bits
		for (int v = 0; v < MAX_VOLUME; v++)
			m_waveform[v][offset] = OUTPUT_LEVEL(((data & 0x0f) - 8) * v);
	}
}


// build the decoded waveform table
void namco_audio_device::build_decoded_waveform(uint8_t *rgnbase)
{
	if (rgnbase != nullptr)
		m_wavedata = rgnbase;
	else
	{
		m_waveram_alloc = make_unique_clear<uint8_t[]>(0x400);
		m_wavedata = m_waveram_alloc.get();
	}

	// 20pacgal has waves in RAM but old sound system
	int size;
	if (rgnbase == nullptr && m_voices != 3)
	{
		m_wave_size = 1;
		size = 32 * 16; // 32 samples, 16 waveforms
	}
	else
	{
		m_wave_size = 0;
		size = 32 * 8; // 32 samples, 8 waveforms
	}

	for (int v = 0; v < MAX_VOLUME; v++)
		m_waveform[v] = std::make_unique<int16_t[]>(size);

	for (int offset = 0; offset < 256; offset++)
		update_namco_waveform(offset, m_wavedata[offset]);
}


// generate sound by oversampling
uint32_t namco_audio_device::namco_update_one(sound_stream &stream, int output, const int16_t *wave, uint32_t counter, uint32_t freq)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		stream.add_int(output, sampindex, wave[WAVEFORM_POSITION(counter)], 32768);
		counter += freq;
	}

	return counter;
}


void namco_audio_device::sound_enable_w(int state)
{
	m_sound_enable = state;
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

void namco_device::pacman_sound_w(offs_t offset, uint8_t data)
{
	sound_channel *voice;
	int ch;

	data &= 0x0f;
	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	if (offset < 0x10)
		ch = (offset - 5) / 5;
	else if (offset == 0x10)
		ch = 0;
	else
		ch = (offset - 0x11) / 5;

	if (ch >= m_voices)
		return;

	// recompute the voice parameters
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
		// the frequency has 20 bits
		// the first voice has extra frequency bits
		voice->frequency = (ch == 0) ? m_soundregs[0x10] : 0;
		voice->frequency += (m_soundregs[ch * 5 + 0x11] << 4);
		voice->frequency += (m_soundregs[ch * 5 + 0x12] << 8);
		voice->frequency += (m_soundregs[ch * 5 + 0x13] << 12);
		voice->frequency += (m_soundregs[ch * 5 + 0x14] << 16); // always 0
		break;

	case 0x15:
		voice->volume[0] = data;
		break;
	}
}

void namco_cus30_device::pacman_sound_w(offs_t offset, uint8_t data)
{
	sound_channel *voice;
	int ch;

	uint8_t *soundregs = &m_wavedata[0x100];

	data &= 0x0f;
	if (soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	soundregs[offset] = data;

	if (offset < 0x10)
		ch = (offset - 5) / 5;
	else if (offset == 0x10)
		ch = 0;
	else
		ch = (offset - 0x11) / 5;

	if (ch >= m_voices)
		return;

	// recompute the voice parameters
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
		// the frequency has 20 bits
		// the first voice has extra frequency bits
		voice->frequency = (ch == 0) ? soundregs[0x10] : 0;
		voice->frequency += (soundregs[ch * 5 + 0x11] << 4);
		voice->frequency += (soundregs[ch * 5 + 0x12] << 8);
		voice->frequency += (soundregs[ch * 5 + 0x13] << 12);
		voice->frequency += (soundregs[ch * 5 + 0x14] << 16); // always 0
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

uint8_t namco_device::polepos_sound_r(offs_t offset)
{
	return m_soundregs[offset];
}

void namco_device::polepos_sound_w(offs_t offset, uint8_t data)
{
	sound_channel *voice;
	int ch;

	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	ch = (offset & 0x1f) / 4;

	// recompute the voice parameters
	voice = m_channel_list + ch;
	switch (offset & 0x23)
	{
	case 0x00:
	case 0x01:
		// the frequency has 16 bits
		voice->frequency = m_soundregs[ch * 4 + 0x00];
		voice->frequency += m_soundregs[ch * 4 + 0x01] << 8;
		break;

	case 0x23:
		voice->waveform_select = data & 7;
		[[fallthrough]];
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

		// if 54XX or 52XX selected, silence this voice
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

Grobda also stuffs values into register offset 0x02 with a frequency of zero
to make 15XX channels act like a 4-bit DAC instead of waveform voices. This
has been emulated by allowing writes to set the upper counter bits directly.
Possibly offsets 0x00 and 0x01 can be used to set the fractional bits.
*/

void namco_15xx_device::namco_15xx_w(offs_t offset, uint8_t data)
{
	sound_channel *voice;
	int ch;

	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	ch = offset / 8;
	if (ch >= m_voices)
		return;

	// recompute the voice parameters
	voice = m_channel_list + ch;
	switch (offset & 7)
	{
	case 0x02:
		voice->counter &= util::make_bitmask<uint32_t>(m_f_fracbits);
		voice->counter |= uint32_t(data & 0x1f) << m_f_fracbits;
		break;

	case 0x03:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x06:
		voice->waveform_select = (data >> 4) & 7;
		[[fallthrough]];
	case 0x04:
	case 0x05:
		// the frequency has 20 bits
		voice->frequency = m_soundregs[ch * 8 + 0x04];
		voice->frequency += m_soundregs[ch * 8 + 0x05] << 8;
		voice->frequency += (m_soundregs[ch * 8 + 0x06] & 15) << 16; // high bits are from here
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

void namco_cus30_device::namcos1_sound_w(offs_t offset, uint8_t data)
{
	sound_channel *voice;

	// verify the offset
	if (offset > 63)
	{
		logerror("NAMCOS1 sound: Attempting to write past the 64 registers segment\n");
		return;
	}

	uint8_t *soundregs = &m_wavedata[0x100];

	if (soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	soundregs[offset] = data;

	int ch = offset / 8;
	if (ch >= m_voices)
		return;

	// recompute the voice parameters
	voice = m_channel_list + ch;
	switch (offset & 7)
	{
	case 0x00:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x01:
		voice->waveform_select = (data >> 4) & 15;
		[[fallthrough]];
	case 0x02:
	case 0x03:
		// the frequency has 20 bits
		voice->frequency = (soundregs[ch * 8 + 0x01] & 15) << 16; // high bits are from here
		voice->frequency += soundregs[ch * 8 + 0x02] << 8;
		voice->frequency += soundregs[ch * 8 + 0x03];
		break;

	case 0x04:
		voice->volume[1] = data & 0x0f;

		int nssw = ((data & 0x80) >> 7);
		if (++voice == m_last_channel)
			voice = m_channel_list;
		voice->noise_sw = nssw;
		break;
	}
}

void namco_cus30_device::namcos1_cus30_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		if (m_wavedata[offset] != data)
		{
			// update the streams
			m_stream->update();

			m_wavedata[offset] = data;

			// update the decoded waveform table
			update_namco_waveform(offset, data);
		}
	}
	else if (offset < 0x140)
		namcos1_sound_w(offset - 0x100, data);
	else
		m_wavedata[offset] = data;
}

uint8_t namco_cus30_device::namcos1_cus30_r(offs_t offset)
{
	if (offset >= 0x100 && offset < 0x140)
	{
		int ch = (offset - 0x100) / 8;
		int reg = offset & 7;

		// reading from register 5 returns the counter (used by baraduke)
		if (ch < m_voices && reg == 5)
		{
			m_stream->update();
			return WAVEFORM_POSITION(m_channel_list[ch].counter);
		}
	}

	return m_wavedata[offset];
}

uint8_t namco_15xx_device::sharedram_r(offs_t offset)
{
	return m_soundregs[offset];
}

void namco_15xx_device::sharedram_w(offs_t offset, uint8_t data)
{
	if (offset < 0x40)
		namco_15xx_w(offset, data);
	else
	{
		m_soundregs[offset] = data;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void namco_audio_device::sound_stream_update(sound_stream &stream)
{
	if (m_stereo)
	{
		// if no sound, we're done
		if (!m_sound_enable)
			return;

		// loop over each voice and add its contribution
		for (sound_channel *voice = m_channel_list; voice < m_last_channel; voice++)
		{
			int lv = voice->volume[0];
			int rv = voice->volume[1];

			if (voice->noise_sw)
			{
				int f = voice->frequency & 0xff;

				// only update if we have non-zero volume
				if (lv || rv)
				{
					int hold_time = 1 << (m_f_fracbits - 16);
					int hold = voice->noise_hold;
					uint32_t delta = f << 4;
					uint32_t c = voice->noise_counter;
					int16_t l_noise_data = OUTPUT_LEVEL(0x07 * (lv >> 1));
					int16_t r_noise_data = OUTPUT_LEVEL(0x07 * (rv >> 1));

					// add our contribution
					for (int i = 0; i < stream.samples(); i++)
					{
						if (voice->noise_state)
						{
							stream.add_int(0, i, l_noise_data, 32768);
							stream.add_int(1, i, r_noise_data, 32768);
						}
						else
						{
							stream.add_int(0, i, -l_noise_data, 32768);
							stream.add_int(1, i, -r_noise_data, 32768);
						}

						if (hold)
						{
							hold--;
							continue;
						}

						hold =  hold_time;

						c += delta;
						int cnt = (c >> 12);
						c &= (1 << 12) - 1;
						for( ;cnt > 0; cnt--)
						{
							if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
							if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
							voice->noise_seed >>= 1;
						}
					}

					// update the counter and hold time for this voice
					voice->noise_counter = c;
					voice->noise_hold = hold;
				}
			}
			else
			{
				// save the counter for this voice
				uint32_t c = voice->counter;

				// only update if we have non-zero left volume
				if (lv)
				{
					const int16_t *lw = &m_waveform[lv][voice->waveform_select * 32];

					// generate sound into the buffer
					c = namco_update_one(stream, 0, lw, voice->counter, voice->frequency);
				}

				// only update if we have non-zero right volume
				if (rv)
				{
					const int16_t *rw = &m_waveform[rv][voice->waveform_select * 32];

					// generate sound into the buffer
					c = namco_update_one(stream, 1, rw, voice->counter, voice->frequency);
				}

				// update the counter for this voice
				voice->counter = c;
			}
		}
	}
	else
	{
		sound_channel *voice;

		// if no sound, we're done
		if (!m_sound_enable)
			return;

		// loop over each voice and add its contribution
		for (voice = m_channel_list; voice < m_last_channel; voice++)
		{
			int v = voice->volume[0];
			if (voice->noise_sw)
			{
				int f = voice->frequency & 0xff;

				// only update if we have non-zero volume
				if (v)
				{
					int hold_time = 1 << (m_f_fracbits - 16);
					int hold = voice->noise_hold;
					uint32_t delta = f << 4;
					uint32_t c = voice->noise_counter;
					int16_t noise_data = OUTPUT_LEVEL(0x07 * (v >> 1));

					// add our contribution
					for (int i = 0; i < stream.samples(); i++)
					{
						if (voice->noise_state)
							stream.add_int(0, i, noise_data, 32768);
						else
							stream.add_int(1, i, -noise_data, 32768);

						if (hold)
						{
							hold--;
							continue;
						}

						hold =  hold_time;

						c += delta;
						int cnt = (c >> 12);
						c &= (1 << 12) - 1;
						for( ;cnt > 0; cnt--)
						{
							if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
							if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
							voice->noise_seed >>= 1;
						}
					}

					// update the counter and hold time for this voice
					voice->noise_counter = c;
					voice->noise_hold = hold;
				}
			}
			else
			{
				// only update if we have non-zero volume
				if (v)
				{
					const int16_t *w = &m_waveform[v][voice->waveform_select * 32];

					// generate sound into buffer and update the counter for this voice
					voice->counter = namco_update_one(stream, 0, w, voice->counter, voice->frequency);
				}
			}
		}
	}
}

void namco_device::sound_stream_update(sound_stream &stream)
{
	namco_audio_device::sound_stream_update(stream);
}

void namco_15xx_device::sound_stream_update(sound_stream &stream)
{
	namco_audio_device::sound_stream_update(stream);
}

void namco_cus30_device::sound_stream_update(sound_stream &stream)
{
	namco_audio_device::sound_stream_update(stream);
}
