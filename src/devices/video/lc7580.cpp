// license:BSD-3-Clause
// copyright-holders:hap
/*

Sanyo LC7580 LCD Driver

53 outputs (static), or 104 outputs (1/2 duty)

TODO:
- any difference between LC7580 and LC7582?
- OSC pin (input is R/C)
- AD/DSP function

*/

#include "emu.h"
#include "lc7580.h"

#include <tuple>


DEFINE_DEVICE_TYPE(LC7580, lc7580_device, "lc7580", "Sanyo LC7580 LCD Driver")
DEFINE_DEVICE_TYPE(LC7582, lc7582_device, "lc7582", "Sanyo LC7582 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

lc7580_device::lc7580_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_write_segs(*this)
{
	// disable nvram by default
	nvram_enable_backup(false);
}

lc7580_device::lc7580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	lc7580_device(mconfig, LC7580, tag, owner, clock)
{ }

lc7582_device::lc7582_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	lc7580_device(mconfig, LC7582, tag, owner, clock)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lc7580_device::device_start()
{
	// zerofill
	m_data = 0;
	m_ce = 0;
	m_clk = 0;
	m_blank = false;
	m_shift = 0;
	std::fill_n(m_latch, std::size(m_latch), 0);

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_ce));
	save_item(NAME(m_clk));
	save_item(NAME(m_blank));
	save_item(NAME(m_shift));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  nvram (when VDD is battery-backed)
//-------------------------------------------------

bool lc7580_device::nvram_write(util::write_stream &file)
{
	std::error_condition err;
	size_t actual;

	std::tie(err, actual) = write(file, &m_shift, sizeof(m_shift));
	if (err)
		return false;
	std::tie(err, actual) = write(file, &m_latch, sizeof(m_latch));
	if (err)
		return false;

	return true;
}

bool lc7580_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;

	std::tie(err, actual) = read(file, &m_shift, sizeof(m_shift));
	if (err || (sizeof(m_shift) != actual))
		return false;
	std::tie(err, actual) = read(file, &m_latch, sizeof(m_latch));
	if (err || (sizeof(m_latch) != actual))
		return false;

	return true;
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void lc7580_device::refresh_output()
{
	if (BIT(m_latch[0], 53))
	{
		// 1/2 duty
		u64 segs[2] = { 0, 0 };

		// COM1 on even bits, COM2 on uneven bits
		for (int i = 0; i < 104; i++)
			segs[i & 1] |= BIT(m_latch[i / 52], i % 52) << (i >> 1);

		for (int i = 0; i < 2; i++)
			m_write_segs(i, m_blank ? 0 : segs[i]);
	}
	else
	{
		// 1/1 duty
		m_write_segs(0, m_blank ? 0 : m_latch[0] & ((1ULL << 53) - 1));
		m_write_segs(1, 0);
	}
}

void lc7580_device::clk_w(int state)
{
	state = (state) ? 1 : 0;

	// clock shift register
	if (state && !m_clk)
		m_shift = m_shift >> 1 | u64(m_data) << 55;

	m_clk = state;
}

void lc7580_device::ce_w(int state)
{
	state = (state) ? 1 : 0;

	// latch at falling edge
	if (!state && m_ce)
	{
		// d0-d52: segment data
		// d53(0): DP (drive mode select, aka duty)
		// d54(0): DQ (AD/DSP function)
		// d55: latch select
		m_latch[BIT(m_shift, 55)] = m_shift;
		refresh_output();
	}

	m_ce = state;
}
