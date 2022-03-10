// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Hiromitsu Shioya
/*********************************************************/
/*    Konami PCM controller                              */
/*********************************************************/

/*
  Changelog, Hiromitsu Shioya 02/05/2002
    fixed start address decode timing. (sample loop bug.)

  Changelog, Mish, August 1999:
    Removed interface support for different memory regions per channel.
    Removed interface support for differing channel volume.

    Added bankswitching.
    Added support for multiple chips.

    (NB: Should different memory regions per channel be needed, the bankswitching function can set this up).

  Chanelog, Nicola, August 1999:
    Added Support for the k007232_VOL() macro.
    Added external port callback, and functions to set the volume of the channels
*/


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

	/* Set up the chips */
	m_port_write_handler.resolve_safe();

	for (int i = 0; i < KDAC_A_PCM_MAX; i++)
	{
		m_channel[i].addr = 0;
		m_channel[i].start = 0;
		m_channel[i].counter = 0x1000;
		m_channel[i].step = 0;
		m_channel[i].play = 0;
		m_channel[i].bank = 0;
	}
	m_channel[0].vol[0] = 255;  /* channel A output to output A */
	m_channel[0].vol[1] = 0;
	m_channel[1].vol[0] = 0;
	m_channel[1].vol[1] = 255;  /* channel B output to output B */

	for (auto & elem : m_wreg)
		elem = 0;

	m_stream = stream_alloc(0, 2, clock()/128);

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
	m_stream->set_sample_rate(clock()/128);
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
	else
	{
		channel_t *channel = &m_channel[(offset >= 6 ? 1 : 0)];
		int reg_index = (offset >= 6 ? 6 : 0);
		if (offset >= 6)
			offset -= 6;

		switch (offset)
		{
		case 0: // address step, LSB
		case 1: // address step, MSB
			channel->step = (BIT(m_wreg[reg_index + 1], 0, 4) << 8) | m_wreg[reg_index];
			// TODO: Bit 4-5 is frequency divider, but not implemented now
			break;
		case 2:
		case 3:
		case 4:
			// working data for start address
			channel->start = (BIT(m_wreg[reg_index + 4], 0) << 16) | (m_wreg[reg_index + 3] << 8) | m_wreg[reg_index + 2];
			break;
		case 5: // start address
			if (channel->start < m_pcmlimit)
			{
				channel->play = true;
				channel->addr = channel->start;
				channel->counter = 0x1000;
			}
			break;
		}
	}
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/

u8 k007232_device::read(offs_t offset)
{
	if (offset == 5 || offset == 11)
	{
		channel_t *channel = &m_channel[(offset == 11) ? 1 : 0];

		if (channel->start < m_pcmlimit)
		{
			channel->play = true;
			channel->addr = channel->start;
			channel->counter = 0x1000;
		}
	}
	return 0;
}

/*****************************************************************************/

void k007232_device::set_volume(int channel, int vol_a, int vol_b)
{
	m_channel[channel].vol[0] = vol_a;
	m_channel[channel].vol[1] = vol_b;
}

void k007232_device::set_bank(int chan_a_bank, int chan_b_bank)
{
	m_channel[0].bank = chan_a_bank << 17;
	m_channel[1].bank = chan_b_bank << 17;
}

/*****************************************************************************/


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k007232_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	if (K007232_LOG_PCM)
	{
		for (int i = 0; i < KDAC_A_PCM_MAX; i++)
		{
			channel_t *channel = &m_channel[i];
			if (channel->play)
			{
				char filebuf[256];
				snprintf(filebuf, 256, "pcm%08x.wav", channel->start);
				util::wav_file_ptr file = util::wav_open(filebuf, stream.sample_rate(), 1);
				if (file)
				{
					u32 addr = channel->start;
					while (!BIT(read_sample(i, addr), 7) && addr < m_pcmlimit)
					{
						int16_t out = ((read_sample(i, addr) & 0x7f) - 0x40) << 7;
						util::wav_add_data_16(*file, &out, 1);
						addr++;
					}
				}
			}
		}
	}

	for (int j = 0; j < outputs[0].samples(); j++)
	{
		s32 lsum = 0, rsum = 0;
		for (int i = 0; i < KDAC_A_PCM_MAX; i++)
		{
			channel_t *channel = &m_channel[i];
			if (channel->play)
			{
				/**** PCM setup ****/
				int vol_a = channel->vol[0] * 2;
				int vol_b = channel->vol[1] * 2;

				u32 addr = channel->addr & 0x1ffff;
				while (channel->counter <= channel->step) // result : clock / (4 * (4096 - frequency))
				{
					if (BIT(read_sample(i, addr++), 7) || addr >= m_pcmlimit)
					{
						// end of sample
						if (BIT(m_wreg[13], i))
						{
							/* loop to the beginning */
							addr = channel->start;
						}
						else
						{
							/* stop sample */
							channel->play = false;
							break;
						}
					}
					channel->counter += (0x1000 - channel->step);
				}
				channel->addr = addr;

				if (!channel->play)
					break;

				channel->counter -= 32;

				int out = (read_sample(i, addr) & 0x7f) - 0x40;

				lsum += out * vol_a;
				rsum += out * vol_b;
			}
		}
		outputs[0].put_int(j, lsum, 32768);
		outputs[1].put_int(j, rsum, 32768);
	}
}
