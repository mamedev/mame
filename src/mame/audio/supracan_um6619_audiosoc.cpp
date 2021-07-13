// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, superctr
/***************************************************************************

    Super A'Can sound driver (UM6619)

    The UM6619 integrates:

    implemented here:

    Custom audio hardware

    implemented in supracan_um6618_cpu.cpp:

    6502 CPU
    DMA Controller
    Pad Interface for 2 controllers


    Currently has a number of unknown registers and functionality.

    --------------

    There are 6 interrupt sources on the 6502 side, all of which use the IRQ line.
    The register at 0x411 is bitmapped to indicate what source(s) are active.
    In priority order from most to least important, they are:

    411 value  How acked                     Notes
    0x40       read reg 0x16 of sound chip   used for DMA-driven sample playback. Register 0x16 may contain which DMA-driven samples are active.
    0x04       read at 0x405                 latch 1?  0xcd is magic value
    0x08       read at 0x404                 latch 2?  0xcd is magic value
    0x10       read at 0x409                 unknown, dispatched but not used in startup 6502 code
    0x20       read at 0x40a                 IRQ request from 68k, flags data available in shared-RAM mailbox
    0x80       read reg 0x14 of sound chip   depends on reg 0x14 of sound chip & 0x40: if not set writes 0x8f to reg 0x14,
                                             otherwise writes 0x4f to reg 0x14 and performs additional processing


****************************************************************************/

#include "emu.h"

#include "supracan_um6619_audiosoc.h"

#define VERBOSE     (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc, "um6619_soc", "UM6619 Audio System on a Chip")

supracan_um6619_audiosoc::supracan_um6619_audiosoc(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: supracan_um6619_cpu_device(mconfig, SUPRACAN_UM6619_AUDIOSOC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_timer(nullptr)
	, m_active_channels(0)
	, m_dma_channels(0)
{
}


void supracan_um6619_audiosoc::device_start()
{
	supracan_um6619_cpu_device::device_start();

	m_stream = stream_alloc(0, 2, clock() / 16 / 5);
	m_mix = std::make_unique<int32_t[]>((clock() / 16 / 5) * 2);
	m_timer = timer_alloc(0);

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
	save_item(NAME(m_regs));
}

void supracan_um6619_audiosoc::device_reset()
{
	supracan_um6619_cpu_device::device_reset();

	m_active_channels = 0;
	m_dma_channels = 0;
	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	m_timer->reset();
	set_sound_irq(7, 0); // Timer IRQ
	set_sound_irq(6, 0); // DMA IRQ
}

void supracan_um6619_audiosoc::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	supracan_um6619_cpu_device::device_timer(timer, id, param, ptr);

	if (m_regs[0x14] & 0x40)
	{
		set_sound_irq(7, 1); // timer irq

		// Update frequency
		uint16_t period = (m_regs[0x12] << 8) + m_regs[0x11];
		m_timer->adjust(clocks_to_attotime(10 * (0x10000 - period)), 0);
	}
}

void supracan_um6619_audiosoc::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	std::fill_n(&m_mix[0], outputs[0].samples() * 2, 0);

	for (int i = 0; i < 16 && m_active_channels != 0; i++)
	{
		if (BIT(m_active_channels, i))
		{
			acan_channel &channel = m_channels[i];
			int32_t *mixp = &m_mix[0];

			for (int s = 0; s < outputs[0].samples(); s++)
			{
				uint8_t data = m_soundram[channel.curr_addr] + 0x80;
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
						set_sound_irq(6, 1); // dma IRQ
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
	for (int i = 0; i < outputs[0].samples(); i++)
	{
		outputs[0].put_int(i, *mixp++, 32768 << 4);
		outputs[1].put_int(i, *mixp++, 32768 << 4);
	}
}

uint8_t supracan_um6619_audiosoc::sound_read(offs_t offset)
{
	m_stream->update();

	if (offset == 0x14)
	{
		// acknowledge timer IRQ?
		set_sound_irq(7, 0);
	}
	else if (offset == 0x16)
	{
		// acknowledge DMA IRQ?
		set_sound_irq(6, 0);
	}
	return m_regs[offset];
}

void supracan_um6619_audiosoc::keyon_voice(uint8_t voice)
{
	acan_channel &channel = m_channels[voice];
	channel.curr_addr = channel.start_addr << 6;
	channel.end_addr = channel.curr_addr + channel.length;

	m_active_channels |= (1 << voice);

	//printf("Keyon voice %d\n", voice);
}

void supracan_um6619_audiosoc::sound_write(offs_t offset, uint8_t data)
{
	m_stream->update();

	const uint8_t upper = (offset >> 4) & 0x0f;
	const uint8_t lower = offset & 0x0f;

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

	default:
		LOG("Unknown sound register: %02x = %02x\n", offset, data);
		break;
	}
}
