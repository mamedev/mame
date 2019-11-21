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

#define	K007232_LOG_PCM	(0)
#define	BASE_SHIFT    	(12)

DEFINE_DEVICE_TYPE(K007232, k007232_device, "k007232", "K007232 PCM Controller")

k007232_device::k007232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K007232, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_rom(*this, DEVICE_SELF)
	, m_port_write_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007232_device::device_start()
{
	/* Set up the chips */
	m_pcmlimit = m_rom.bytes();

	m_port_write_handler.resolve_safe();

	for (int i = 0; i < KDAC_A_PCM_MAX; i++)
	{
		m_addr[i] = 0;
		m_start[i] = 0;
		m_step[i] = 0;
		m_play[i] = 0;
		m_bank[i] = 0;
	}
	m_vol[0][0] = 255;  /* channel A output to output A */
	m_vol[0][1] = 0;
	m_vol[1][0] = 0;
	m_vol[1][1] = 255;  /* channel B output to output B */

	for (auto & elem : m_wreg)
		elem = 0;

	m_stream = machine().sound().stream_alloc(*this, 0 , 2, clock()/128);

	make_fncodes();

	save_item(NAME(m_vol));
	save_item(NAME(m_addr));
	save_item(NAME(m_start));
	save_item(NAME(m_step));
	save_item(NAME(m_bank));
	save_item(NAME(m_play));
	save_item(NAME(m_wreg));
}

void k007232_device::make_fncodes()
{
	for (int i = 0; i < 0x200; i++)
	{
		m_fncode[i] = (32 << BASE_SHIFT) / (0x200 - i);
	}
}

uint32_t k007232_device::get_start_address(int channel)
{
	const uint32_t reg_index = (channel ? 6 : 0);
	return (BIT(m_wreg[reg_index + 4], 0) << 16) | (m_wreg[reg_index + 3] << 8) | m_wreg[reg_index + 2] | m_bank[channel];
}

/************************************************/
/*    Konami PCM write register                 */
/************************************************/

WRITE8_MEMBER(k007232_device::write)
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
		int channel = (offset >= 6 ? 1 : 0);
		int reg_index = (offset >= 6 ? 6 : 0);
		if (offset >= 6)
			offset -= 6;

		switch (offset)
		{
		case 0: // address step, LSB
		case 1: // address step, MSB
			m_step[channel] = m_fncode[(BIT(m_wreg[reg_index + 1], 0) << 8) | m_wreg[reg_index]];
			break;
		case 2:
		case 3:
		case 4:
			// working data for start address
			break;
		case 5: // start address
			m_start[channel] = get_start_address(channel);
			if (m_start[channel] < m_pcmlimit)
			{
				m_play[channel] = 1;
				m_addr[channel] = 0;
			}
			break;
		}
	}
}

/************************************************/
/*    Konami PCM read register                  */
/************************************************/

READ8_MEMBER(k007232_device::read)
{
	if (offset == 5 || offset == 11)
	{
		int channel = (offset == 11) ? 1 : 0;

		m_start[channel] = get_start_address(channel);

		if (m_start[channel] <  m_pcmlimit)
		{
			m_play[channel] = 1;
			m_addr[channel] = 0;
		}
	}
	return 0;
}

/*****************************************************************************/

void k007232_device::set_volume(int channel, int vol_a, int vol_b)
{
	m_vol[channel][0] = vol_a;
	m_vol[channel][1] = vol_b;
}

void k007232_device::set_bank(int chan_a_bank, int chan_b_bank)
{
	m_bank[0] = chan_a_bank << 17;
	m_bank[1] = chan_b_bank << 17;
}

/*****************************************************************************/


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k007232_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	memset(outputs[0], 0, samples * sizeof(stream_sample_t));
	memset(outputs[1], 0, samples * sizeof(stream_sample_t));

	if (K007232_LOG_PCM)
	{
		for (int i = 0; i < KDAC_A_PCM_MAX; i++)
		{
			if (m_play[i])
			{
				char filebuf[256];
				snprintf(filebuf, 256, "pcm%08x.wav", m_start[i]);
				wav_file *file = wav_open(filebuf, stream.sample_rate(), 1);
				if (file != nullptr)
				{
					uint32_t addr = m_start[i];
					while (!BIT(m_rom[addr], 7) && addr < m_pcmlimit)
					{
						int16_t out = ((m_rom[addr] & 0x7f) - 0x40) << 7;
						wav_add_data_16(file, &out, 1);
						addr++;
					}
					wav_close(file);
				}
			}
		}
	}

	for (int i = 0; i < KDAC_A_PCM_MAX; i++)
	{
		if (m_play[i])
		{
			/**** PCM setup ****/
			uint32_t addr_lsb = (m_addr[i] >> BASE_SHIFT) & 0x000fffff;
			uint32_t addr = m_start[i] + addr_lsb;
			int vol_a = m_vol[i][0] * 2;
			int vol_b = m_vol[i][1] * 2;

			for (int j = 0; j < samples; j++)
			{
				uint32_t old_addr = addr;
				addr_lsb = (m_addr[i] >> BASE_SHIFT) & 0x000fffff;
				addr = m_start[i] + addr_lsb;
				while (old_addr <= addr)
				{
					if (BIT(m_rom[old_addr], 7) || old_addr >= m_pcmlimit)
					{
						// end of sample
						if (BIT(m_wreg[13], i))
						{
							/* loop to the beginning */
							m_start[i] = get_start_address(i);
							addr = m_start[i];
							m_addr[i] = 0;
							old_addr = addr; /* skip loop */
						}
						else
						{
							/* stop sample */
							m_play[i] = 0;
						}
						break;
					}
					old_addr++;
				}

				if (m_play[i] == 0)
					break;

				m_addr[i] += m_step[i];

				int out = (m_rom[addr] & 0x7f) - 0x40;

				outputs[0][j] += out * vol_a;
				outputs[1][j] += out * vol_b;
			}
		}
	}
}
