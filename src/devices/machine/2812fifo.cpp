// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**************************************************************************

    2812 32*8 First-In First-Out Memory (AMD, Plessey, and others)

    These devices contain a file of 32 data registers and corresponding
    control registers indicating when the data registers are valid.
    Data ripples from the input register towards the output until it
    reaches a register containing valid data.  The first and last data
    registers support serial operation.

    The half-full flag responds to the number of valid locations.  It
    favours glitch-free operation over precision.  It's never asserted
    when fewer than 13 registers are valid, and it's always asserted
    when at least 16 registers are valid.

    TODO:
    * Propagation delays
    * Serial I/O
    * Am2813 32*9 version without serial I/O

**************************************************************************/

#include "emu.h"
#include "2812fifo.h"

#include <algorithm>
#include <cassert>


DEFINE_DEVICE_TYPE(FIFO2812, fifo2812_device, "fifo2812", "2812 32x8 FIFO Memory");


fifo2812_device::fifo2812_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, FIFO2812, tag, owner, clock),
	m_q_cb(*this),
	m_ir_cb(*this),
	m_or_cb(*this),
	m_flag_cb(*this),
	m_control(0U),
	m_count(0U),
	m_d(0U), m_mr(1U), m_pl(0U), m_pd(0U), m_oe(1U)
{
	std::fill(std::begin(m_data), std::end(m_data), 0U);
}

fifo2812_device::~fifo2812_device()
{
}


void fifo2812_device::mr_w(int state)
{
	if (bool(state) != bool(m_mr))
	{
		m_mr = state ? 1U : 0U;
		if (!m_mr)
			device_reset();
	}
}

void fifo2812_device::pl_w(int state)
{
	if (bool(state) != bool(m_pl))
	{
		m_pl = state ? 1U : 0U;
		if (m_mr)
		{
			if (m_pl)
			{
				m_data[0] = m_d;
				if (!BIT(m_control, 0))
				{
					m_control |= u32(1) << 0;
					m_ir_cb(1);
					if (15U == ++m_count)
						m_flag_cb(1);
				}
			}
			else if (BIT(m_control, 0) && !BIT(m_control, 1))
			{
				unsigned bit(1);
				m_data[bit] = m_data[bit - 1];
				m_control |= u32(1) << bit;
				m_control &= ~(u32(1) << (bit - 1));
				m_ir_cb(0);
				for (++bit; ((LENGTH - 1) > bit) && !BIT(m_control, bit); ++bit)
				{
					m_data[bit] = m_data[bit - 1];
					m_control |= u32(1) << bit;
					m_control &= ~(u32(1) << (bit - 1));
				}
				if (!m_pd && ((LENGTH - 1) == bit) && !BIT(m_control, bit))
				{
					if (m_data[bit] != m_data[bit - 1])
					{
						m_data[bit] = m_data[bit - 1];
						if (m_oe)
							m_q_cb(0U, m_data[bit], 0xffU);
					}
					m_control |= u32(1) << bit;
					m_control &= ~(u32(1) << (bit - 1));
					m_or_cb(1);
				}
			}
		}
	}
}

void fifo2812_device::pd_w(int state)
{
	if (bool(state) != bool(m_pd))
	{
		m_pd = state ? 1U : 0U;
		if (m_mr)
		{
			if (m_pd)
			{
				if (BIT(m_control, LENGTH - 1))
				{
					m_control &= ~(u32(1) << (LENGTH - 1));
					m_or_cb(0);
					if (15U == m_count--)
						m_flag_cb(0);
				}
			}
			else if (BIT(m_control, LENGTH - 2))
			{
				unsigned bit(LENGTH - 2);
				if (m_data[bit + 1] != m_data[bit])
				{
					m_data[bit + 1] = m_data[bit];
					if (m_oe)
						m_q_cb(0U, m_data[bit + 1], 0xffU);
				}
				m_control |= u32(1) << (bit + 1);
				m_control &= ~(u32(1) << bit);
				m_or_cb(1);
				for (--bit; (0U < bit) && BIT(m_control, bit); --bit)
				{
					m_data[bit + 1] = m_data[bit];
					m_control |= u32(1) << (bit + 1);
					m_control &= ~(u32(1) << bit);
				}
				if (!m_pl && (0U == bit) && BIT(m_control, bit))
				{
					m_data[bit + 1] = m_data[bit];
					m_control |= u32(1) << (bit + 1);
					m_control &= ~(u32(1) << bit);
					m_ir_cb(0);
				}
			}
		}
	}
}

void fifo2812_device::oe_w(int state)
{
	if (bool(state) != bool(m_oe))
	{
		m_oe = state ? 1U : 0U;
		if (m_oe)
			m_q_cb(0U, m_data[LENGTH - 1], 0xffU);
		else
			m_q_cb(0U, 0xffU, 0x00U);
	}
}


u8 fifo2812_device::read()
{
	if (machine().side_effects_disabled())
	{
		return m_data[LENGTH - 1];
	}
	else
	{
		assert(!m_pd);
		pd_w(1);
		u8 const data(m_data[LENGTH - 1]);
		pd_w(0);
		return data;
	}
}

void fifo2812_device::write(u8 data)
{
	assert(!m_pl);
	d_w(data);
	pl_w(1);
	pl_w(0);
}


void fifo2812_device::device_resolve_objects()
{
	m_d = 0U;
	m_mr = 1U;
	m_pl = 0U;
	m_pd = 0U;
	m_oe = 1U;
}

void fifo2812_device::device_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_data));
	save_item(NAME(m_count));
	save_item(NAME(m_d));
	save_item(NAME(m_mr));
	save_item(NAME(m_pl));
	save_item(NAME(m_pd));
	save_item(NAME(m_oe));
}

void fifo2812_device::device_reset()
{
	u32 const prev_ir(BIT(m_control, 0));
	u32 const prev_or(BIT(m_control, LENGTH - 1));
	u8 const prev_q(m_data[LENGTH - 1]);
	bool const prev_flag(15U <= m_count);
	m_control = 0U;
	std::fill(std::begin(m_data), std::end(m_data), 0U);
	m_count = 0U;
	if (m_oe && (0U != prev_q))
		m_q_cb(0U, 0U, 0xffU);
	if (prev_ir)
		m_ir_cb(0);
	if (prev_or)
		m_or_cb(0);
	if (prev_flag)
		m_flag_cb(0);
}
