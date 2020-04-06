// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    54/74161 4-bit binary counter

*****************************************************************************/

#include "emu.h"
#include "74161.h"

DEFINE_DEVICE_TYPE(TTL74160, ttl74160_device, "ttl74160", "54/74160 Decade Counter")
DEFINE_DEVICE_TYPE(TTL74161, ttl74161_device, "ttl74161", "54/74161 Binary Counter")
DEFINE_DEVICE_TYPE(TTL74162, ttl74162_device, "ttl74162", "54/74162 Decade Counter")
DEFINE_DEVICE_TYPE(TTL74163, ttl74163_device, "ttl74163", "54/74163 Binary Counter")

ttl7416x_device::ttl7416x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool synchronous_reset, uint8_t limit)
	: device_t(mconfig, type, tag, owner, clock)
	, m_qa_func(*this)
	, m_qb_func(*this)
	, m_qc_func(*this)
	, m_qd_func(*this)
	, m_output_func(*this)
	, m_tc_func(*this)
	, m_clear(0)
	, m_pe(0)
	, m_cet(0)
	, m_cep(0)
	, m_pclock(0)
	, m_p(0)
	, m_cetpre(0)
	, m_ceppre(0)
	, m_out(0)
	, m_synchronous_reset(synchronous_reset)
	, m_limit(limit)
{
}

ttl74160_device::ttl74160_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7416x_device(mconfig, TTL74160, tag, owner, clock, false, 10)
{
}

ttl74161_device::ttl74161_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7416x_device(mconfig, TTL74161, tag, owner, clock, false, 16)
{
}

ttl74162_device::ttl74162_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7416x_device(mconfig, TTL74162, tag, owner, clock, true, 10)
{
}

ttl74163_device::ttl74163_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl7416x_device(mconfig, TTL74163, tag, owner, clock, true, 16)
{
}

void ttl7416x_device::device_start()
{
	save_item(NAME(m_clear));
	save_item(NAME(m_pe));
	save_item(NAME(m_cet));
	save_item(NAME(m_cep));
	save_item(NAME(m_pclock));
	save_item(NAME(m_p));
	save_item(NAME(m_out));

	m_output_func.resolve_safe();
	m_tc_func.resolve_safe();
}

void ttl7416x_device::device_reset()
{
	init();
}

void ttl7416x_device::init()
{
	m_clear = 0;
	m_pe = 1;
	m_cet = m_cetpre;
	m_cep = m_ceppre;
	m_pclock = 0;
	m_p = 0;

	m_out = 0;
}

void ttl7416x_device::tick()
{
	if (m_synchronous_reset && m_clear)
	{
		init();
		return;
	}

	uint8_t last_out = m_out;
	uint8_t last_tc = tc_r();

	if (m_pe)
	{
		m_out = m_p & 0xf;
	}
	else
	{
		if (bool(m_cet) && bool(m_cep))
		{
			increment();
		}
	}

	if (m_out != last_out)
	{
		m_output_func(m_out);
	}

	if (tc_r() != last_tc)
	{
		m_tc_func(tc_r());
	}
}

/*
    State machine:
    limit 16: 0->1->2->...->e->f(tc)->0
    limit 10: 0->1->2->...->9(tc)->0
              10->11->4(no tc)
              12->13->4(no tc)
              14->15->0(no tc)
*/
void ttl7416x_device::increment()
{
	m_out++;

	if (m_limit == 10)
	{
		if (m_out == 12 || m_out == 14) m_out = 4;
		if (m_out == 10) m_out = 0;
	}
	m_out &= 0x0f;
}

WRITE_LINE_MEMBER( ttl7416x_device::clear_w )
{
	m_clear = state;
	if (!m_synchronous_reset)
		init();
}


WRITE_LINE_MEMBER( ttl7416x_device::pe_w )
{
	m_pe = state ^ 1;
}

WRITE_LINE_MEMBER( ttl7416x_device::cet_w )
{
	m_cet = state;
}

WRITE_LINE_MEMBER( ttl7416x_device::cep_w )
{
	m_cep = state;
}

WRITE_LINE_MEMBER( ttl7416x_device::clock_w )
{
	uint8_t last_clock = m_pclock;
	m_pclock = state;
	if (m_pclock != last_clock && m_pclock != 0)
	{
		tick();
	}
}

void ttl7416x_device::p_w(uint8_t data)
{
	m_p = data & 0xf;
}

WRITE_LINE_MEMBER( ttl7416x_device::p1_w )
{
	m_p &= ~(1 << 0);
	m_p |= (state << 0);
}

WRITE_LINE_MEMBER( ttl7416x_device::p2_w )
{
	m_p &= ~(1 << 1);
	m_p |= (state << 1);
}

WRITE_LINE_MEMBER( ttl7416x_device::p3_w )
{
	m_p &= ~(1 << 2);
	m_p |= (state << 2);
}

WRITE_LINE_MEMBER( ttl7416x_device::p4_w )
{
	m_p &= ~(1 << 3);
	m_p |= (state << 3);
}

READ_LINE_MEMBER( ttl7416x_device::output_r )
{
	return m_out;
}

/*
    TC is an asynchronous signal, depending on the current states of CET, CEP,
    and the counter value.
*/
READ_LINE_MEMBER( ttl7416x_device::tc_r )
{
	return ((m_cet==1) && (m_cep==1) && (m_out==m_limit-1))? 1:0;
}
