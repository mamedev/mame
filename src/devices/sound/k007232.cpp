// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Hiromitsu Shioya
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

#include "emu.h"
#include "k007232.h"
#include "wavwrite.h"
#include <algorithm>

#define K007232_LOG_PCM (0)

DEFINE_DEVICE_TYPE(K007232, k007232_device, "k007232", "K007232 PCM Controller")

k007232_device::k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K007232, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 17) // 17 bit address, 8 bit data, can be bankswitched per each voices
	, m_rom(*this, DEVICE_SELF)
	, m_pcmlimit(~0)
	, m_bank(0)
	, m_stream(nullptr)
	, m_port_write_handler(*this)
{
	std::fill(std::begin(m_wreg), std::end(m_wreg), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007232_device::device_start()
{
	// assumes it can make an address mask with m_rom.length() - 1
	assert (!m_rom.found() || !(m_rom.length() & (m_rom.length() - 1)));

	m_pcmlimit = 1 << 17;
	// default mapping (bankswitched ROM)
	if (m_rom.found() && !has_configured_map(0))
	{
		if (m_rom.bytes() > 0x20000)
			space(0).install_read_handler(0x00000, std::min<offs_t>(0x1ffff, m_rom.bytes() - 1), read8sm_delegate(*this, FUNC(k007232_device::read_rom_default)));
		else
		{
			space(0).install_rom(0x00000, m_rom.length() - 1, m_rom.target());
			m_pcmlimit = m_rom.bytes();
		}
	}
	space(0).cache(m_cache);

	// set up the chips
	for (int i = 0; i < 2; i++)
	{
		m_channel[i].addr = 0;
		m_channel[i].start = 0;
		m_channel[i].counter = 0x1000;
		m_channel[i].step = 0;
		m_channel[i].play = 0;
		m_channel[i].bank = 0;
	}
	m_channel[0].vol[0] = 255;  // channel A output to output A
	m_channel[0].vol[1] = 0;
	m_channel[1].vol[0] = 0;
	m_channel[1].vol[1] = 255;  // channel B output to output B

	for (auto & elem : m_wreg)
		elem = 0;

	m_stream = stream_alloc(0, 2, clock() / 128);

	save_item(STRUCT_MEMBER(m_channel, vol));
	save_item(STRUCT_MEMBER(m_channel, addr));
	save_item(STRUCT_MEMBER(m_channel, counter));
	save_item(STRUCT_MEMBER(m_channel, start));
	save_item(STRUCT_MEMBER(m_channel, step));
	save_item(STRUCT_MEMBER(m_channel, bank));
	save_item(STRUCT_MEMBER(m_channel, play));
	save_item(NAME(m_wreg));
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void k007232_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 128);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector k007232_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_data_config) };
}

/************************************************/
/*    Konami PCM write register                 */
/************************************************/

void k007232_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	m_wreg[offset] = data; // standard data write

	if (offset == 12)
	{
		// external port, usually volume control
		m_port_write_handler(0, data);
	}
	else if (offset == 13)
	{
		// loop flag, handled by standard data write
	}
	else if (offset < 12)
	{
		channel_t &channel = m_channel[(offset >= 6 ? 1 : 0)];
		int const reg_index = (offset >= 6 ? 6 : 0);

		switch (offset - reg_index)
		{
		case 0: // address step, LSB
		case 1: // address step, MSB
			channel.step = (BIT(m_wreg[reg_index + 1], 0, 4) << 8) | m_wreg[reg_index];
			// TODO: Bit 4-5 is frequency divider, but not implemented now
			break;
		case 2:
		case 3:
		case 4:
			// working data for start address
			channel.start = (BIT(m_wreg[reg_index + 4], 0) << 16) | (m_wreg[reg_index + 3] << 8) | m_wreg[reg_index + 2];
			break;
		case 5: // start address
			start(reg_index ? 1 : 0);
			break;
		}
	}
}

void k007232_device::start(int ch)
{
	channel_t &channel = m_channel[ch];

	if (channel.start < m_pcmlimit)
	{
		channel.play = true;
		channel.addr = channel.start;
		channel.counter = 0x1000;

		if (K007232_LOG_PCM)
		{
			char filebuf[256];
			snprintf(filebuf, 256, "pcm%08x.wav", channel.start);
			util::wav_file_ptr file = util::wav_open(filebuf, m_stream->sample_rate(), 1);
			if (file)
			{
				u32 addr = channel.start;
				while (addr < m_pcmlimit && !BIT(read_sample(ch, addr), 7))
				{
					s16 out = ((read_sample(ch, addr) & 0x7f) - 0x40) << 7;
					util::wav_add_data_16(*file, &out, 1);
					addr++;
				}
			}
		}
	}
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/

u8 k007232_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 5 || offset == 11)
		{
			m_stream->update();
			start((offset == 11) ? 1 : 0);
		}
	}
	return 0;
}

/*****************************************************************************/

void k007232_device::set_volume(int channel, int vol_a, int vol_b)
{
	m_stream->update();
	m_channel[channel].vol[0] = vol_a;
	m_channel[channel].vol[1] = vol_b;
}

void k007232_device::set_bank(int chan_a_bank, int chan_b_bank)
{
	m_stream->update();
	m_channel[0].bank = chan_a_bank << 17;
	m_channel[1].bank = chan_b_bank << 17;
}

/*****************************************************************************/


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k007232_device::sound_stream_update(sound_stream &stream)
{
	for (int j = 0; j < stream.samples(); j++)
	{
		s32 lsum = 0, rsum = 0;
		for (int i = 0; i < 2; i++)
		{
			channel_t &channel = m_channel[i];
			if (channel.play)
			{
				// PCM setup
				int const vol_a = channel.vol[0] * 2;
				int const vol_b = channel.vol[1] * 2;

				u32 addr = channel.addr & 0x1ffff;
				while (channel.counter <= channel.step) // result : clock / (4 * (4096 - frequency))
				{
					if (addr >= m_pcmlimit || BIT(read_sample(i, addr++), 7))
					{
						// end of sample
						if (BIT(m_wreg[13], i))
						{
							// loop to the beginning
							addr = channel.start;
						}
						else
						{
							// stop sample
							channel.play = false;
							break;
						}
					}
					channel.counter += (0x1000 - channel.step);
				}
				channel.addr = addr;

				if (!channel.play)
					break;

				channel.counter -= 32;

				int const out = (read_sample(i, addr) & 0x7f) - 0x40;

				lsum += out * vol_a;
				rsum += out * vol_b;
			}
		}
		stream.put_int(0, j, lsum, 32768);
		stream.put_int(1, j, rsum, 32768);
	}
}
