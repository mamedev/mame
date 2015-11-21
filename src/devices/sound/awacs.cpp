// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    awacs.c

    AWACS/Singer style 16-bit audio I/O for '040 and PowerPC Macs

    Emulation by R. Belmont

***************************************************************************/

#include "emu.h"
#include "awacs.h"

// device type definition
const device_type AWACS = &device_creator<awacs_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  awacs_device - constructor
//-------------------------------------------------

awacs_device::awacs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AWACS, "AWACS", tag, owner, clock, "awacs", __FILE__),
		device_sound_interface(mconfig, *this), m_stream(nullptr), m_play_ptr(0), m_buffer_size(0), m_buffer_num(0), m_playback_enable(false), m_dma_space(nullptr), m_dma_offset_0(0), m_dma_offset_1(0), m_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void awacs_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 22050);

	memset(m_regs, 0, sizeof(m_regs));

	m_timer = timer_alloc(0, NULL);

	save_item(NAME(m_play_ptr));
	save_item(NAME(m_buffer_size));
	save_item(NAME(m_playback_enable));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void awacs_device::device_reset()
{
	m_stream->update();

	memset(m_regs, 0, sizeof(m_regs));

	m_play_ptr = 0;
	m_buffer_size = 0;
	m_playback_enable = false;
	m_dma_space = NULL;
	m_dma_offset_0 = m_dma_offset_1 = 0;
	m_buffer_num = 0;
}

//-------------------------------------------------
//  device_timer - called when our device timer expires
//-------------------------------------------------

void awacs_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void awacs_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;
	int offset = (m_buffer_num == 0) ? m_dma_offset_0 : m_dma_offset_1;

	outL = outputs[0];
	outR = outputs[1];

	if (m_playback_enable)
	{
		for (int i = 0; i < samples; i++)
		{
			outL[i] = (INT16)m_dma_space->read_word(offset + m_play_ptr);
			outR[i] = (INT16)m_dma_space->read_word(offset + m_play_ptr + 2);
			m_play_ptr += 4;
		}

		// out of buffer?
		if (m_play_ptr >= m_buffer_size)
		{
			UINT8 bufflag[2] = { 0x40, 0x80 };

			m_regs[0x18] |= bufflag[m_buffer_num];
			m_buffer_num ^= 1;
			m_play_ptr = 0;
		}
	}
	else
	{
		for (int i = 0; i < samples; i++)
		{
			outL[i] = 0;
			outR[i] = 0;
		}
	}
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

READ8_MEMBER( awacs_device::read )
{
	return m_regs[offset];
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

WRITE8_MEMBER( awacs_device::write )
{
	switch (offset)
	{
		case 0x8:
		case 0x9:
			m_regs[offset] = data;
			m_buffer_size = (m_regs[8]<<6) | (m_regs[9]>>2);
//            printf("buffer size = %x samples, %x bytes\n", m_buffer_size, m_buffer_size*4);
			m_buffer_size *= 4; // samples * 16 bits * stereo
			break;

		case 0x10:
			{
				static const int rates[4] = { 22100, 29400, 44100, 22100 };
				m_stream->set_sample_rate(rates[(data>>1)&3]);
//                printf("rate %d, enable: %d\n", rates[(data>>1)&3], data & 1);
				m_playback_enable = (data & 1) ? true : false;

				if (m_playback_enable && !(m_regs[0x10]&1))
				{
					m_play_ptr = 0;
					m_buffer_num = 0;
				}
			}
			break;

		case 0x18:
			m_regs[offset] &= 0xf0;
			m_regs[offset] |= (data & 0x0f);
			m_regs[offset] &= ~(data & 0xf0);
			return;
	}

	m_regs[offset] = data;
}

void awacs_device::set_dma_base(address_space &space, int offset0, int offset1)
{
	m_dma_space = &space;
	m_dma_offset_0 = offset0;
	m_dma_offset_1 = offset1;
}
