/***************************************************************************

    okim6295.h

    OKIM 6295 ADCPM sound chip.

****************************************************************************

    Library to transcode from an ADPCM source to raw PCM.
    Written by Buffoni Mirko in 08/06/97
    References: various sources and documents.

    R. Belmont 31/10/2003
    Updated to allow a driver to use both MSM6295s and "raw" ADPCM voices
    (gcpinbal). Also added some error trapping for MAME_DEBUG builds

****************************************************************************

    OKIM 6295 ADPCM chip:

    Command bytes are sent:

        1xxx xxxx = start of 2-byte command sequence, xxxxxxx is the sample
                    number to trigger
        abcd vvvv = second half of command; one of the abcd bits is set to
                    indicate which voice the v bits seem to be volumed

        0abc d000 = stop playing; one or more of the abcd bits is set to
                    indicate which voice(s)

    Status is read:

        ???? abcd = one bit per voice, set to 0 if nothing is playing, or
                    1 if it is active

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "okim6295.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// ADPCM state and tables
bool adpcm_state::s_tables_computed = false;
const INT8 adpcm_state::s_index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
int adpcm_state::s_diff_lookup[49*16];

// volume lookup table. The manual lists only 9 steps, ~3dB per step. Given the dB values,
// that seems to map to a 5-bit volume control. Any volume parameter beyond the 9th index
// results in silent playback.
const UINT8 okim6295_device::s_volume_table[16] =
{
	0x20,	//   0 dB
	0x16,	//  -3.2 dB
	0x10,	//  -6.0 dB
	0x0b,	//  -9.2 dB
	0x08,	// -12.0 dB
	0x06,	// -14.5 dB
	0x04,	// -18.0 dB
	0x03,	// -20.5 dB
	0x02,	// -24.0 dB
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

// default address map
static ADDRESS_MAP_START( okim6295, 0, 8 )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  okim6295_device_config - constructor
//-------------------------------------------------

okim6295_device_config::okim6295_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, tag, owner, clock),
	  device_config_sound_interface(mconfig, *this),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("samples", ENDIANNESS_LITTLE, 8, 18, 0, NULL, *ADDRESS_MAP_NAME(okim6295))
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *okim6295_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(okim6295_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *okim6295_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, okim6295_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void okim6295_device_config::device_config_complete()
{
	// copy the pin7 state from inline data
	m_pin7 = m_inline_data[INLINE_PIN7];
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *okim6295_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  okim6295_device - constructor
//-------------------------------------------------

okim6295_device::okim6295_device(running_machine &_machine, const okim6295_device_config &config)
	: device_t(_machine, config),
	  device_sound_interface(_machine, config, *this),
	  device_memory_interface(_machine, config, *this),
	  m_config(config),
	  m_command(-1),
	  m_bank_installed(false),
	  m_bank_offs(0),
	  m_stream(NULL),
	  m_pin7_state(m_config.m_pin7)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim6295_device::device_start()
{
	// create the stream
	int divisor = m_config.m_pin7 ? 132 : 165;
	m_stream = stream_create(this, 0, 1, clock() / divisor, this, static_stream_generate);

	state_save_register_device_item(this, 0, m_command);
	state_save_register_device_item(this, 0, m_bank_offs);
	for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++)
	{
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_playing);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_sample);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_count);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_adpcm.m_signal);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_adpcm.m_step);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_volume);
		state_save_register_device_item(this, voicenum, m_voice[voicenum].m_base_offset);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim6295_device::device_reset()
{
	stream_update(m_stream);
	for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++)
		m_voice[voicenum].m_playing = false;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void okim6295_device::device_post_load()
{
	set_bank_base(m_bank_offs);
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void okim6295_device::device_clock_changed()
{
	int divisor = m_pin7_state ? 132 : 165;
	stream_set_sample_rate(m_stream, clock() / divisor);
}


//-------------------------------------------------
//  stream_generate - handle update requests for
//  our sound stream
//-------------------------------------------------

STREAM_UPDATE( okim6295_device::static_stream_generate )
{
	reinterpret_cast<okim6295_device *>(param)->stream_generate(inputs, outputs, samples);
}

void okim6295_device::stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	// iterate over voices and accumulate sample data
	for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++)
		m_voice[voicenum].generate_adpcm(space(), outputs[0], samples);
}


//-------------------------------------------------
//  set_bank_base - old-style bank management;
//  assumes multiple 256k banks
//-------------------------------------------------

void okim6295_device::set_bank_base(offs_t base)
{
	// flush out anything pending
	stream_update(m_stream);

	// if we are setting a non-zero base, and we have no bank, allocate one
	if (!m_bank_installed && base != 0)
	{
		// override our memory map with a bank
		memory_install_read_bank(space(), 0x00000, 0x3ffff, 0, 0, tag());
		m_bank_installed = true;
	}

	// if we have a bank number, set the base pointer
	if (m_bank_installed)
	{
		m_bank_offs = base;
		memory_set_bankptr(&m_machine, tag(), m_region->base.u8 + base);
	}
}


//-------------------------------------------------
//  set_pin7 - change the state of pin 7, which
//  alters the frequency we output
//-------------------------------------------------

void okim6295_device::set_pin7(int pin7)
{
	m_pin7_state = pin7;
	device_clock_changed();
}


//-------------------------------------------------
//  status_read - read the status register
//-------------------------------------------------

READ8_DEVICE_HANDLER( okim6295_r )
{
	return downcast<okim6295_device *>(device)->status_read();
}

UINT8 okim6295_device::status_read()
{
	UINT8 result = 0xf0;	// naname expects bits 4-7 to be 1

	// set the bit to 1 if something is playing on a given channel
	stream_update(m_stream);
	for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++)
		if (m_voice[voicenum].m_playing)
			result |= 1 << voicenum;

	return result;
}


//-------------------------------------------------
//  data_write - write to the command/data register
//-------------------------------------------------

WRITE8_DEVICE_HANDLER( okim6295_w )
{
	downcast<okim6295_device *>(device)->data_write(data);
}

void okim6295_device::data_write(UINT8 data)
{
	// if a command is pending, process the second half
	if (m_command != -1)
	{
		// the manual explicitly says that it's not possible to start multiple voices at the same time
		int voicemask = data >> 4;
		if (voicemask != 0 && voicemask != 1 && voicemask != 2 && voicemask != 4 && voicemask != 8)
			popmessage("OKI6295 start %x contact MAMEDEV", voicemask);

		// update the stream
		stream_update(m_stream);

		// determine which voice(s) (voice is set by a 1 bit in the upper 4 bits of the second byte)
		for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++, voicemask >>= 1)
			if (voicemask & 1)
			{
				okim_voice &voice = m_voice[voicenum];

				// determine the start/stop positions
				offs_t base = m_command * 8;

				offs_t start = memory_raw_read_byte(space(), base + 0) << 16;
				start |= memory_raw_read_byte(space(), base + 1) << 8;
				start |= memory_raw_read_byte(space(), base + 2) << 0;
				start &= 0x3ffff;

				offs_t stop = memory_raw_read_byte(space(), base + 3) << 16;
				stop |= memory_raw_read_byte(space(), base + 4) << 8;
				stop |= memory_raw_read_byte(space(), base + 5) << 0;
				stop &= 0x3ffff;

				// set up the voice to play this sample
				if (start < stop)
				{
					if (!voice.m_playing) // fixes Got-cha and Steel Force
					{
						voice.m_playing = true;
						voice.m_base_offset = start;
						voice.m_sample = 0;
						voice.m_count = 2 * (stop - start + 1);

						// also reset the ADPCM parameters
						voice.m_adpcm.reset();
						voice.m_volume = s_volume_table[data & 0x0f];
					}
					else
						logerror("OKIM6295:'%s' requested to play sample %02x on non-stopped voice\n",tag(),m_command);
				}

				// invalid samples go here
				else
				{
					logerror("OKIM6295:'%s' requested to play invalid sample %02x\n",tag(),m_command);
					voice.m_playing = false;
				}
			}

		// reset the command
		m_command = -1;
	}

	// if this is the start of a command, remember the sample number for next time
	else if (data & 0x80)
		m_command = data & 0x7f;

	// otherwise, see if this is a silence command
	else
	{
		// update the stream, then turn it off
		stream_update(m_stream);

		// determine which voice(s) (voice is set by a 1 bit in bits 3-6 of the command
		int voicemask = data >> 3;
		for (int voicenum = 0; voicenum < OKIM6295_VOICES; voicenum++, voicemask >>= 1)
			if (voicemask & 1)
				m_voice[voicenum].m_playing = false;
	}
}



//**************************************************************************
//  OKIM VOICE
//**************************************************************************

//-------------------------------------------------
//  okim_voice - constructor
//-------------------------------------------------

okim6295_device::okim_voice::okim_voice()
	: m_playing(false),
	  m_base_offset(0),
	  m_sample(0),
	  m_count(0),
	  m_volume(0)
{
}


//-------------------------------------------------
//  generate_adpcm - generate ADPCM samples and
//  add them to an output stream
//-------------------------------------------------

void okim6295_device::okim_voice::generate_adpcm(const address_space *space, stream_sample_t *buffer, int samples)
{
	// skip if not active
	if (!m_playing)
		return;

	// loop while we still have samples to generate
	while (samples-- != 0)
	{
		// fetch the next sample byte
		int nibble = memory_raw_read_byte(space, m_base_offset + m_sample / 2) >> (((m_sample & 1) << 2) ^ 4);

		// output to the buffer, scaling by the volume
		// signal in range -2048..2047, volume in range 2..32 => signal * volume / 2 in range -32768..32767
		*buffer++ += m_adpcm.clock(nibble) * m_volume / 2;

		// next!
		if (++m_sample >= m_count)
		{
			m_playing = false;
			break;
		}
	}
}



//**************************************************************************
//  ADPCM STATE HELPER
//**************************************************************************

//-------------------------------------------------
//  reset - reset the ADPCM state
//-------------------------------------------------

void adpcm_state::reset()
{
	// reset the signal/step
	m_signal = -2;
	m_step = 0;
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

INT16 adpcm_state::clock(UINT8 nibble)
{
	// update the signal
	m_signal += s_diff_lookup[m_step * 16 + (nibble & 15)];

	// clamp to the maximum
	if (m_signal > 2047)
		m_signal = 2047;
	else if (m_signal < -2048)
		m_signal = -2048;

	// adjust the step size and clamp
	m_step += s_index_shift[nibble & 7];
	if (m_step > 48)
		m_step = 48;
	else if (m_step < 0)
		m_step = 0;

	// return the signal
	return m_signal;
}


//-------------------------------------------------
//  compute_tables - precompute tables for faster
//  sound generation
//-------------------------------------------------

void adpcm_state::compute_tables()
{
	// skip if we already did it
	if (s_tables_computed)
		return;
	s_tables_computed = true;

	// nibble to bit map
	static const INT8 nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	// loop over all possible steps
	for (int step = 0; step <= 48; step++)
	{
		// compute the step value
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		// loop over all nibbles and compute the difference
		for (int nib = 0; nib < 16; nib++)
		{
			s_diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}
