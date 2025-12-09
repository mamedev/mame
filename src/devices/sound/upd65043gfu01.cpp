// license: BSD-3-Clause
// copyright-holders: Devin Acker

/*
    NEC uPD65043GF-U01

    This is the custom sound chip used in the Zoomer PDA.
    It features 3 square wave generators and 1 noise generator configurable as either white or
    "metal" noise, as well as an interrupt-driven 8-bit PCM stream.

    TODO:
    - Verify noise generator behavior (the current implementation is based on presumed similarities
      with the SN76489)
    - Verify PCM interrupt behavior (currently only tested with the "About" dialog in Palm-developed
      Zoomer system apps, which play a short .wav file)
*/

#include "emu.h"
#include "upd65043gfu01.h"

#include <algorithm>


namespace {

constexpr s8 OUTPUT_LEVEL[16] =
{
	0, 5, 6, 8, 10, 12, 15, 20, 25, 31, 40, 50, 63, 80, 100, 127
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(UPD65043GFU01, upd65043gfu01_device, "upd65043gfu01", "NEC uPD65043GF-U01")

//**************************************************************************
upd65043gfu01_device::upd65043gfu01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD65043GFU01, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_cb(*this)
{
}

//**************************************************************************
void upd65043gfu01_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / 8);
	m_irq_timer = timer_alloc(FUNC(upd65043gfu01_device::irq_timer), this);

	std::fill(std::begin(m_period), std::end(m_period), 0);
	std::fill(std::begin(m_count), std::end(m_count), 0);
	std::fill(std::begin(m_volume), std::end(m_volume), 0);
	std::fill(std::begin(m_output), std::end(m_output), 0);

	m_noise_mode = 0;

	m_pcm_period = m_pcm_count = 0;
	m_pcm_buffer_read = 0;
	m_pcm_buffer_write = 1;

	std::fill(std::begin(m_pcm_buffer), std::end(m_pcm_buffer), 0);

	save_item(NAME(m_control));
	save_item(NAME(m_period));
	save_item(NAME(m_count));
	save_item(NAME(m_volume));
	save_item(NAME(m_output));
	save_item(NAME(m_noise_mode));
	save_item(NAME(m_pcm_period));
	save_item(NAME(m_pcm_count));
	save_item(NAME(m_pcm_buffer));
	save_item(NAME(m_pcm_buffer_read));
	save_item(NAME(m_pcm_buffer_write));
}

//**************************************************************************
void upd65043gfu01_device::device_reset()
{
	m_control = 0xff; // disable everything?

	m_irq_timer->adjust(attotime::never);
	m_irq_cb(0);
}

//**************************************************************************
void upd65043gfu01_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 8);
	update_irq();
}

//**************************************************************************
u8 upd65043gfu01_device::read(offs_t offset)
{
	switch (offset & 0xf)
	{
	case 0x0: case 0x2: case 0x4:
		return m_period[offset >> 1];

	case 0x1: case 0x3: case 0x5:
		return m_period[offset >> 1] >> 8;

	case 0x6:
		return m_noise_mode;

	case 0x7: case 0x8: case 0x9: case 0xa:
		return m_volume[offset - 0x7];

	case 0xc:
		return m_control;

	case 0xd:
		return m_pcm_period;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: unknown register read 0x%x\n", machine().describe_context(), offset & 0xf);
		return 0;
	}
}

//**************************************************************************
void upd65043gfu01_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	switch (offset)
	{
	case 0x0: case 0x2: case 0x4:
		m_period[offset >> 1] &= 0x0f00;
		m_period[offset >> 1] |= data;

		if (offset == 0x4)
			update_noise();
		break;

	case 0x1: case 0x3: case 0x5:
		m_period[offset >> 1] &= 0x00ff;
		m_period[offset >> 1] |= (data << 8);

		if (offset == 0x5)
			update_noise();
		break;

	case 0x6:
		m_noise_mode = data;
		update_noise();
		break;

	case 0x7: case 0x8: case 0x9: case 0xa:
		m_volume[offset - 0x7] = data & 0xf;
		break;

	case 0xb:
		m_pcm_buffer[m_pcm_buffer_write & 0x1ff] = s8(u8(data - 0x80));
		m_pcm_buffer_write++;
		break;

	case 0xc:
		m_control = data;
		update_irq();
		break;

	case 0xd:
		m_pcm_period = data << 1;
		update_irq();
		break;

	default:
		logerror("%s: unknown register write %x = %02x\n", machine().describe_context(), offset, data);
		break;
	}
}

//**************************************************************************
TIMER_CALLBACK_MEMBER(upd65043gfu01_device::irq_timer)
{
	m_irq_cb(1);
}

//**************************************************************************
void upd65043gfu01_device::update_irq()
{
	if (!BIT(m_control, 2) || BIT(m_control, 4))
	{
		// IRQ disabled or PCM stopped
		m_irq_timer->adjust(attotime::never);
		m_irq_cb(0);
	}
	else
	{
		// recalculate timer so the IRQ fires when there are 128 samples or fewer in the buffer
		// (GEOS will push up to 128 more samples at this point)
		const u16 samples_left = std::min<u16>(128, (m_pcm_buffer_write - m_pcm_buffer_read) & 0x1ff);
		const u16 ticks_left = (samples_left * m_pcm_period) - m_pcm_count;
		m_irq_timer->adjust(m_stream->sample_period() * ticks_left);
	}
}

//**************************************************************************
void upd65043gfu01_device::update_noise()
{
	// TODO: this behavior is a complete guess
	m_period[3] = BIT(m_control, 0) ? (m_period[2] << 1) : (1 << 5);
}

//**************************************************************************
void upd65043gfu01_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s16 sample = 0;

		for (int i = 0; i < 4; i++)
		{
			m_count[i]++;
			if (m_count[i] >= m_period[i])
			{
				m_count[i] = 0;

				if (i < 3)
					m_output[i] ^= 1;
				else
					m_output[i] = machine().rand() & 1;
			}

			if (!BIT(m_control, 3))
			{
				if (BIT(m_output[i], 0))
					sample += OUTPUT_LEVEL[m_volume[i] & 0xf];
				else
					sample -= OUTPUT_LEVEL[m_volume[i] & 0xf];
			}
		}

		m_pcm_count++;
		if (m_pcm_count >= m_pcm_period)
		{
			m_pcm_count = 0;
			if (((m_pcm_buffer_read + 1) ^ m_pcm_buffer_write) & 0x1ff)
				m_pcm_buffer_read++;
		}

		if (!BIT(m_control, 4))
			sample += m_pcm_buffer[m_pcm_buffer_read & 0x1ff];

		stream.put_int_clamp(0, i, sample, 1 << 10);
	}
}
