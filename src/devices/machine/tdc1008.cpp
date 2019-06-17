// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    tdc1008.cpp
    TRW TDC1008 VLSI Multiplier - Accumulator

***************************************************************************/

#include "emu.h"
#include "tdc1008.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(TDC1008, tdc1008_device, "tdc1008", "TRW TDC1008 Multiplier-Accumulator")

//-------------------------------------------------
//  tdc1008_device - constructor
//-------------------------------------------------

tdc1008_device::tdc1008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TDC1008, tag, owner, clock)
	, m_x_in(0)
	, m_y_in(0)
	, m_xtp_in(0)
	, m_msp_in(0)
	, m_lsp_in(0)
	, m_p_in(0)
	, m_tsx(false)
	, m_tsm(false)
	, m_tsl(false)
	, m_clk_x(false)
	, m_clk_y(false)
	, m_clk_p(false)
	, m_prel(false)
	, m_rnd_in(false)
	, m_rnd(false)
	, m_tc_in(false)
	, m_tc(false)
	, m_acc_in(false)
	, m_acc(false)
	, m_sub_in(false)
	, m_sub(false)
	, m_xtp(*this)
	, m_msp(*this)
	, m_lsp(*this)
	, m_p(*this)
{
}


void tdc1008_device::device_start()
{
	save_item(NAME(m_x_in));
	save_item(NAME(m_y_in));
	save_item(NAME(m_xtp_in));
	save_item(NAME(m_msp_in));
	save_item(NAME(m_lsp_in));
	save_item(NAME(m_p_in));
	save_item(NAME(m_tsx));
	save_item(NAME(m_tsm));
	save_item(NAME(m_tsl));
	save_item(NAME(m_clk_x));
	save_item(NAME(m_clk_y));
	save_item(NAME(m_clk_p));
	save_item(NAME(m_prel));
	save_item(NAME(m_rnd_in));
	save_item(NAME(m_rnd));
	save_item(NAME(m_tc_in));
	save_item(NAME(m_tc));
	save_item(NAME(m_acc_in));
	save_item(NAME(m_acc));
	save_item(NAME(m_sub_in));
	save_item(NAME(m_sub));
	save_item(NAME(m_x.u));
	save_item(NAME(m_y.u));
	save_item(NAME(m_p_out.u));

	m_xtp.resolve_safe();
	m_msp.resolve_safe();
	m_lsp.resolve_safe();
	m_p.resolve_safe();
}

void tdc1008_device::device_reset()
{
	m_x_in = 0;
	m_y_in = 0;
	m_xtp_in = 0;
	m_msp_in = 0;
	m_lsp_in = 0;
	m_p_in = 0;
	m_tsx = false;
	m_tsm = false;
	m_tsl = false;
	m_clk_x = false;
	m_clk_y = false;
	m_clk_p = false;
	m_prel = false;
	m_rnd_in = false;
	m_rnd = false;
	m_tc_in = false;
	m_tc = false;
	m_acc_in = false;
	m_acc = false;
	m_sub_in = false;
	m_sub = false;

	m_x.u = 0;
	m_y.u = 0;
	m_p_out.u = 0;
}

WRITE8_MEMBER(tdc1008_device::x_w)
{
	m_x_in = data;
}

WRITE8_MEMBER(tdc1008_device::y_w)
{
	m_y_in = data;
}

WRITE_LINE_MEMBER(tdc1008_device::tsx_w)
{
	m_tsx = (bool)state;
	if (m_prel && m_tsx)
		m_p_out.u = (m_p_out.u & 0x0ffff) | (m_xtp_in << 16);
}

WRITE_LINE_MEMBER(tdc1008_device::tsm_w)
{
	m_tsm = (bool)state;
	if (m_prel && m_tsm)
		m_p_out.u = (m_p_out.u & 0x700ff) | (m_msp_in << 8);
}

WRITE_LINE_MEMBER(tdc1008_device::tsl_w)
{
	m_tsl = (bool)state;
	if (m_prel && m_tsl)
		m_p_out.u = (m_p_out.u & 0x7ff00) | m_lsp_in;
}

WRITE8_MEMBER(tdc1008_device::xtp_w)
{
	m_xtp_in = data;
	if (m_prel && m_tsx)
		m_p_out.u = (m_p_out.u & 0x0ffff) | (m_xtp_in << 16);
}

WRITE8_MEMBER(tdc1008_device::msp_w)
{
	m_msp_in = data;
	if (m_prel && m_tsm)
		m_p_out.u = (m_p_out.u & 0x700ff) | (m_msp_in << 8);
}

WRITE8_MEMBER(tdc1008_device::lsp_w)
{
	m_lsp_in = data;
	if (m_prel && m_tsl)
		m_p_out.u = (m_p_out.u & 0x7ff00) | m_lsp_in;
}

WRITE32_MEMBER(tdc1008_device::output_w)
{
	m_p_in = data;
}

WRITE_LINE_MEMBER(tdc1008_device::clk_x_w)
{
	bool old = m_clk_x;
	m_clk_x = (bool)state;
	if (!old && m_clk_x)
	{
		m_x.u = m_x_in;
		latch_flags();
	}
}

WRITE_LINE_MEMBER(tdc1008_device::clk_y_w)
{
	bool old = m_clk_y;
	m_clk_y = (bool)state;
	if (!old && m_clk_y)
	{
		m_y.u = m_y_in;
		latch_flags();
	}
}

WRITE_LINE_MEMBER(tdc1008_device::clk_p_w)
{
	bool old = m_clk_p;
	m_clk_p = (bool)state;
	if (!old && m_clk_p)
	{
		if (m_tc)
		{
			int32_t new_product = (int32_t)m_x.s * (int32_t)m_y.s;
			if (m_rnd)
			{
				new_product += 0x80;
			}
			if (m_acc)
			{
				if (m_sub)
					m_p_out.s = new_product - m_p_out.s;
				else
					m_p_out.s = new_product + m_p_out.s;
			}
			else
			{
				m_p_out.s = new_product;
			}
		}
		else
		{
			uint32_t new_product = (uint32_t)m_x.u * (uint32_t)m_y.u;
			if (m_rnd)
			{
				new_product += 0x80;
			}
			if (m_acc)
			{
				if (m_sub)
					m_p_out.u = new_product - m_p_out.u;
				else
					m_p_out.u = new_product + m_p_out.u;
			}
			else
			{
				m_p_out.u = new_product;
			}
		}

		if (!m_tsx && !m_prel)
			m_xtp(m_p_out.u >> 16);
		if (!m_tsm && !m_prel)
			m_msp((uint8_t)(m_p_out.u >> 8));
		if (!m_tsl && !m_prel)
			m_lsp((uint8_t)m_p_out.u);
		m_p(m_p_out.u);
	}
}

WRITE_LINE_MEMBER(tdc1008_device::prel_w)
{
	m_prel = (bool)state;
	if (m_prel)
	{
		if (m_tsx)
			m_p_out.u = (m_p_out.u & 0x0ffff) | (m_xtp_in << 16);
		if (m_tsm)
			m_p_out.u = (m_p_out.u & 0x700ff) | (m_msp_in << 8);
		if (m_tsl)
			m_p_out.u = (m_p_out.u & 0x7ff00) | m_lsp_in;
	}
}

WRITE_LINE_MEMBER(tdc1008_device::rnd_w)
{
	m_rnd_in = (bool)state;
}

WRITE_LINE_MEMBER(tdc1008_device::tc_w)
{
	m_tc_in = (bool)state;
}

WRITE_LINE_MEMBER(tdc1008_device::acc_w)
{
	m_acc_in = (bool)state;
}

WRITE_LINE_MEMBER(tdc1008_device::sub_w)
{
	m_sub_in = (bool)state;
}

void tdc1008_device::latch_flags()
{
	m_rnd = m_rnd_in;
	m_tc = m_tc_in;
	m_acc = m_acc_in;
	m_sub = m_sub_in;
}
