// license:GPL-2.0+
// copyright-holders:Peter Trauner
/***************************************************************************
 supervision sound hardware

 PeT mess@utanet.at
***************************************************************************/

#include "emu.h"
#include "svis_snd.h"

#include <algorithm>

// configurable logging
#define LOG_DMA     (1U << 1)
#define LOG_NOISE   (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_DMA | LOG_NOISE)

#include "logmacro.h"

#define LOGDMA(...)       LOGMASKED(LOG_DMA,     __VA_ARGS__)
#define LOGNOISE(...)     LOGMASKED(LOG_NOISE,   __VA_ARGS__)


// device type definition
DEFINE_DEVICE_TYPE(SVISION_SND, svision_sound_device, "svision_sound", "Super Vision Custom Sound")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  svision_sound_device - constructor
//-------------------------------------------------

svision_sound_device::svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SVISION_SND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_cartrom(*this, finder_base::DUMMY_TAG)
	, m_mixer_channel(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void svision_sound_device::device_start()
{
	m_dma = DMA();
	m_noise = NOISE();
	std::fill(std::begin(m_channel), std::end(m_channel), CHANNEL());

	m_mixer_channel = stream_alloc(0, 2, machine().sample_rate());

	save_item(STRUCT_MEMBER(m_noise, reg));
	save_item(STRUCT_MEMBER(m_noise, on));
	save_item(STRUCT_MEMBER(m_noise, right));
	save_item(STRUCT_MEMBER(m_noise, left));
	save_item(STRUCT_MEMBER(m_noise, play));
	save_item(STRUCT_MEMBER(m_noise, state));
	save_item(STRUCT_MEMBER(m_noise, volume));
	save_item(STRUCT_MEMBER(m_noise, count));
	save_item(STRUCT_MEMBER(m_noise, step));
	save_item(STRUCT_MEMBER(m_noise, pos));
	save_item(STRUCT_MEMBER(m_noise, value));

	save_item(STRUCT_MEMBER(m_dma, reg));
	save_item(STRUCT_MEMBER(m_dma, on));
	save_item(STRUCT_MEMBER(m_dma, right));
	save_item(STRUCT_MEMBER(m_dma, left));
	save_item(STRUCT_MEMBER(m_dma, ca14to16));
	save_item(STRUCT_MEMBER(m_dma, start));
	save_item(STRUCT_MEMBER(m_dma, size));
	save_item(STRUCT_MEMBER(m_dma, pos));
	save_item(STRUCT_MEMBER(m_dma, step));
	save_item(STRUCT_MEMBER(m_dma, finished));

	save_item(STRUCT_MEMBER(m_channel, reg));
	save_item(STRUCT_MEMBER(m_channel, waveform));
	save_item(STRUCT_MEMBER(m_channel, volume));
	save_item(STRUCT_MEMBER(m_channel, pos));
	save_item(STRUCT_MEMBER(m_channel, size));
	save_item(STRUCT_MEMBER(m_channel, count));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void svision_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 lsum = 0;
		s32 rsum = 0;
		for (int j = 0; j < std::size(m_channel); j++)
		{
			CHANNEL &channel(m_channel[j]);
			if (channel.size != 0)
			{
				if (channel.on || channel.count)
				{
					bool on = false;
					switch (channel.waveform)
					{
						case 0:
							on = channel.pos <= (28 * channel.size) >> 5;
							break;
						case 1:
							on = channel.pos <= (24 * channel.size) >> 5;
							break;
						default:
						case 2:
							on = channel.pos <= channel.size / 2;
							break;
						case 3:
							on = channel.pos <= (9 * channel.size) >> 5;
							break;
					}
					{
						int16_t s = on ? channel.volume << 8 : 0;
						if (j == 0)
							rsum += s;
						else
							lsum += s;
					}
				}
				channel.pos++;
				if (channel.pos >= channel.size)
					channel.pos = 0;
			}
		}
		if (m_noise.on && (m_noise.play || m_noise.count))
		{
			int16_t s = (m_noise.value ? 1 << 8: 0) * m_noise.volume;
			int b1, b2;
			if (m_noise.left)
				lsum += s;
			if (m_noise.right)
				rsum += s;
			m_noise.pos += m_noise.step;
			if (m_noise.pos >= 1.0)
			{
				switch (m_noise.type)
				{
					case NOISE::Type::Type7Bit:
						m_noise.value = m_noise.state & 0x40 ? 1 : 0;
						b1 = (m_noise.state & 0x40) != 0;
						b2 = (m_noise.state & 0x20) != 0;
						m_noise.state = (m_noise.state << 1) + (b1 != b2 ? 1 : 0);
						break;
					case NOISE::Type::Type14Bit:
					default:
						m_noise.value = m_noise.state & 0x2000 ? 1 : 0;
						b1 = (m_noise.state & 0x2000) != 0;
						b2 = (m_noise.state & 0x1000) != 0;
						m_noise.state = (m_noise.state << 1) + (b1 != b2 ? 1 : 0);
				}
				m_noise.pos -= 1;
			}
		}
		if (m_dma.on)
		{
			uint8_t sample;
			int16_t s;
			uint16_t const addr = m_dma.start + (unsigned) m_dma.pos / 2;
			if (addr >= 0x8000 && addr < 0xc000)
			{
				sample = ((uint8_t*)m_cartrom->base())[(addr & 0x3fff) | m_dma.ca14to16];
			}
			else
			{
				sample = m_maincpu->space(AS_PROGRAM).read_byte(addr);
			}
			if (((unsigned)m_dma.pos) & 1)
				s = (sample & 0xf);
			else
				s = (sample & 0xf0) >> 4;
			s <<= 8;
			if (m_dma.left)
				lsum += s;
			if (m_dma.right)
				rsum += s;
			m_dma.pos += m_dma.step;
			if (m_dma.pos >= m_dma.size)
			{
				m_dma.finished = true; // TODO: only ever set, never read?
				m_dma.on = false;
				m_irq_cb(1);
			}
		}
		stream.put_int(0, i, lsum, 32768);
		stream.put_int(1, i, rsum, 32768);
	}
}


void svision_sound_device::sounddma_w(offs_t offset, uint8_t data)
{
	LOGDMA("%.6f svision snddma write %04x %02x\n", machine().time().as_double(), offset + 0x18, data);
	m_dma.reg[offset] = data;
	switch (offset)
	{
		case 0:
		case 1:
			m_dma.start = (m_dma.reg[0] | (m_dma.reg[1] << 8));
			break;
		case 2:
			m_dma.size = (data ? data : 0x100) * 32;
			break;
		case 3:
			m_dma.step = unscaled_clock() / (256.0 * machine().sample_rate() * (1 + (data & 3)));
			m_dma.right = data & 4;
			m_dma.left = data & 8;
			m_dma.ca14to16 = ((data & 0x70) >> 4) << 14;
			break;
		case 4:
			m_dma.on = data & 0x80;
			if (m_dma.on)
			{
				m_dma.pos = 0.0;
			}
			break;
	}
}


void svision_sound_device::noise_w(offs_t offset, uint8_t data)
{
	LOGNOISE("%.6f svision noise write %04x %02x\n", machine().time().as_double(), offset + 0x28, data);
	m_noise.reg[offset] = data;
	switch (offset)
	{
		case 0:
			m_noise.volume = data & 0xf;
			m_noise.step = unscaled_clock() / (256.0 * machine().sample_rate() * (1 + (data >> 4)));
			break;
		case 1:
			m_noise.count = data + 1;
			break;
		case 2:
			m_noise.type = NOISE::Type(data & 1);
			m_noise.play = data & 2;
			m_noise.right = data & 4;
			m_noise.left = data & 8;
			m_noise.on = data & 0x10; // honey bee start
			m_noise.state = 1;
			break;
	}
	m_noise.pos = 0.0;
}


void svision_sound_device::sound_decrement()
{
	if (m_channel[0].count > 0)
		m_channel[0].count--;
	if (m_channel[1].count > 0)
		m_channel[1].count--;
	if (m_noise.count > 0)
		m_noise.count--;
}


void svision_sound_device::soundport_w(uint8_t which, offs_t offset, uint8_t data)
{
	uint16_t size;

	m_mixer_channel->update();
	m_channel[which].reg[offset] = data;

	switch (offset)
	{
		case 0:
		case 1:
			size = m_channel[which].reg[0] | ((m_channel[which].reg[1] & 7) << 8);
			if (size)
			{
				// m_channel[which].size = (int) (machine().sample_rate() * (size << 5) / 4e6);
				m_channel[which].size = (int) (machine().sample_rate() * (size << 5) / unscaled_clock());
			}
			else
			{
				m_channel[which].size = 0;
			}
			m_channel[which].pos = 0;
			break;
		case 2:
			m_channel[which].on = data & 0x40;
			m_channel[which].waveform = (data & 0x30) >> 4;
			m_channel[which].volume = data & 0xf;
			break;
		case 3:
			m_channel[which].count = data + 1;
			break;
	}
}
