// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    burgundy.cpp

     "Burgundy" stereo 16-bit audio CODEC (iMac, Blue & White G3)

***************************************************************************/

#include "emu.h"
#include "burgundy.h"

#define LOG_REGISTERS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(BURGUNDY, burgundy_device, "burgundy", "Burgundy audio I/O")

constexpr u32 CODEC_BUSY = (1 << 23);
constexpr u32 CODEC_PRESENT = (1 << 22);

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  burgundy_device - constructor
//-------------------------------------------------

burgundy_device::burgundy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BURGUNDY, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_output_cb(*this, 0)
	, m_input_cb(*this)
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void burgundy_device::device_start()
{
	// create the stream
	m_stream = stream_alloc(0, 2, clock() / 512, STREAM_SYNCHRONOUS);

	save_item(NAME(m_phase));
	save_item(NAME(m_active));
	save_item(NAME(m_registers));
	save_item(NAME(m_codec_status));
	save_item(NAME(m_counter));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void burgundy_device::device_reset()
{
	m_phase = 0;
	m_codec_status = 0;
	m_counter = 0;
	m_stream->set_sample_rate(clock() / 512);
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void burgundy_device::sound_stream_update(sound_stream &stream)
{
	if (m_codec_status & CODEC_BUSY)
	{
		m_codec_status &= ~CODEC_BUSY;   // clear busy
		m_counter++;
		m_counter &= 3;
	}

	if (m_active & ACTIVE_OUT)
	{
		const u32 data = swapendian_int32(m_output_cb(m_phase));
		const s16 left = data >> 16;
		const s16 right = data;
		stream.put_int(0, 0, left, 32768);
		stream.put_int(1, 0, right, 32768);
	}
	else
	{
		stream.put_int(0, 0, 0, 32768);
		stream.put_int(1, 0, 0, 32768);
	}

	m_phase = (m_phase + 1) & 0xfff;
}

uint32_t burgundy_device::read_macrisc(offs_t offset)
{
	switch (offset)
	{
	case 0: // Audio Control
		return 0;

	case 4: // Audio CODEC Control
		return m_last_codec_control;

	case 8: // Audio CODEC Status
		return m_codec_status;
	}

	return 0;
}

void burgundy_device::write_macrisc(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		case 0: // Audio Control
			break;

		case 4: // Audio CODEC Control
			m_last_codec_control = data;

			m_reg_addr = (data >> 12) & 0xff;
			m_cur_byte = (data >> 8) & 3;
			m_last_byte = (data >> 10) & 3;

			if (BIT(data, 21))
			{
				u32 reg_mask = 0xff << (m_cur_byte << 3);
				m_registers[m_reg_addr] &= ~reg_mask;
				m_registers[m_reg_addr] |= ((data & 0xff) << (m_cur_byte << 3));
				LOGMASKED(LOG_REGISTERS, "%s: reg %x is now %x\n", tag(), m_reg_addr, m_registers[m_reg_addr]);

				if (m_reg_addr == 0x60)
				{
					if ((m_registers[0x60] & 6) != 0)
					{
						LOG("%s: Playback enabled\n", tag());
						m_active |= ACTIVE_OUT;
					}
					else
					{
						LOG("%s: Playback disabled\n", tag());
						m_active &= ~ACTIVE_OUT;
					}
				}
			}
			else
			{
				u8 reg_data = (m_registers[m_reg_addr] >> (m_cur_byte << 3)) & 0xff;
				m_codec_status = CODEC_BUSY | CODEC_PRESENT | (m_counter << 14) | (m_cur_byte << 12) | (reg_data << 4);
			}
			break;

		case 8: // Audio CODEC Status
			break;
	}
}
