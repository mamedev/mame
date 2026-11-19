// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    Texas Instruments TLC5620 quadruple 8-bit digital-to-analog converter

    Four voltage output converters behind a serial interface.  Eleven
    bits are shifted in on falling edges of CLK: two that select a
    converter, RNG, which doubles its output, and the code, top bit first.
    A falling edge of LOAD gives them to the selected converter.  Its
    output follows at once while LDAC is low; with LDAC high it waits for
    LDAC to fall, so that several outputs can change together.

    An output is the reference voltage of its converter times the code
    over 256, twice that with RNG.

    TODO:
    - the references are not inputs here, an output is given relative to
      its own

***************************************************************************/

#include "emu.h"
#include "tlc5620.h"

#include <algorithm>
#include <iterator>


DEFINE_DEVICE_TYPE(TLC5620, tlc5620_device, "tlc5620", "Texas Instruments TLC5620 DAC")


tlc5620_device::tlc5620_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TLC5620, tag, owner, clock)
	, m_out_cb(*this)
	, m_shift(0)
	, m_clk(false)
	, m_data(false)
	, m_load(true)
	, m_ldac(false)
{
}

void tlc5620_device::device_start()
{
	std::fill(std::begin(m_input), std::end(m_input), 0);
	std::fill(std::begin(m_output), std::end(m_output), 0);

	save_item(NAME(m_shift));
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_clk));
	save_item(NAME(m_data));
	save_item(NAME(m_load));
	save_item(NAME(m_ldac));
}

void tlc5620_device::update(u32 channel)
{
	if (m_output[channel] != m_input[channel])
	{
		m_output[channel] = m_input[channel];
		m_out_cb(channel, BIT(m_output[channel], 0, 8) << BIT(m_output[channel], 8));
	}
}

void tlc5620_device::clk_w(int state)
{
	const bool clk = state != 0;
	if (m_clk && !clk)
		m_shift = ((m_shift << 1) | (m_data ? 1 : 0)) & 0x7ff;
	m_clk = clk;
}

void tlc5620_device::data_w(int state)
{
	m_data = state != 0;
}

void tlc5620_device::load_w(int state)
{
	const bool load = state != 0;
	if (m_load && !load)
	{
		const u32 channel = BIT(m_shift, 9, 2);
		m_input[channel] = BIT(m_shift, 0, 9);
		if (!m_ldac)
			update(channel);
	}
	m_load = load;
}

void tlc5620_device::ldac_w(int state)
{
	const bool ldac = state != 0;
	if (m_ldac && !ldac)
	{
		for (u32 channel = 0; channel < 4; channel++)
			update(channel);
	}
	m_ldac = ldac;
}
