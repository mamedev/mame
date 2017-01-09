// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*****************************************************************************

    5/74164 8-bit parallel-out serial shift registers

*****************************************************************************/

#include "emu.h"
#include "74164.h"

const device_type TTL74164 = &device_creator<ttl74164_device>;

ttl74164_device::ttl74164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL74164, "74164 8-bit parallel-out serial-in", tag, owner, clock, "74164", __FILE__)
	, m_qa_func(*this)
	, m_qb_func(*this)
	, m_qc_func(*this)
	, m_qd_func(*this)
	, m_qe_func(*this)
	, m_qf_func(*this)
	, m_qg_func(*this)
	, m_qh_func(*this)
	, m_output_func(*this)
	, m_clear(0)
	, m_clk(0)
	, m_a(0)
	, m_b(0)
	, m_out(0)
{
}

void ttl74164_device::device_start()
{
	save_item(NAME(m_clear));
	save_item(NAME(m_clk));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_out));

	m_qa_func.resolve_safe();
	m_qb_func.resolve_safe();
	m_qc_func.resolve_safe();
	m_qd_func.resolve_safe();
	m_qe_func.resolve_safe();
	m_qf_func.resolve_safe();
	m_qg_func.resolve_safe();
	m_qh_func.resolve_safe();
	m_output_func.resolve(); // To be able to use isnull() test
}

void ttl74164_device::device_reset()
{
}

WRITE_LINE_MEMBER( ttl74164_device::clear_w )
{
	if (m_clear != 0 && state == 0)
		m_out = 0;
	m_clear = state;

}

WRITE_LINE_MEMBER( ttl74164_device::clock_w )
{
	if (m_clk == 0 && state == 1 && m_clear != 0)
    {
		if (m_a == 1 && m_b == 1)
			m_out = 0x80 | (m_out >> 1);
		else
			m_out = 0x7f & (m_out >> 1);

		// Callbacks
		if (m_output_func.isnull()) // Mixing write lines and the write callback is not supported
		{
			m_qa_func(0x80 & m_out ? 1 : 0);
			m_qb_func(0x40 & m_out ? 1 : 0);
			m_qc_func(0x20 & m_out ? 1 : 0);
			m_qd_func(0x10 & m_out ? 1 : 0);
			m_qe_func(0x08 & m_out ? 1 : 0);
			m_qf_func(0x04 & m_out ? 1 : 0);
			m_qg_func(0x02 & m_out ? 1 : 0);
			m_qh_func(0x01 & m_out ? 1 : 0);
		}
		else
		{
			m_output_func(m_out);
		}
	}	
}

WRITE_LINE_MEMBER( ttl74164_device::a_w )
{
	m_a = state;
}

WRITE_LINE_MEMBER( ttl74164_device::b_w )
{
	m_b = state;
}

READ_LINE_MEMBER( ttl74164_device::output_r )
{
	return m_out;
}
