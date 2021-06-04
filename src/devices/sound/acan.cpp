// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Super A'Can sound driver

	Currently has a number of unknown registers and functionality.

****************************************************************************/

#include "emu.h"
#include "sound/acan.h"

#define VERBOSE		(1)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(ACANSND, acan_sound_device, "acansound", "Super A'Can Audio")

acan_sound_device::acan_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACANSND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_ram_read(*this)
	, m_active_channels(0)
{
}


void acan_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock());

	m_ram_read.resolve_safe(0);

	// register for savestates
	save_item(NAME(m_active_channels));
	save_item(STRUCT_MEMBER(m_channels, pitch));
	save_item(STRUCT_MEMBER(m_channels, length));
	save_item(STRUCT_MEMBER(m_channels, start_addr));
	save_item(STRUCT_MEMBER(m_channels, curr_addr));
	save_item(STRUCT_MEMBER(m_channels, end_addr));
	save_item(STRUCT_MEMBER(m_channels, addr_increment));
	save_item(STRUCT_MEMBER(m_channels, frac));
	save_item(STRUCT_MEMBER(m_channels, envelope));
	save_item(STRUCT_MEMBER(m_channels, volume));
	save_item(STRUCT_MEMBER(m_channels, volume_l));
	save_item(STRUCT_MEMBER(m_channels, volume_r));
	save_item(STRUCT_MEMBER(m_channels, one_shot));
	save_item(NAME(m_regs));
}

void acan_sound_device::device_reset()
{
	m_active_channels = 0;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

void acan_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	int32_t mix[clock()*2];

	std::fill_n(&mix[0], outputs[0].samples() * 2, 0);

	for (int i = 0; i < 15 && m_active_channels != 0; i++)
	{
		if (BIT(m_active_channels, i))
		{
			acan_channel &channel = m_channels[i];
			int32_t *mixp = &mix[0];

			for (int s = 0; s < outputs[0].samples(); s++)
			{
				uint8_t data = m_ram_read(channel.curr_addr) + 0x80;
				int16_t sample = (int16_t)(data << 8);

				channel.frac += channel.addr_increment;
				channel.curr_addr += (uint16_t)(channel.frac >> 16);
				channel.frac = (uint16_t)channel.frac;

				*mixp++ += (sample * channel.volume_l) >> 8;
				*mixp++ += (sample * channel.volume_r) >> 8;

				if (channel.curr_addr >= channel.end_addr)
				{
					if (channel.one_shot)
					{
						m_active_channels &= ~(1 << i);
					}
					else
					{
						channel.curr_addr -= channel.length;
					}
				}
			}
		}
	}

	int32_t *mixp = &mix[0];
	for (int i = 0; i < outputs[0].samples(); i++)
	{
		outputs[0].put_int(i, *mixp++, 32768 << 4);
		outputs[1].put_int(i, *mixp++, 32768 << 4);
	}
}

uint8_t acan_sound_device::read(offs_t offset)
{
	return m_regs[offset];
}

void acan_sound_device::write(offs_t offset, uint8_t data)
{
	const uint8_t upper = (offset >> 4) & 0x0f;
	const uint8_t lower = offset & 0x0f;

	m_regs[offset] = data;

	switch (upper)
	{
		case 0x1:
			if (lower == 0x7) // Keyon/keyoff
			{
				const uint16_t mask = 1 << (data & 0xf);
				if (data & 0xf0)
					m_active_channels |= mask;
				else
					m_active_channels &= ~mask;
			}
			else
			{
				LOG("Unknown audio register: %02x = %02x\n", offset, data);
			}
			break;

		case 0x2: // Pitch (low byte)
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.pitch &= 0xff00;
				channel.pitch |= data;
				channel.addr_increment = (uint32_t)channel.pitch << 6;
				channel.frac = 0;
			}
			break;

		case 0x3: // Pitch (high byte)
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.pitch &= 0x00ff;
				channel.pitch |= data << 8;
				channel.addr_increment = (uint32_t)channel.pitch << 6;
				channel.frac = 0;
			}
			break;

		case 0x5: // Waveform length
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.length = (data & ~0x01) << 6;
				channel.end_addr = channel.curr_addr + channel.length;
				channel.one_shot = BIT(data, 0);
			}
			break;

		case 0x6: // Waveform address (divided by 0x40, high byte)
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.start_addr &= 0x00ff;
				channel.start_addr |= data << 8;
				channel.curr_addr = channel.start_addr << 6;
				channel.end_addr = channel.curr_addr + channel.length;
			}
			break;

		case 0x7: // Waveform address (divided by 0x40, low byte)
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.start_addr &= 0xff00;
				channel.start_addr |= data;
				channel.curr_addr = channel.start_addr << 6;
				channel.end_addr = channel.curr_addr + channel.length;
			}
			break;

		case 0xa: // Envelope Parameters? (not yet known)
		case 0xb:
		case 0xc:
		case 0xd:
			if (lower < 0xf)
				m_channels[lower].envelope[upper - 0xa] = data;
			break;

		case 0xe: // Volume
			if (lower < 0xf)
			{
				acan_channel &channel = m_channels[lower];
				channel.volume = data;
				channel.volume_l = (data & 0xf0) | (data >> 4);
				channel.volume_r = (data & 0x0f) | (data << 4);
			}
			break;

		default:
			LOG("Unknown audio register: %02x = %02x\n", offset, data);
			break;
	}
}
