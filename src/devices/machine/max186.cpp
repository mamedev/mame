// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
	Maxim MAX186/MAX188 ADC
	Emulation by R. Belmont

	TODO: Reference settling after power-down, analog acquisition time/droop.
*/
#include "emu.h"
#include "max186.h"

#include <algorithm>
#include <cmath>

DEFINE_DEVICE_TYPE(MAX186, max186_device, "max186", "Maxim MAX186 ADC")

max186_device::max186_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAX186, tag, owner, clock)
	, m_input_cb(*this)
	, m_timer(nullptr)
	, m_result(0)
	, m_command(0)
	, m_command_bits(0)
	, m_output_bits(0)
	, m_cs(true)
	, m_din(false)
	, m_sclk(false)
	, m_shdn(true)
	, m_dout(false)
	, m_sstrb(true)
	, m_external_clock(false)
	, m_busy(false)
	, m_restart(true)
{
}

void max186_device::device_start()
{
	m_input_cb.resolve_safe(0.0);
	m_timer = timer_alloc(FUNC(max186_device::conversion_done), this);
	save_item(NAME(m_result));
	save_item(NAME(m_command));
	save_item(NAME(m_command_bits));
	save_item(NAME(m_output_bits));
	save_item(NAME(m_cs));
	save_item(NAME(m_din));
	save_item(NAME(m_sclk));
	save_item(NAME(m_shdn));
	save_item(NAME(m_dout));
	save_item(NAME(m_sstrb));
	save_item(NAME(m_external_clock));
	save_item(NAME(m_busy));
	save_item(NAME(m_restart));
}

void max186_device::cs_w(int state)
{
	if (m_cs != bool(state))
	{
		m_cs = bool(state);
		m_command_bits = 0;
		m_restart = true;
		if (!m_cs && m_external_clock)
		{
			m_sstrb = false;
		}
	}
}

void max186_device::shdn_w(int state)
{
	m_shdn = bool(state);
	if (!m_shdn)
	{
		m_timer->adjust(attotime::never);
		m_busy = false;
		m_command_bits = m_output_bits = 0;
		m_dout = false;
		m_restart = true;
	}
}

void max186_device::sclk_w(int state)
{
	if (m_sclk == bool(state))
	{
		return;
	}
	m_sclk = bool(state);
	if (m_cs || !m_shdn)
	{
		return;
	}

	if (m_sclk)
	{
		// A new command can overlap the last five result bits. CS also
		// permits an early restart, abandoning the previous conversion.
		if (!m_command_bits && m_din && (m_restart || (!m_busy && m_output_bits <= 5)))
		{
			m_command = 1;
			m_command_bits = 1;
			m_restart = false;
		}
		else if (m_command_bits && m_command_bits < 8)
		{
			m_command = (m_command << 1) | m_din;
			++m_command_bits;
		}
	}
	else if (m_command_bits == 8)
	{
		m_command_bits = 0;
		convert();
	}
	else if (!m_busy)
	{
		if (m_external_clock)
		{
			m_sstrb = false;
		}
		m_dout = m_output_bits ? BIT(m_result, --m_output_bits) : 0;
	}
}

void max186_device::convert()
{
	// SEL0/SEL1 choose a pair; SEL2 chooses its positive input.
	const u8 channel = (BIT(m_command, 4, 2) << 1) | BIT(m_command, 6);
	double input = m_input_cb(channel);
	if (!BIT(m_command, 2))
	{
		input -= m_input_cb(channel ^ 1);
	}
	const int code = int(std::floor(input * 4096.0 + 0.5));
	m_result = (BIT(m_command, 3) ? std::clamp(code, 0, 4095) : std::clamp(code, -2048, 2047)) & 0xfff;
	m_dout = false;
	m_output_bits = 12;

	// Power-down commands retain the preceding clock mode, and power
	// down only after this conversion. A new START wakes the converter.
	if (BIT(m_command, 1))
	{
		m_external_clock = BIT(m_command, 0);
	}
	m_busy = !m_external_clock;
	m_sstrb = m_external_clock;
	m_timer->adjust(m_busy ? attotime::from_nsec(5500) : attotime::never);
}

TIMER_CALLBACK_MEMBER(max186_device::conversion_done)
{
	m_busy = false;
	m_sstrb = true;
}
