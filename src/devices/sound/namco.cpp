// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/***************************************************************************

    NAMCO WSG sound driver.

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

#include <algorithm>

#define LOG_STREAM (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSTREAM(...)  LOGMASKED(LOG_STREAM, __VA_ARGS__)

// quality parameter: internal sample rate is 192 KHz, output is 48 KHz
static constexpr uint32_t INTERNAL_RATE = 192000;

void namco_15xx_device::amap(address_map &map)
{
	map(0x000, 0x03f).rw(FUNC(namco_15xx_device::namco_15xx_r), FUNC(namco_15xx_device::namco_15xx_w));
	map(0x040, 0x3ff).ram().share(m_sharedram);
}

void namco_cus30_device::amap(address_map &map)
{
	map(0x000, 0x0ff).ram().share(m_waveram);
	map(0x100, 0x13f).rw(FUNC(namco_cus30_device::cus30_r), FUNC(namco_cus30_device::cus30_w));
	map(0x140, 0x3ff).ram().share(m_sharedram);
}

DEFINE_DEVICE_TYPE(NAMCO_WSG,   namco_wsg_device,   "namco_wsg",   "Namco WSG")
DEFINE_DEVICE_TYPE(POLEPOS_WSG, polepos_wsg_device, "polepos_wsg", "Namco Pole Position WSG")
DEFINE_DEVICE_TYPE(NAMCO_15XX,  namco_15xx_device,  "namco_15xx",  "Namco 15xx")
DEFINE_DEVICE_TYPE(NAMCO_CUS30, namco_cus30_device, "namco_cus30", "Namco CUS30")

template <unsigned Voices, bool Packed>
namco_audio_device<Voices, Packed>::namco_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_BIG, 8, 8)
	, m_wave_ptr(*this, DEVICE_SELF)
	, m_sound_enable(false)
	, m_stream(nullptr)
	, m_namco_clock(0)
	, m_sample_rate(0)
	, m_f_fracbits(0)
	, m_soundregs(nullptr)
{
}

namco_wsg_device::namco_wsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device<3, false>(mconfig, NAMCO_WSG, tag, owner, clock)
{
}

polepos_wsg_device::polepos_wsg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device<8, false>(mconfig, POLEPOS_WSG, tag, owner, clock)
{
}

namco_15xx_device::namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device<8, false>(mconfig, NAMCO_15XX, tag, owner, clock)
	, m_sharedram(*this, "sharedram")
{
}

namco_cus30_device::namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_audio_device<8, true>(mconfig, NAMCO_CUS30, tag, owner, clock)
	, m_sharedram(*this, "sharedram")
	, m_waveram(*this, "waveram")
	, m_stereo(false)
{
}

template <unsigned Voices, bool Packed>
device_memory_interface::space_config_vector namco_audio_device<Voices, Packed>::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_data_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

template <unsigned Voices, bool Packed>
void namco_audio_device<Voices, Packed>::device_start()
{
	// build the waveform table
	if (!has_configured_map(0) && m_wave_ptr)
		space(0).install_rom(0x00, 0xff, m_wave_ptr.target());

	space(0).cache(m_data);

	// start with sound enabled, many games don't have a sound enable register
	m_sound_enable = true;

	// reset all the voices
	for (auto &voice : m_channel_list)
	{
		voice.frequency = 0;
		voice.volume[0] = voice.volume[1] = voice.volume[2] = voice.volume[3] = 0;
		voice.waveform_select = 0;
		voice.counter = 0;
		voice.noise_sw = 0;
		voice.noise_state = 0;
		voice.noise_seed = 1;
		voice.noise_counter = 0;
		voice.noise_hold = 0;
	}

	// register with the save state system
	save_item(NAME(m_sound_enable));

	save_item(STRUCT_MEMBER(m_channel_list, frequency));
	save_item(STRUCT_MEMBER(m_channel_list, counter));
	save_item(STRUCT_MEMBER(m_channel_list, volume));
	save_item(STRUCT_MEMBER(m_channel_list, noise_sw));
	save_item(STRUCT_MEMBER(m_channel_list, noise_state));
	save_item(STRUCT_MEMBER(m_channel_list, noise_seed));
	save_item(STRUCT_MEMBER(m_channel_list, noise_hold));
	save_item(STRUCT_MEMBER(m_channel_list, noise_counter));
	save_item(STRUCT_MEMBER(m_channel_list, waveform_select));
}


void namco_wsg_device::device_start()
{
	namco_audio_device::device_start();

	// get stream channels
	m_stream = stream_alloc(0, 1, INTERNAL_RATE);

	m_soundregs = make_unique_clear<uint8_t[]>(0x20);
	save_pointer(NAME(m_soundregs), 0x20);
}


void polepos_wsg_device::device_start()
{
	namco_audio_device::device_start();

	// get stream channels
	m_stream = stream_alloc(0, 4, INTERNAL_RATE);

	m_soundregs = make_unique_clear<uint8_t[]>(0x40);
	save_pointer(NAME(m_soundregs), 0x40);
}


void namco_15xx_device::device_start()
{
	namco_audio_device::device_start();

	// get stream channels
	m_stream = stream_alloc(0, 1, INTERNAL_RATE);

	// initialize
	std::fill_n(&m_sharedram[0], m_sharedram.length(), 0);

	m_soundregs = make_unique_clear<uint8_t[]>(0x40);
	save_pointer(NAME(m_soundregs), 0x40);
}


void namco_cus30_device::device_start()
{
	// waveform RAM is internal
	if (!has_configured_map(0))
		space(0).install_ram(0x00, 0xff, m_waveram.target());

	namco_audio_device::device_start();

	// get stream channels
	m_stream = stream_alloc(0, m_stereo ? 2 : 1, INTERNAL_RATE);

	// initialize
	std::fill_n(&m_waveram[0], m_waveram.length(), 0);
	std::fill_n(&m_sharedram[0], m_sharedram.length(), 0);

	m_soundregs = make_unique_clear<uint8_t[]>(0x40);
	save_pointer(NAME(m_soundregs), 0x40);
}


template <unsigned Voices, bool Packed>
void namco_audio_device<Voices, Packed>::device_clock_changed()
{
	int clock_multiple;

	// adjust internal clock
	m_namco_clock = clock();
	for (clock_multiple = 0; m_namco_clock < INTERNAL_RATE; clock_multiple++)
		m_namco_clock *= 2;

	m_f_fracbits = clock_multiple + 15;

	// adjust output clock
	m_sample_rate = m_namco_clock;

	LOGSTREAM("%s: Namco: freq fractional bits = %d: internal freq = %d, output freq = %d\n", machine().describe_context(), m_f_fracbits, m_namco_clock, m_sample_rate);

	m_stream->set_sample_rate(m_sample_rate);
}

template <unsigned Voices, bool Packed>
inline int namco_audio_device<Voices, Packed>::waveform_r(uint16_t pos)
{
	if (Packed) // use full byte, first 4 high bits, then low 4 bits
		return ((m_data.read_byte((pos >> 1) & 0xff) >> (BIT(~pos, 0) << 2)) & 0x0f) - 8;
	else // use only low 4 bits
		return (m_data.read_byte(pos & 0xff) & 0x0f) - 8;
}


// generate sound by oversampling
template <unsigned Voices, bool Packed>
uint32_t namco_audio_device<Voices, Packed>::namco_update_one(sound_stream &stream, int output, uint16_t select, int volume, uint32_t counter, uint32_t freq)
{
	select <<= 5;
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		const int waveform = waveform_r(select + waveform_position(counter));
		stream.add_int(output, sampindex, waveform * volume, MIX_RES);
		counter += freq;
	}

	return counter;
}


template <unsigned Voices, bool Packed>
void namco_audio_device<Voices, Packed>::sound_enable_w(int state)
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

void namco_wsg_device::pacman_sound_w(offs_t offset, uint8_t data)
{
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

	if (ch >= MAX_VOICES)
		return;

	// recompute the voice parameters
	sound_channel &voice = m_channel_list[ch];
	switch (offset - ch * 5)
	{
	case 0x05:
		voice.waveform_select = data & 7;
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
		// the frequency has 20 bits
		// the first voice has extra frequency bits
		voice.frequency = (ch == 0) ? m_soundregs[0x10] : 0;
		voice.frequency += (m_soundregs[ch * 5 + 0x11] << 4);
		voice.frequency += (m_soundregs[ch * 5 + 0x12] << 8);
		voice.frequency += (m_soundregs[ch * 5 + 0x13] << 12);
		voice.frequency += (m_soundregs[ch * 5 + 0x14] << 16); // always 0
		break;

	case 0x15:
		voice.volume[0] = data;
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

uint8_t polepos_wsg_device::polepos_sound_r(offs_t offset)
{
	return m_soundregs[offset];
}

void polepos_wsg_device::polepos_sound_w(offs_t offset, uint8_t data)
{
	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	const int ch = (offset & 0x1f) >> 2;

	// recompute the voice parameters
	sound_channel &voice = m_channel_list[ch];
	switch (offset & 0x23)
	{
	case 0x00:
	case 0x01:
		// the frequency has 16 bits
		voice.frequency = m_soundregs[ch * 4 + 0x00];
		voice.frequency += m_soundregs[ch * 4 + 0x01] << 8;
		break;

	case 0x23:
		voice.waveform_select = data & 7;
		[[fallthrough]];
	case 0x02:
	case 0x03:
		// front speakers ?
		voice.volume[0] = m_soundregs[ch * 4 + 0x03] >> 4;
		voice.volume[1] = m_soundregs[ch * 4 + 0x03] & 0x0f;
		// rear speakers ?
		voice.volume[2] = m_soundregs[ch * 4 + 0x23] >> 4;
		voice.volume[3] = m_soundregs[ch * 4 + 0x02] >> 4;

		// if 54XX or 52XX selected, silence this voice
		if (m_soundregs[ch * 4 + 0x23] & 8)
			voice.volume[0] = voice.volume[1] = voice.volume[2] = voice.volume[3] = 0;
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
	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	const int ch = offset >> 3;
	if (ch >= MAX_VOICES)
		return;

	// recompute the voice parameters
	sound_channel &voice = m_channel_list[ch];
	switch (offset & 7)
	{
	case 0x02:
		voice.counter &= util::make_bitmask<uint32_t>(m_f_fracbits);
		voice.counter |= uint32_t(data & 0x1f) << m_f_fracbits;
		break;

	case 0x03:
		voice.volume[0] = data & 0x0f;
		break;

	case 0x06:
		voice.waveform_select = (data >> 4) & 7;
		[[fallthrough]];
	case 0x04:
	case 0x05:
		// the frequency has 20 bits
		voice.frequency = m_soundregs[ch * 8 + 0x04];
		voice.frequency += m_soundregs[ch * 8 + 0x05] << 8;
		voice.frequency += (m_soundregs[ch * 8 + 0x06] & 15) << 16; // high bits are from here
		break;
	}
}

uint8_t namco_15xx_device::namco_15xx_r(offs_t offset)
{
	return m_soundregs[offset];
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

void namco_cus30_device::cus30_w(offs_t offset, uint8_t data)
{
	// verify the offset
	if (offset > 63)
	{
		logerror("%s: NAMCOS1 sound: Attempting to write past the 64 registers segment\n", machine().describe_context());
		return;
	}

	if (m_soundregs[offset] == data)
		return;

	// update the streams
	m_stream->update();

	// set the register
	m_soundregs[offset] = data;

	const int ch = offset >> 3;
	if (ch >= MAX_VOICES)
		return;

	// recompute the voice parameters
	sound_channel &voice = m_channel_list[ch];
	switch (offset & 7)
	{
	case 0x00:
		voice.volume[0] = data & 0x0f;
		break;

	case 0x01:
		voice.waveform_select = (data >> 4) & 15;
		[[fallthrough]];
	case 0x02:
	case 0x03:
		// the frequency has 20 bits
		voice.frequency = (m_soundregs[ch * 8 + 0x01] & 15) << 16; // high bits are from here
		voice.frequency += m_soundregs[ch * 8 + 0x02] << 8;
		voice.frequency += m_soundregs[ch * 8 + 0x03];
		break;

	case 0x04:
		{
			voice.volume[1] = data & 0x0f;

			sound_channel &voice2 = m_channel_list[(ch + 1) & 7];
			voice2.noise_sw = BIT(data, 7);
		}
		break;
	}
}

uint8_t namco_cus30_device::cus30_r(offs_t offset)
{
	const int ch = offset >> 3;
	const int reg = offset & 7;

	// reading from register 5 returns the counter (used by baraduke)
	if (ch < MAX_VOICES && reg == 5)
	{
		m_stream->update();
		return waveform_position(m_channel_list[ch].counter);
	}

	return m_soundregs[offset];
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

template <unsigned Voices, bool Packed>
void namco_audio_device<Voices, Packed>::sound_stream_update(sound_stream &stream)
{
	// if no sound, we're done
	if (!m_sound_enable)
		return;

	// loop over each voice and add its contribution
	for (auto &voice : m_channel_list)
	{
		const int v = voice.volume[0];
		{
			// only update if we have non-zero volume
			if (v)
			{
				// generate sound into buffer and update the counter for this voice
				voice.counter = namco_update_one(stream, 0, voice.waveform_select, v, voice.counter, voice.frequency);
			}
		}
	}
}

void namco_wsg_device::sound_stream_update(sound_stream &stream)
{
	namco_audio_device::sound_stream_update(stream);
}

void polepos_wsg_device::sound_stream_update(sound_stream &stream)
{
	// if no sound, we're done
	if (!m_sound_enable)
		return;

	// loop over each voice and add its contribution
	for (auto &voice : m_channel_list)
	{
		const int flv = voice.volume[0];
		const int frv = voice.volume[1];
		const int rlv = voice.volume[2];
		const int rrv = voice.volume[3];

		// save the counter for this voice
		uint32_t c = voice.counter;

		// only update if we have non-zero front left volume
		if (flv)
		{
			// generate sound into the buffer
			c = namco_update_one(stream, 0, voice.waveform_select, flv, voice.counter, voice.frequency);
		}

		// only update if we have non-zero front right volume
		if (frv)
		{
			// generate sound into the buffer
			c = namco_update_one(stream, 1, voice.waveform_select, frv, voice.counter, voice.frequency);
		}


		// only update if we have non-zero rear left volume
		if (rlv)
		{
			// generate sound into the buffer
			c = namco_update_one(stream, 2, voice.waveform_select, rlv, voice.counter, voice.frequency);
		}

		// only update if we have non-zero rear right volume
		if (rrv)
		{
			// generate sound into the buffer
			c = namco_update_one(stream, 3, voice.waveform_select, rrv, voice.counter, voice.frequency);
		}

		// update the counter for this voice
		voice.counter = c;
	}
}

void namco_15xx_device::sound_stream_update(sound_stream &stream)
{
	namco_audio_device::sound_stream_update(stream);
}

void namco_cus30_device::sound_stream_update(sound_stream &stream)
{
	// if no sound, we're done
	if (!m_sound_enable)
		return;

	// loop over each voice and add its contribution
	for (auto &voice : m_channel_list)
	{
		const int lv = voice.volume[0];
		const int rv = voice.volume[1];

		if (voice.noise_sw)
		{
			const int f = voice.frequency & 0xff;

			// only update if we have non-zero volume
			if (lv || rv)
			{
				const int hold_time = 1 << (m_f_fracbits - 16);
				int hold = voice.noise_hold;
				const uint32_t delta = f << 4;
				uint32_t c = voice.noise_counter;
				const int16_t l_noise_data = 0x07 * (lv >> 1);
				const int16_t r_noise_data = m_stereo ? (0x07 * (rv >> 1)) : 0;

				// add our contribution
				for (int i = 0; i < stream.samples(); i++)
				{
					if (voice.noise_state)
					{
						stream.add_int(0, i, l_noise_data, MIX_RES);
						if (m_stereo)
							stream.add_int(1, i, r_noise_data, MIX_RES);
					}
					else
					{
						stream.add_int(0, i, -l_noise_data, MIX_RES);
						if (m_stereo)
							stream.add_int(1, i, -r_noise_data, MIX_RES);
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
					for ( ;cnt > 0; cnt--)
					{
						if ((voice.noise_seed + 1) & 2) voice.noise_state ^= 1;
						if (voice.noise_seed & 1) voice.noise_seed ^= 0x28000;
						voice.noise_seed >>= 1;
					}
				}

				// update the counter and hold time for this voice
				voice.noise_counter = c;
				voice.noise_hold = hold;
			}
		}
		else
		{
			// save the counter for this voice
			uint32_t c = voice.counter;

			// only update if we have non-zero left volume
			if (lv)
			{
				// generate sound into the buffer
				c = namco_update_one(stream, 0, voice.waveform_select, lv, voice.counter, voice.frequency);
			}

			// only update if we have non-zero right volume
			if (rv && m_stereo)
			{
				// generate sound into the buffer
				c = namco_update_one(stream, 1, voice.waveform_select, rv, voice.counter, voice.frequency);
			}

			// update the counter for this voice
			voice.counter = c;
		}
	}
}

// template class instantiation
template class namco_audio_device<3, false>;
template class namco_audio_device<8, false>;
template class namco_audio_device<8, true>;
