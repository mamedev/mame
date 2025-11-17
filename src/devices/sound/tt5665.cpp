// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Aaron Giles,cam900
/***************************************************************************

    tt5665.cpp

    Tontek TT5665 ADCPM sound chip.

****************************************************************************

    Emulation core by cam900
    Base code by OKI MSM6295 ADPCM emulation(okim6295.cpp),
    by Mirko Buffoni,Aaron Giles

****************************************************************************

    Tontek TT5665 ADPCM chip:

    Command bytes are sent:

        1xxx xxxx = start of 2-byte command sequence, xxxxxxx is the sample
                    number to trigger
        xxab vvvv = second half of command; the ab bits are voice number,
                    the x bits are high bits of sample number, the v bits
                    are volume

        0abc d000 = stop playing; one or more of the abcd bits is set to
                    indicate which voice(s)

    Status is read:

        ???? abcd = one bit per voice, set to 0 if nothing is playing, or
                    1 if it is active

    ADPCM data and volume is compatible with OKI MSM6295, but some Command
    byte is slightly different.

    Compares to other Tontek OKI MSM6295 derivatives:
    |--------------------|-----------------|-----------------|-----------------|
    | Part number        |     TT5665      |     TT6297      |     TT6298      |
    |--------------------|-----------------|-----------------|-----------------|
    | Address Bus width  | 24bit (128MBit) | 24bit (128MBit) | 23bit (64MBit)  |
    |--------------------|-----------------|-----------------|-----------------|
    | Output channels    |   2 (DAOL/R)    |     1 (DAO)     |     1 (DAO)     |
    |--------------------|-----------------|-----------------|-----------------|
    | Variable Frequency |  SS, S0-1(For   |   Not exists    |   Not exists    |
    | Pins               |  DAOL only)     |                 |                 |
    |--------------------|-----------------|-----------------|-----------------|
    | Crystal Oscillator |   2 (XT, XTB)   |     1 (XT)      |   2 (XT, XTB)   |
    | Pins               |                 |                 |                 |
    |--------------------|-----------------|-----------------|-----------------|
    | Frequency Divider  | DAOR: 136, 170  |       136       |       136       |
    |                    | DAOL: DAOR * n  |                 |                 |
    |                    |  (n : 1,2,4,8)  |                 |                 |
    |--------------------|-----------------|-----------------|-----------------|
    These chips share Command bytes and ADPCM format.

    Frequency Table (Unit: KHz) :
                      |---------------------------|---------------------------|
                      |         SS = "1"          |         SS = "0"          |
    |-----------------|---------------------------|---------------------------|
    |     (S1,S0)     |  00  |  01  |  10  |  11  |  00  |  01  |  10  |  11  |
    |----------|------|------|------|------|------|------|------|------|------|
    | OSC =    | DAOR |  8.0 |  8.0 |  8.0 |  8.0 |  6.4 |  6.4 |  6.4 |  6.4 |
    | 1.088MHz |------|------|------|------|------|------|------|------|------|
    |          | DAOL |  8.0 |  4.0 |  2.0 |  1.0 |  6.4 |  3.2 |  1.6 |  0.8 |
    |----------|------|------|------|------|------|------|------|------|------|
    | OSC =    | DAOR | 16.0 | 16.0 | 16.0 | 16.0 | 12.8 | 12.8 | 12.8 | 12.8 |
    | 2.176MHz |------|------|------|------|------|------|------|------|------|
    |          | DAOL | 16.0 |  8.0 |  4.0 |  2.0 | 12.8 |  6.4 |  3.2 |  1.6 |
    |----------|------|------|------|------|------|------|------|------|------|
    | OSC =    | DAOR | 32.0 | 32.0 | 32.0 | 32.0 | 25.6 | 25.6 | 25.6 | 25.6 |
    | 4.352MHz |------|------|------|------|------|------|------|------|------|
    |          | DAOL | 32.0 | 16.0 |  8.0 |  4.0 | 25.6 | 12.8 |  6.4 |  3.2 |
    |----------|------|------|------|------|------|------|------|------|------|

***************************************************************************/

#include "emu.h"
#include "tt5665.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TT5665, tt5665_device, "tt5665", "Tontek TT5665 ADPCM Voice Synthesis LSI")

// same as MSM6295
const sound_stream::sample_t tt5665_device::s_volume_table[16] =
{
	sound_stream::sample_t(0x20) / sound_stream::sample_t(0x20),   //   0 dB
	sound_stream::sample_t(0x16) / sound_stream::sample_t(0x20),   //  -3.2 dB
	sound_stream::sample_t(0x10) / sound_stream::sample_t(0x20),   //  -6.0 dB
	sound_stream::sample_t(0x0b) / sound_stream::sample_t(0x20),   //  -9.2 dB
	sound_stream::sample_t(0x08) / sound_stream::sample_t(0x20),   // -12.0 dB
	sound_stream::sample_t(0x06) / sound_stream::sample_t(0x20),   // -14.5 dB
	sound_stream::sample_t(0x04) / sound_stream::sample_t(0x20),   // -18.0 dB
	sound_stream::sample_t(0x03) / sound_stream::sample_t(0x20),   // -20.5 dB
	sound_stream::sample_t(0x02) / sound_stream::sample_t(0x20),   // -24.0 dB
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
	sound_stream::sample_t(0x00) / sound_stream::sample_t(0x20),
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tt5665_device - constructor
//-------------------------------------------------

tt5665_device::tt5665_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TT5665, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_ss_state(~u8(0))
	, m_s0_s1_state(0)
	, m_command{-1,-1}
	, m_stream(nullptr)
	, m_daol_output(0.0)
	, m_daol_timing(0)
{
}


//-------------------------------------------------
//  device_validity_check - device-specific checks
//-------------------------------------------------

void tt5665_device::device_validity_check(validity_checker &valid) const
{
	if ((SS_LOW != m_ss_state) && (SS_HIGH != m_ss_state))
		osd_printf_error("Initial pin SS state not configured\n");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tt5665_device::device_start()
{
	if ((SS_LOW != m_ss_state) && (SS_HIGH != m_ss_state))
		m_ss_state = 0;

	// create the stream
	m_stream = stream_alloc(0, 2, clock() / freq_divider());

	save_item(NAME(m_command));
	save_item(NAME(m_ss_state));
	save_item(NAME(m_s0_s1_state));
	save_item(NAME(m_daol_output));
	save_item(NAME(m_daol_timing));

	save_item(STRUCT_MEMBER(m_voice, m_playing));
	save_item(STRUCT_MEMBER(m_voice, m_sample));
	save_item(STRUCT_MEMBER(m_voice, m_count));
	save_item(STRUCT_MEMBER(m_voice, m_volume));
	save_item(STRUCT_MEMBER(m_voice, m_base_offset));
	for (int voicenum = 0; voicenum < TT5665_VOICES * 2; voicenum++)
	{
		save_item(NAME(m_voice[voicenum].m_adpcm.m_signal), voicenum);
		save_item(NAME(m_voice[voicenum].m_adpcm.m_step), voicenum);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tt5665_device::device_reset()
{
	m_stream->update();
	for (auto & elem : m_voice)
		elem.m_playing = false;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void tt5665_device::device_post_load()
{
	device_clock_changed();
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void tt5665_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / freq_divider());
}


//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

void tt5665_device::sound_stream_update(sound_stream &stream)
{
	bool update_daol = false;

	// iterate over voices and accumulate sample data
	// loop while we still have samples to generate
	for (int s = 0; s < stream.samples(); s++)
	{
		// adjust DAOL clock timing
		m_daol_timing--;
		if (m_daol_timing <= 0)
		{
			update_daol = true;
			m_daol_output = 0;
		}
		s32 daor_output = 0;

		for (int b = 0; b < TT5665_VOICES; b++)
		{
			// refresh DAOL output
			if (update_daol)
				m_voice[b].generate_adpcm(*this, &m_daol_output);

			// refresh DAOR output
			m_voice[b + 4].generate_adpcm(*this, &daor_output);
		}
		stream.put_int(0, s, m_daol_output, 2048);
		stream.put_int(1, s, daor_output, 2048);
		if (update_daol)
		{
			update_daol = false;
			m_daol_timing = 1 << m_s0_s1_state;
		}
	}
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void tt5665_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  set_ss - change the state of pin SS, which
//  alters the frequency we output
//-------------------------------------------------

void tt5665_device::set_ss(bool ss)
{
	assert(started());
	m_ss_state = ss ? 1 : 0;
	device_clock_changed();
}


//-------------------------------------------------
//  set_s0_s1 - change the state of S0 & S1, which
//  alters the frequency we output
//-------------------------------------------------

void tt5665_device::set_s0_s1(int s0_s1)
{
	m_stream->update();
	assert(started());
	m_s0_s1_state = s0_s1 & 3;
}


//-------------------------------------------------
//  read - read the status register
//-------------------------------------------------

u8 tt5665_device::read(offs_t offset)
{
	const u8 ch = offset & 1;
	u8 result = 0xf0;    // naname expects bits 4-7 to be 1

	// set the bit to 1 if something is playing on a given channel
	m_stream->update();
	for (int voicenum = 0; voicenum < TT5665_VOICES; voicenum++)
		if (m_voice[voicenum + (ch * 4)].m_playing)
			result |= 1 << voicenum;

	return result;
}


//-------------------------------------------------
//  write - write to the command register
//-------------------------------------------------

void tt5665_device::write(offs_t offset, u8 data)
{
	const u8 ch = offset & 1;
	if (m_command[ch] != -1) // if a command is pending, process the second half
	{
		// the chip can only play one voice at a time, but has extra phrase spaces. that's are main difference from MSM 6295.
		const int voicemask = (data >> 4) & 3;
		const int basemask = (data >> 6) & 3;

		// update the stream
		m_stream->update();

		// determine which voice
		tt5665_voice &voice = m_voice[voicemask + (ch * 4)];

		const int sample_ind = (basemask << 7) | m_command[ch];
		if (!voice.m_playing) // TODO: Same as MSM6295?
		{
			// determine the start/stop positions
			const offs_t base = sample_ind * 8;

			offs_t start = read_byte(base + 0) << 16;
			start |= read_byte(base + 1) << 8;
			start |= read_byte(base + 2) << 0;
			start &= 0xffffff;

			offs_t stop = read_byte(base + 3) << 16;
			stop |= read_byte(base + 4) << 8;
			stop |= read_byte(base + 5) << 0;
			stop &= 0xffffff;

			if (start < stop)
			{
				// set up the voice to play this sample
				voice.m_playing = true;
				voice.m_base_offset = start;
				voice.m_sample = 0;
				voice.m_count = 2 * (stop - start + 1);

				// also reset the ADPCM parameters
				voice.m_adpcm.reset();
				voice.m_volume = s_volume_table[data & 0x0f];
			}
			else
			{
				// invalid samples go here
				logerror("Requested to play invalid sample %04x\n", sample_ind);
			}
		}
		else
		{
			logerror("Requested to play sample %04x on non-stopped voice\n", sample_ind);
		}

		// reset the command
		m_command[ch] = -1;
	}
	else if (data & 0x80) // if this is the start of a command, remember the sample number for next time
	{
		m_command[ch] = data & 0x7f;
	}
	else // otherwise, see if this is a silence command
	{
		// update the stream, then turn it off
		m_stream->update();

		// determine which voice(s) (voice is set by a 1 bit in bits 3-6 of the command
		int voicemask = data >> 3;
		for (int voicenum = 0; voicenum < TT5665_VOICES; voicenum++, voicemask >>= 1)
			if (voicemask & 1)
				m_voice[voicenum + (ch * 4)].m_playing = false;
	}
}



//**************************************************************************
//  TT5665 VOICE
//**************************************************************************

//-------------------------------------------------
//  tt5665_voice - constructor
//-------------------------------------------------

tt5665_device::tt5665_voice::tt5665_voice()
	: m_playing(false)
	, m_base_offset(0)
	, m_sample(0)
	, m_count(0)
	, m_volume(0)
{
}


//-------------------------------------------------
//  generate_adpcm - generate ADPCM samples and
//  add them to an output stream
//-------------------------------------------------

void tt5665_device::tt5665_voice::generate_adpcm(device_rom_interface &rom, s32 *buffer)
{
	// skip if not active
	if (!m_playing)
		return;

	// fetch the next sample byte
	const int nibble = rom.read_byte(m_base_offset + m_sample / 2) >> (((m_sample & 1) << 2) ^ 4);

	// output to the buffer, scaling by the volume
	// signal in range -2048..2047; 12 bit built-in DAC
	*buffer += m_adpcm.clock(nibble) * m_volume;

	// next!
	if (++m_sample >= m_count)
	{
		m_playing = false;
	}
}
