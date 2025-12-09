// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Flower custom sound chip

    Similar to Wiping and Namco 15xx designs

    TODO:
    - several unknown registers (effects and unknown register tied to repeat port);
    - repeat certainly needs a cutoff, which is unknown about how it works;
    - keyon/off behavior is not verified
    - PCM output is incorrect/unverified

***************************************************************************/

#include "emu.h"
#include "flower_a.h"


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

flower_sound_device::flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FLOWER_CUSTOM, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_memory_interface(mconfig, *this),
	  m_io_space_config("io", ENDIANNESS_LITTLE, 8, 7, 0, address_map_constructor(FUNC(flower_sound_device::regs_map), this)),
	  m_stream(nullptr),
	  m_mixer_lookup(nullptr),
	  m_sample_rom(*this, "samples"),
	  m_volume_rom(*this, "soundvol")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void flower_sound_device::device_start()
{
	m_iospace = &space(AS_IO);
	m_stream = stream_alloc(0, 1, clock()/2);

	m_mixer_buffer.resize(clock()/2);
	make_mixer_table(MAX_VOICES, defgain);

	save_item(STRUCT_MEMBER(m_channel_list, start_nibbles));
	save_item(STRUCT_MEMBER(m_channel_list, raw_frequency));
	save_item(STRUCT_MEMBER(m_channel_list, start_address));
	save_item(STRUCT_MEMBER(m_channel_list, position));
	save_item(STRUCT_MEMBER(m_channel_list, frequency));
	save_item(STRUCT_MEMBER(m_channel_list, volume));
	save_item(STRUCT_MEMBER(m_channel_list, volume_bank));
	//save_item(STRUCT_MEMBER(m_channel_list, effect));
	save_item(STRUCT_MEMBER(m_channel_list, enable));
	save_item(STRUCT_MEMBER(m_channel_list, repeat));

	save_item(NAME(m_io_regs));
}


/* build a table to divide by the number of voices; gain is specified as gain*16 */
void flower_sound_device::make_mixer_table(int voices, int gain)
{
	/* allocate memory */
	m_mixer_table.resize(256 * voices);

	/* find the middle of the table */
	m_mixer_lookup = &m_mixer_table[128 * voices];

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
	for (auto &voice : m_channel_list)
	{
		voice.start_address = 0;
		voice.position = 0;
		voice.volume = 0;
		voice.enable = false;
		voice.repeat = false;
	}
}

void flower_sound_device::sound_stream_update(sound_stream &stream)
{
	short *mix;
	u8 raw_sample;

	std::fill_n(&m_mixer_buffer[0], stream.samples(), 0);

	for (auto &voice : m_channel_list)
	{
		int ch_volume = voice.volume;
		int ch_frequency = voice.frequency;

		if (!voice.enable)
			continue;

		mix = &m_mixer_buffer[0];

		for (int i = 0; i < stream.samples(); i++)
		{
			// Volume LUT ROM address bit:
			// Bit 0-7: Sample ROM data
			// Bit 8-11: Channel Volume
			// Bit 12: Sample Position bit 6 (Sample nibble select bit for 4 bit data)
			// Bit 13: Volume LUT bank select? (4 bit/8 bit data mode or wavetable/PCM?)
			const u16 volume_index = (((ch_volume | voice.volume_bank) & 0x2f) << 8) | (BIT(voice.position, 6) << 12);
			if (voice.repeat)
			{
				raw_sample = m_sample_rom[((voice.start_address >> 7) & 0x7e00) | ((voice.position >> 7) & 0x1ff)];
				// guess: key on/off bit is lowest bit of volume bank register?
				if ((voice.volume_bank & 0x10) != 0x10)
				{
					voice.enable = false;
					break;
				}
			}
			else
			{
				raw_sample = m_sample_rom[((voice.start_address + voice.position) >> 7) & 0x7fff];
				if (raw_sample == 0xff)
				{
					voice.enable = false;
					break;
				}
			}
			*mix++ += m_volume_rom[(volume_index | raw_sample) & 0x3fff] - 0x80;
			voice.position += ch_frequency;
		}
	}

	/* mix it down */
	mix = &m_mixer_buffer[0];
	for (int i = 0; i < stream.samples(); i++)
		stream.put_int(0, i, m_mixer_lookup[*mix++], 32768);
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

void flower_sound_device::lower_write(offs_t offset, u8 data)
{
	m_stream->update();
	m_io_regs[offset] = data;
	m_iospace->write_byte(offset,data);
}

void flower_sound_device::upper_write(offs_t offset, u8 data)
{
	m_stream->update();
	m_io_regs[offset|0x40] = data;
	m_iospace->write_byte(offset|0x40,data);
}

void flower_sound_device::frequency_w(offs_t offset, u8 data)
{
	u8 ch = (offset >> 3) & 0x7;
	fl_sound_channel &voice = m_channel_list[ch];

	// Low nibbles: part of frequency
	// High nibbles: unknown
	voice.raw_frequency[offset & 3] = data;

	voice.frequency = (voice.raw_frequency[2] & 0xf) << 12;
	voice.frequency|= (voice.raw_frequency[3] & 0xf) << 8;
	voice.frequency|= (voice.raw_frequency[0] & 0xf) << 4;
	voice.frequency|= (voice.raw_frequency[1] & 0xf) << 0;
}

void flower_sound_device::repeat_w(offs_t offset, u8 data)
{
	u8 ch = (offset >> 3) & 0x7;
	fl_sound_channel &voice = m_channel_list[ch];

	voice.repeat = BIT(data, 4); // Bit 4: Repeat flag?
}

void flower_sound_device::unk_w(offs_t offset, u8 data)
{
	// same as above?
}

void flower_sound_device::volume_w(offs_t offset, u8 data)
{
	u8 ch = (offset >> 3) & 0x7;
	fl_sound_channel &voice = m_channel_list[ch];

	// Low nibbles: unknown
	// High nibbles: volume
	voice.volume = data >> 4;
}

void flower_sound_device::start_address_w(offs_t offset, u8 data)
{
	u8 ch = (offset >> 3) & 0x7;
	fl_sound_channel &voice = m_channel_list[ch];

	// Low nibbles: part of start address
	// High nibbles: unknown
	voice.start_nibbles[offset & 7] = data;
	/*
	if ((offset & 7) == 4)
	    voice.effect = data >> 4;
	*/
}

void flower_sound_device::sample_trigger_w(offs_t offset, u8 data)
{
	u8 ch = (offset >> 3) & 0x7;
	fl_sound_channel &voice = m_channel_list[ch];

	voice.enable = true; // BIT(data, 0);
	voice.volume_bank = (data & 3) << 4; // Bit 0: Keyon/off?, Bit 1: PCM/Wavetable mode select?
	voice.start_address = 0;
	voice.position = 0;
	for (int i = 5; i >= 0; i--)
	{
		voice.start_address = (voice.start_address << 4) | (voice.start_nibbles[i] & 0xf);
	}
}
