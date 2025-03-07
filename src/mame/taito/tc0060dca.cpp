// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

   tc0060dca.cpp - Taito TC0060DCA stereo programmable volume control
   Emulation by R. Belmont, volume curve measured by Stephen Leary

***************************************************************************/

#include "emu.h"
#include "tc0060dca.h"

#include <cmath>

DEFINE_DEVICE_TYPE(TC0060DCA, tc0060dca_device, "tc0060dca", "Taito TC0060DCA volume control")

tc0060dca_device::tc0060dca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TC0060DCA, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr)
{
}

void tc0060dca_device::volume1_w(u8 data)
{
	m_stream->update();
	m_gain[0] = m_atten_table[data];
}

void tc0060dca_device::volume2_w(u8 data)
{
	m_stream->update();
	m_gain[1] = m_atten_table[data];
}

void tc0060dca_device::device_start()
{
	m_stream = stream_alloc(2, 2, SAMPLE_RATE_OUTPUT_ADAPTIVE);

	for (int x = 0; x < 256; x++)
		m_atten_table[x] = 1.0 / (1.0 + exp(-10 * ((x / 256.0) - 0.6)));

	m_gain[0] = m_gain[1] = m_atten_table[0xff];
	save_item(NAME(m_gain));
}

void tc0060dca_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	for (int i = 0; i < outputs[0].samples(); i++)
	{
		const stream_buffer::sample_t l = inputs[0].get(i);
		const stream_buffer::sample_t r = inputs[1].get(i);
		outputs[0].put(i, l * m_gain[0]);
		outputs[1].put(i, r * m_gain[1]);
	}
}
