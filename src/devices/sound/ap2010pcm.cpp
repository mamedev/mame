// license:BSD-3-Clause
// copyright-holders:QUFB
/**********************************************************************

    PCM audio functions of the AP2010 LSI

**********************************************************************/

#include "emu.h"

#include "ap2010pcm.h"

#include <algorithm>

#define VERBOSE (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(AP2010PCM, ap2010pcm_device, "ap2010pcm", "AP2010 PCM")

ap2010pcm_device::ap2010pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AP2010PCM, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_sample_rate(0)
	, m_fifo_size(0)
	, m_fifo_head(0)
	, m_fifo_tail(0)
	, m_fifo_fast_size(0)
	, m_fifo_fast_head(0)
	, m_fifo_fast_tail(0)
	, m_stream(nullptr)
{ }

void ap2010pcm_device::device_start()
{
	m_regs = make_unique_clear<uint32_t[]>(0x40/4);

	m_sample_rate = 8000;

	std::fill(std::begin(m_fifo_data), std::end(m_fifo_data), 0);
	std::fill(std::begin(m_fifo_fast_data), std::end(m_fifo_fast_data), 0);

	m_stream = stream_alloc(0, 1, m_sample_rate);

	save_pointer(NAME(m_regs), 0x40/4);

	save_item(NAME(m_volume));

	save_item(NAME(m_sample_rate));

	save_item(NAME(m_fifo_data));
	save_item(NAME(m_fifo_size));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));

	save_item(NAME(m_fifo_fast_data));
	save_item(NAME(m_fifo_fast_size));
	save_item(NAME(m_fifo_fast_head));
	save_item(NAME(m_fifo_fast_tail));
}

void ap2010pcm_device::sound_stream_update(sound_stream &stream)
{
	int16_t sample = 0;
	uint16_t sample_empty_count = 0;
	uint16_t fifo_size = m_fifo_size;
	uint16_t fifo_fast_size = m_fifo_fast_size;
	for (size_t i = 0; i < stream.samples(); i++) {
		if (m_fifo_fast_size) {
			sample = fifo_fast_pop();
		} else if (m_fifo_size) {
			sample = fifo_pop();
		} else {
			sample = 0;
			sample_empty_count++;
		}

		stream.put_int(0, i, sample * m_volume, 32768);
	}
	if (fifo_size && sample_empty_count) {
		LOG("pcm 0s = %d (had %d + fast %d, needed %d)\n", sample_empty_count, fifo_size, fifo_fast_size, stream.samples());
	}
}

uint32_t ap2010pcm_device::reg_r(offs_t offset)
{
	offset &= 0x3f;
	if (offset == 0) {
		// PCM data (0x5001000c) only received when 0x50010000 & 1 != 0;
		// PCM parameters (0x50010010, 0x50010018) only received when 0x50010000 & 4 != 0;
		return (m_regs[0x4/4] != 0) ? 0x0f : 0;
	} else if (offset == 0x4/4 && m_fifo_size > 0x1ff) {
		// TODO: Verify in hardware, bit 1 might be cleared while busy playing?
		return m_regs[offset] & 0xfffffffe;
	} else if (offset == 0x1c/4) {
		uint32_t fifo_size = m_fifo_size;
		LOG("pcm asked size -> %d\n", fifo_size);

		if (fifo_size > 0x1ff) {
			// FIXME: Expected max length, what happens if more data is streamed?
			fifo_size = 0x1ff;
		} else if (fifo_size > 1) {
			/*
			    Workaround to avoid missing samples during data stream:

			    while (dVar1 = read_volatile_4(PCM_CTRL), (dVar1 & 1) != 0) {
			        len = read_volatile_4(PCM_BUFLEN);
			        len_diff = 0x13e - (len & 0x1ff);
			        cVar2 = read_volatile_1(w_sound_test_var1);
			        if (cVar2 == '\x10') {
			            if (len_diff < 1) {
			                return in_lr;
			            }
			            pcm_write_1x_data();
			        }
			        ...
			    }
			*/
			fifo_size -= 2;
		}
		return fifo_size;
	}

	return m_regs[offset];
}

void ap2010pcm_device::reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offset &= 0x3f;
	COMBINE_DATA(&m_regs[offset]);

	m_stream->update();

	switch (offset) {
		case 0x4/4:
			if ((data & 0x78) == 0x78) {
				m_sample_rate = 8000 * (1 + BIT(data, 1));
				m_stream->set_sample_rate(m_sample_rate);

				// When a new stream starts, stop playback of previous stream
				m_fifo_size = 0;
				m_fifo_head = 0;
				m_fifo_tail = 0;

				LOG("pcm stream start, rate = %d\n", m_sample_rate);
			}
			break;
		case 0xc/4:
			if (ACCESSING_BITS_16_31) {
				fifo_push((data & 0xffff0000U) >> 16);
			}
			if (ACCESSING_BITS_0_15) {
				fifo_push(data & 0x0000ffffU);
			}
			break;
		// These samples are always played first
		case 0x10/4:
			if (ACCESSING_BITS_16_31) {
				fifo_fast_push((data & 0xffff0000U) >> 16);
			}
			if (ACCESSING_BITS_0_15) {
				fifo_fast_push(data & 0x0000ffffU);
			}
			break;
		// Panning. TODO: Identify bits for each channel
		case 0x14/4:
			LOG("pcm pan = %08x\n", data);
			break;
		// Volume control. When video output is disabled, it's possible to adjust volume
		// using the 2 touch areas on the bottom-left of the Storyware. Range 0..345
		case 0x18/4:
			m_volume = std::min(((data & 0x1ff00000U) >> 20) / 345.0f, 1.0f);
			LOG("pcm vol = %08x -> %d\n", data, m_volume);
			break;
	}
}

uint16_t ap2010pcm_device::fifo_pop()
{
	uint16_t sample = m_fifo_data[m_fifo_head];
	m_fifo_head = (m_fifo_head + 1) & (FIFO_MAX_SIZE - 1);
	m_fifo_size--;
	return sample;
}

uint16_t ap2010pcm_device::fifo_fast_pop()
{
	uint16_t sample = m_fifo_fast_data[m_fifo_fast_head];
	m_fifo_fast_head = (m_fifo_fast_head + 1) & (FIFO_MAX_SIZE - 1);
	m_fifo_fast_size--;
	return sample;
}

void ap2010pcm_device::fifo_push(uint16_t sample)
{
	if (sample == 0) {
		return;
	}

	// trash old data
	if (m_fifo_size > FIFO_MAX_SIZE - 1) {
		fifo_pop();
	}

	m_fifo_data[m_fifo_tail] = sample;
	m_fifo_tail = (m_fifo_tail + 1) & (FIFO_MAX_SIZE - 1);
	m_fifo_size++;
}

void ap2010pcm_device::fifo_fast_push(uint16_t sample)
{
	if (sample == 0) {
		return;
	}

	// trash old data
	if (m_fifo_fast_size > FIFO_MAX_SIZE - 1) {
		fifo_fast_pop();
	}

	m_fifo_fast_data[m_fifo_fast_tail] = sample;
	m_fifo_fast_tail = (m_fifo_fast_tail + 1) & (FIFO_MAX_SIZE - 1);
	m_fifo_fast_size++;
}
