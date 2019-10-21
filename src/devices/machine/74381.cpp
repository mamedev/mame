// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    74381.cpp
    TI SN74S381 ALU / Function Generator

***************************************************************************/

#include "emu.h"
#include "74381.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(SN74S381, sn74s381_device, "sn74s381", "TI SN74S381 ALU / Function Generator")

//-------------------------------------------------
//  sn74s381_device - constructor
//-------------------------------------------------

sn74s381_device::sn74s381_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SN74S381, tag, owner, clock)
	, m_a(0)
	, m_b(0)
	, m_s(0)
	, m_cn(0)
	, m_f(0)
	, m_p(false)
	, m_g(false)
	, m_f_out(*this)
	, m_p_out(*this)
	, m_g_out(*this)
{
}

void sn74s381_device::device_start()
{
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_s));
	save_item(NAME(m_cn));
	save_item(NAME(m_f));
	save_item(NAME(m_p));
	save_item(NAME(m_g));

	m_f_out.resolve_safe();
	m_p_out.resolve_safe();
	m_g_out.resolve_safe();
}

void sn74s381_device::device_reset()
{
	m_a = 0;
	m_b = 0;
	m_s = 0;
	m_cn = 0;
	m_f = 0;
	m_p = false;
	m_g = false;

	m_f_out(0);
	m_p_out(true);
	m_g_out(true);
}

void sn74s381_device::a_w(uint8_t data)
{
	m_a = data;
	update();
}

void sn74s381_device::b_w(uint8_t data)
{
	m_b = data;
	update();
}

void sn74s381_device::s_w(uint8_t data)
{
	m_s = data;
	update();
}

void sn74s381_device::cn_w(int state)
{
	m_cn = state;
	update();
}

void sn74s381_device::update()
{
	switch (m_s)
	{
	case 0: // Clear
		m_f = 0;
		m_p = true;
		m_g = true;
		break;
	case 1: // B - A
	{
		const uint8_t c = 1 - m_cn;
		const uint8_t d = (m_a + c) & 0xf;
		m_f = (m_b - d) & 0xf;
		m_p = (( m_a ^ m_b) >> 3) == 0;
		m_g = ((~m_a & m_b) >> 3) != 0;
		break;
	}
	case 2: // A - B
	{
		const uint8_t c = 1 - m_cn;
		const uint8_t d = (m_b + c) & 0xf;
		m_f = (m_a - d) & 0xf;
		m_p = ((m_a ^  m_b) >> 3) == 0;
		m_g = ((m_a & ~m_b) >> 3) != 0;
		break;
	}
	case 3: // A + B
	{
		const uint8_t c = m_cn;
		const uint8_t d = (m_b + c) & 0xf;
		m_f = (m_a + d) & 0xf;
		m_p = ((m_a ^ m_b) >> 3) != 0;
		m_g = ((m_a & m_b) >> 3) != 0;
		break;
	}
	case 4: // A ^ B
		m_f = m_a ^ m_b;
		m_p = (m_a >> 3) != (m_b >> 3);
		m_g = false;
		break;
	case 5: // A | B
		m_f = m_a | m_b;
		m_p = (m_a >> 3) != 0 || (m_b >> 3) != 0;
		m_g = false;
		break;
	case 6: // A & B
		m_f = m_a & m_b;
		m_p = (m_a >> 3) != 0 && (m_b >> 3) != 0;
		m_g = false;
		break;
	case 7: // Preset
		m_f = 0xf;
		m_p = true;
		m_g = false;
		break;
	}

	m_f_out(m_f);
	m_p_out(m_p ? 0 : 1);
	m_g_out(m_g ? 0 : 1);
}
