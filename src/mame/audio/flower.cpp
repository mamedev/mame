// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Flower custom sound chip

    Similar to Wiping and Namco 15xx designs

    TODO:
    - several unknown registers (effects and unknown register tied to repeat port);
    - repeat certainly needs a cutoff, which is unknown about how it works;

***************************************************************************/

#include "emu.h"
#include "flower.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(FLOWER_CUSTOM, flower_sound_device, "flower_sound", "Flower Custom Sound")

// TODO: select() unsupported by DEVICE_ADDRESS_MAP, so we need a trampoline here
void flower_sound_device::regs_map(address_map &map)
{
	map(0x00, 0x03).select(0x38).w(FUNC(flower_sound_device::frequency_w));
	map(0x04, 0x04).select(0x38).w(FUNC(flower_sound_device::repeat_w));
	map(0x05, 0x05).select(0x38).w(FUNC(flower_sound_device::unk_w));
	map(0x07, 0x07).select(0x38).w(FUNC(flower_sound_device::volume_w));
	map(0x40, 0x45).select(0x38).w(FUNC(flower_sound_device::start_address_w));
	map(0x47, 0x47).select(0x38).w(FUNC(flower_sound_device::sample_trigger_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  flower_sound_device - constructor
//-------------------------------------------------

flower_sound_device::flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FLOWER_CUSTOM, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_memory_interface(mconfig, *this),
	  m_io_space_config("io", ENDIANNESS_LITTLE, 8, 7, 0, address_map_constructor(FUNC(flower_sound_device::regs_map), this)),
	  m_stream(nullptr),
	  m_mixer_table(nullptr),
	  m_mixer_lookup(nullptr),
	  m_mixer_buffer(nullptr),
	  m_last_channel(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void flower_sound_device::device_start()
{
	m_iospace = &space(AS_IO);
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock()/2);

	m_mixer_buffer = make_unique_clear<short[]>(clock()/2);
	make_mixer_table(MAX_VOICES, defgain);

	m_last_channel = m_channel_list + MAX_VOICES;

	m_sample_rom = machine().root_device().memregion("samples")->base();
	m_volume_rom = machine().root_device().memregion("soundvol")->base();

	for (int i = 0; i < MAX_VOICES; i++)
	{
		save_item(NAME(m_channel_list[i].start_address), i);
		save_item(NAME(m_channel_list[i].position), i);
		save_item(NAME(m_channel_list[i].frequency), i);
		save_item(NAME(m_channel_list[i].volume), i);
		save_item(NAME(m_channel_list[i].volume_bank), i);
		save_item(NAME(m_channel_list[i].effect), i);
		save_item(NAME(m_channel_list[i].enable), i);
		save_item(NAME(m_channel_list[i].repeat), i);

		// assign a channel number (debugger aid)
		m_channel_list[i].channel_number = i;
	}
}


/* build a table to divide by the number of voices; gain is specified as gain*16 */
void flower_sound_device::make_mixer_table(int voices, int gain)
{
	/* allocate memory */
	m_mixer_table = make_unique_clear<int16_t[]>(256 * voices);

	/* find the middle of the table */
	m_mixer_lookup = m_mixer_table.get() + (128 * voices);

	/* fill in the table - 16 bit case */
	for (int i = 0; i < voices * 128; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		m_mixer_lookup[ i] = val;
		m_mixer_lookup[-i] = -val;
	}
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void flower_sound_device::device_reset()
{
	for (fl_sound_channel *voice = m_channel_list; voice < m_last_channel; voice++)
	{
		voice->start_address = 0;
		voice->position = 0;
		voice->volume = 0;
		voice->enable = false;
		voice->repeat = false;
	}
}

void flower_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];
	short *mix;
	uint8_t raw_sample;

	memset(m_mixer_buffer.get(), 0, samples * sizeof(short));

	for (fl_sound_channel *voice = m_channel_list; voice < m_last_channel; voice++)
	{
		int ch_volume = voice->volume;
		int ch_frequency = voice->frequency;

		if (voice->enable == false)
			continue;

		mix = m_mixer_buffer.get();

		for (int i = 0; i < samples; i++)
		{
			if (voice->repeat == true)
			{
				raw_sample = m_sample_rom[((voice->start_address >> 7) & 0x7e00) | ((voice->position >> 7) & 0x1ff)];
				// guess: cut off after a number of repetitions
				if ((voice->position >> 7) & 0x20000)
				{
					voice->enable = false;
					break;
				}
			}
			else
			{
				raw_sample = m_sample_rom[((voice->start_address + voice->position) >> 7) & 0x7fff];
				if (raw_sample == 0xff)
				{
					voice->enable = false;
					break;
				}
			}
			ch_volume |= voice->volume_bank;

			*mix++ += m_volume_rom[(ch_volume << 8 | raw_sample) & 0x3fff] - 0x80;
			voice->position += ch_frequency;
		}
	}

	/* mix it down */
	mix = m_mixer_buffer.get();
	for (int i = 0; i < samples; i++)
		*buffer++ = m_mixer_lookup[*mix++];
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector flower_sound_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_io_space_config)
	};
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_MEMBER( flower_sound_device::lower_write )
{
	m_stream->update();
	m_iospace->write_byte(offset,data);
}

WRITE8_MEMBER( flower_sound_device::upper_write )
{
	m_stream->update();
	m_iospace->write_byte(offset|0x40,data);
}

WRITE8_MEMBER( flower_sound_device::frequency_w )
{
	uint8_t ch = (offset >> 3) & 0x7;
	fl_sound_channel *voice;

	voice = &m_channel_list[ch];

	voice->raw_frequency[offset & 3] = data & 0xf;

	voice->frequency = voice->raw_frequency[2] << 12;
	voice->frequency|= voice->raw_frequency[3] << 8;
	voice->frequency|= voice->raw_frequency[0] << 4;
	voice->frequency|= voice->raw_frequency[1] << 0;
}

WRITE8_MEMBER( flower_sound_device::repeat_w )
{
	uint8_t ch = (offset >> 3) & 0x7;
	fl_sound_channel *voice;

	voice = &m_channel_list[ch];
	voice->repeat = BIT(data,4);
}

WRITE8_MEMBER( flower_sound_device::unk_w )
{
	// same as above?
}

WRITE8_MEMBER( flower_sound_device::volume_w )
{
	uint8_t ch = (offset >> 3) & 0x7;
	fl_sound_channel *voice;

	voice = &m_channel_list[ch];
	voice->volume = data >> 4;
}

WRITE8_MEMBER( flower_sound_device::start_address_w )
{
	uint8_t ch = (offset >> 3) & 0x7;
	fl_sound_channel *voice;

	voice = &m_channel_list[ch];
	voice->start_nibbles[offset & 7] = data & 0xf;
	if ((offset & 7) == 4)
		voice->effect = data >> 4;
}

WRITE8_MEMBER( flower_sound_device::sample_trigger_w )
{
	uint8_t ch = (offset >> 3) & 0x7;
	fl_sound_channel *voice;

	voice = &m_channel_list[ch];

	voice->enable = true;
	voice->volume_bank = (data & 3) << 4;
	voice->start_address = 0;
	voice->position = 0;
	for (int i = 5; i >= 0; i--)
	{
		voice->start_address = (voice->start_address << 4) | voice->start_nibbles[i];
	}
}
