// license:BSD-3-Clause
// copyright-holders:MrBaristaNZ
/***************************************************************************

    Cirrus Logic CS4341 Stereo DAC with I2C control port

    The sample rate is the LRCK the board supplies, set here as a fixed
    MCLK / LRCK ratio.  Control port registers (datasheet DS298PP2):
    0: MCLK Control (bit 1 MCLK divide by 2, revision C and later)
    1: Mode Control (bit 7 auto-mute, bits 6-4 data format, bits 3-2
       de-emphasis, bit 1 quiescent voltage ramp, bit 0 power down)
    2: Volume and Mixing Control (bit 7 channel A volume sets both
       channels, bit 6 soft ramp, bit 5 zero cross, bits 4-0 ATAPI mixing)
    3, 4: channel A, B volume (bit 7 mute, bits 6-0 attenuation in 1 dB steps)
    The memory address pointer has auto-increment in bit 7 and the register
    number in bits 2-0.

    TODO:
    - ATAPI mixing other than normal stereo, de-emphasis, soft ramp and
      zero cross
    - register 0 (MCLK divide on later revisions) is stored but not used

***************************************************************************/

#include "emu.h"
#include "cs4341.h"

#include <algorithm>
#include <cmath>

#define LOG_WRITES      (1U << 1)
#define LOG_OVERRUNS    (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CS4341, cs4341_device, "cs4341", "Cirrus Logic CS4341 DAC")

cs4341_device::cs4341_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CS4341, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	i2c_hle_interface(mconfig, *this, 0x10),
	m_stream(nullptr),
	m_mclk_ratio(256)
{
}

void cs4341_device::device_start()
{
	m_stream = stream_alloc(0, 2, sample_rate());

	for (auto &buffer : m_buffer)
		std::fill(std::begin(buffer), std::end(buffer), 0);

	save_item(NAME(m_regs));
	save_item(NAME(m_buffer));
	save_item(NAME(m_bufin));
	save_item(NAME(m_bufout));
}

void cs4341_device::device_reset()
{
	// power-up defaults: auto-mute, quiescent ramp and power down on; stereo mixing
	m_regs[0] = 0x00;
	m_regs[1] = 0x83;
	m_regs[2] = 0x49;
	m_regs[3] = 0x00;
	m_regs[4] = 0x00;
	update_volume();

	std::fill(std::begin(m_bufin), std::end(m_bufin), 0);
	std::fill(std::begin(m_bufout), std::end(m_bufout), 0);
}

void cs4341_device::device_post_load()
{
	update_volume();
}

void cs4341_device::device_clock_changed()
{
	m_stream->set_sample_rate(sample_rate());
}

void cs4341_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 2; channel++)
	{
		u32 curout = m_bufout[channel];
		const u32 curin = m_bufin[channel];

		int sampindex;
		for (sampindex = 0; curout != curin && sampindex < stream.samples(); sampindex++)
		{
			stream.put(channel, sampindex, m_buffer[channel][curout] * m_volume[channel]);
			curout = (curout + 1) % BUFFER_SIZE;
		}
		m_bufout[channel] = curout;
		stream.fill(channel, 0.0, sampindex);
	}
}

void cs4341_device::write(offs_t channel, s16 data)
{
	m_stream->update();

	const u32 next = (m_bufin[channel] + 1) % BUFFER_SIZE;
	if (next != m_bufout[channel])
	{
		m_buffer[channel][m_bufin[channel]] = data;
		m_bufin[channel] = next;
	}
	else
	{
		LOGMASKED(LOG_OVERRUNS, "write: buffer overrun on channel %d\n", channel);
	}
}

u8 cs4341_device::read_data(u16 offset)
{
	hold_pointer(offset);
	const u8 reg = offset & 0x07;
	return (reg < std::size(m_regs)) ? m_regs[reg] : 0;
}

void cs4341_device::write_data(u16 offset, u8 data)
{
	hold_pointer(offset);
	const u8 reg = offset & 0x07;
	LOGMASKED(LOG_WRITES, "%s: register %02x = %02x\n", machine().describe_context(), offset, data);
	if (reg >= std::size(m_regs))
	{
		logerror("%s: write to unknown register %02x = %02x\n", machine().describe_context(), offset, data);
		return;
	}

	m_stream->update();
	m_regs[reg] = data;
	update_volume();
}

void cs4341_device::hold_pointer(u16 offset)
{
	// bit 7 of the memory address pointer enables auto-increment: without it, successive bytes
	// go to the same register, so undo the step the I2C interface takes after each byte
	if (!BIT(offset, 7))
		m_data_offset--;
}

void cs4341_device::update_volume()
{
	const bool power_down = BIT(m_regs[1], 0);
	const bool a_equals_b = BIT(m_regs[2], 7);

	for (int channel = 0; channel < 2; channel++)
	{
		const u8 volume = m_regs[(a_equals_b || !channel) ? 3 : 4];
		if (power_down || BIT(volume, 7))
			m_volume[channel] = 0.0;
		else
			m_volume[channel] = std::pow(10.0, -double(volume & 0x7f) / 20.0) / 32768.0;
	}
}
