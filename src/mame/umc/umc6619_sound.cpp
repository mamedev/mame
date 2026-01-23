// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, superctr
/***************************************************************************

    Super A'Can UMC 6619 sound driver

    Currently has a number of unknown registers and functionality.

****************************************************************************/

#include "emu.h"
#include "umc6619_sound.h"

#define VERBOSE     (0)
#include "logmacro.h"

#define LIVE_AUDIO_VIEW 0

// device type definition
DEFINE_DEVICE_TYPE(UMC6619_SOUND, umc6619_sound_device, "umc6619_sound", "UMC UM6619 Sound Engine")

umc6619_sound_device::umc6619_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UMC6619_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer(nullptr)
	, m_timer_irq_handler(*this)
	, m_dma_irq_handler(*this)
	, m_ram_read(*this, 0)
	, m_active_channels(0)
	, m_dma_channels(0)
{
}


void umc6619_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 16 / 5);
	m_mix = std::make_unique<int32_t[]>((clock() / 16 / 5) * 2);
	m_timer = timer_alloc(FUNC(umc6619_sound_device::channel_irq), this);

	// register for savestates
	save_item(NAME(m_active_channels));
	save_item(NAME(m_dma_channels));
	save_item(STRUCT_MEMBER(m_channels, pitch));
	save_item(STRUCT_MEMBER(m_channels, length));
	save_item(STRUCT_MEMBER(m_channels, start_addr));
	save_item(STRUCT_MEMBER(m_channels, curr_addr));
	save_item(STRUCT_MEMBER(m_channels, end_addr));
	save_item(STRUCT_MEMBER(m_channels, addr_increment));
	save_item(STRUCT_MEMBER(m_channels, frac));
	save_item(STRUCT_MEMBER(m_channels, register9));
	save_item(STRUCT_MEMBER(m_channels, envelope));
	save_item(STRUCT_MEMBER(m_channels, volume));
	save_item(STRUCT_MEMBER(m_channels, volume_l));
	save_item(STRUCT_MEMBER(m_channels, volume_r));
	save_item(STRUCT_MEMBER(m_channels, one_shot));
	save_item(STRUCT_MEMBER(m_channels, unk_upper_05));
	save_item(NAME(m_regs));
}

void umc6619_sound_device::device_reset()
{
	m_active_channels = 0;
	m_dma_channels = 0;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	for (auto &channel : m_channels)
	{
		channel.pitch = 0;
		channel.length = 0;
		channel.start_addr = 0;
		channel.curr_addr = 0;
		channel.end_addr = 0;
		channel.addr_increment = 0;
		channel.frac = 0;
		channel.register9 = 0;
		std::fill(std::begin(channel.envelope), std::end(channel.envelope), 0);
		channel.volume = 0;
		channel.volume_l = 0;
		channel.volume_r = 0;
		channel.one_shot = false;
		channel.unk_upper_05 = 0;
	}

	m_timer->reset();
	m_timer_irq_handler(0);
	m_dma_irq_handler(0);
}

TIMER_CALLBACK_MEMBER(umc6619_sound_device::channel_irq)
{
	if (m_regs[0x14] & 0x40)
	{
		m_timer_irq_handler(1);

		// Update frequency
		uint16_t period = (m_regs[0x12] << 8) + m_regs[0x11];
		m_timer->adjust(clocks_to_attotime(10 * (0x10000 - period)), 0);
	}
}

std::string umc6619_sound_device::print_audio_state()
{
	std::ostringstream outbuffer;

	util::stream_format(outbuffer, "channel | address               | length | pitch | one?   |  vol | DMA? | (unk09)    |\n");

	for (int i = 0; i < 16; i++)
	{
		acan_channel &channel = m_channels[i];

		util::stream_format(outbuffer, "%02d: %01d | %04x (%04x-%04x) | %04x | %04x | %d (%02x)| %02x | %02x | %02x %02x %02x %02x|\n"
			, i
			, BIT(m_active_channels, i)
			, channel.curr_addr
			, (channel.start_addr << 6) & 0xffff
			, channel.end_addr
			, channel.length
			, channel.pitch
			, channel.one_shot
			, channel.unk_upper_05
			, channel.volume
			, channel.register9
			, channel.envelope[0]
			, channel.envelope[1]
			, channel.envelope[2]
			, channel.envelope[3]
		);
	}

	return outbuffer.str();
}

void umc6619_sound_device::sound_stream_update(sound_stream &stream)
{
	std::fill_n(&m_mix[0], stream.samples() * 2, 0);

	if (LIVE_AUDIO_VIEW)
		popmessage(print_audio_state());

	for (int i = 0; i < 16 && m_active_channels != 0; i++)
	{
		if (BIT(m_active_channels, i))
		{
			acan_channel &channel = m_channels[i];
			int32_t *mixp = &m_mix[0];

			for (int s = 0; s < stream.samples(); s++)
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
					if (channel.register9)
					{
						m_dma_irq_handler(1);
						keyon_voice(i);
					}
					else if (channel.one_shot)
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

	int32_t *mixp = &m_mix[0];
	for (int i = 0; i < stream.samples(); i++)
	{
		stream.put_int(0, i, *mixp++, 32768 << 4);
		stream.put_int(1, i, *mixp++, 32768 << 4);
	}
}

uint8_t umc6619_sound_device::read(offs_t offset)
{
	if (offset == 0x14)
	{
		// acknowledge timer IRQ?
		m_timer_irq_handler(0);
	}
	else if (offset == 0x16)
	{
		// acknowledge DMA IRQ?
		m_dma_irq_handler(0);
	}
	// TODO: offset 0x15 (read by streaming DMAs, cfr. staiwbbl)
	return m_regs[offset];
}

void umc6619_sound_device::keyon_voice(uint8_t voice)
{
	acan_channel &channel = m_channels[voice];
	channel.curr_addr = channel.start_addr << 6;
	channel.end_addr = channel.curr_addr + channel.length;

	m_active_channels |= (1 << voice);

	//printf("Keyon voice %d\n", voice);
}

void umc6619_sound_device::write(offs_t offset, uint8_t data)
{
	const uint8_t upper = (offset >> 4) & 0x0f;
	const uint8_t lower = offset & 0x0f;

	m_stream->update();
	m_regs[offset] = data;

	switch (upper)
	{
	case 0x1:
		switch (lower)
		{
		case 0x1: // Timer frequency (low byte)
			LOG("%s: Sound timer frequency (low byte) = %02x\n", machine().describe_context(), data);
			break;

		case 0x2: // Timer frequency (high byte)
			LOG("%s: Sound timer frequency (high byte) = %02x\n", machine().describe_context(), data);
			break;

		case 0x4: // Timer control
			// The meaning of the data that is actually written is unknown
			LOG("%s: Sound timer control = %02x\n", machine().describe_context(), data);
			if (BIT(data, 7))
			{
				// Update frequency
				uint16_t period = (m_regs[0x12] << 8) + m_regs[0x11];
				m_timer->adjust(clocks_to_attotime(10 * (0x10000 - period)), 0);
			}
			break;

		case 0x6: // DMA-driven channel flags?
			// The meaning of the data that is actually written is unknown
			m_dma_channels = data << 8;
			LOG("%s: DMA-driven channel flag(?) = %02x\n", machine().describe_context(), data);
			break;

		case 0x7: // Keyon/keyoff
		{
			LOG("%s: Sound key control, voice %02x key%s\n", machine().describe_context(), data & 0xf, (data & 0xf0) ? "on" : "off");
			const uint16_t mask = 1 << (data & 0xf);
			if (data & 0xf0)
			{
				keyon_voice(data & 0xf);
			}
			else
			{
				m_active_channels &= ~mask;
			}
			break;
		}

		default:
			LOG("Unknown sound register: %02x = %02x\n", offset, data);
			break;
		}
		break;

	case 0x2: // Pitch (low byte)
	{
		acan_channel &channel = m_channels[lower];
		channel.pitch &= 0xff00;
		channel.pitch |= data;
		channel.addr_increment = (uint32_t)channel.pitch << 6;
		break;
	}

	case 0x3: // Pitch (high byte)
	{
		acan_channel &channel = m_channels[lower];
		channel.pitch &= 0x00ff;
		channel.pitch |= data << 8;
		channel.addr_increment = (uint32_t)channel.pitch << 6;
		break;
	}

	case 0x5: // Waveform length
	{
		acan_channel &channel = m_channels[lower];
		channel.length = 0x40 << ((data & 0x0e) >> 1);
		channel.one_shot = BIT(data, 0);
		channel.unk_upper_05 = data & 0xf0;
		LOG("%s: Waveform length and attributes (voice %02x): %02x\n", machine().describe_context(), lower, data);
		break;
	}

	case 0x6: // Waveform address (divided by 0x40, high byte)
	{
		acan_channel &channel = m_channels[lower];
		channel.start_addr &= 0x00ff;
		channel.start_addr |= data << 8;
		LOG("%s: Waveform address (high) (voice %02x): %02x, will be %04x\n", machine().describe_context(), lower, data, channel.start_addr << 6);
		break;
	}

	case 0x7: // Waveform address (divided by 0x40, low byte)
	{
		acan_channel &channel = m_channels[lower];
		channel.start_addr &= 0xff00;
		channel.start_addr |= data;
		LOG("%s: Waveform address (low) (voice %02x): %02x, will be %04x\n", machine().describe_context(), lower, data, channel.start_addr << 6);
		break;
	}

	case 0x9: // Unknown (set to 0xFF for DMA-driven channels)
	{
		acan_channel &channel = m_channels[lower];
		channel.register9 = data;
		LOG("%s: Unknown voice register 9 (voice %02x): %02x\n", machine().describe_context(), lower, data);
		break;
	}

	case 0xa: // Envelope Parameters? (not yet known)
	case 0xb:
	case 0xc:
	case 0xd:
		m_channels[lower].envelope[upper - 0xa] = data;
		LOG("%s: Envelope parameter %d (voice %02x) = %02x\n", machine().describe_context(), upper - 0xa, lower, data);
		break;

	case 0xe: // Volume
	{
		acan_channel &channel = m_channels[lower];
		channel.volume = data;
		channel.volume_l = (data & 0xf0) | (data >> 4);
		channel.volume_r = (data & 0x0f) | (data << 4);
		LOG("%s: Volume register (voice %02x) = = %02x\n", machine().describe_context(), lower, data);
		break;
	}

	// case 4:
	// Normally 0x03 for keyon channels, 0x01 for streaming DMAs
	// (staiwbbl, formduel, sangofgt)

	default:
		LOG("Unknown sound register: %02x = %02x\n", offset, data);
		break;
	}
}
