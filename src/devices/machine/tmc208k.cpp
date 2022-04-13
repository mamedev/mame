// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    tmc208k.cpp
    TRW TMC208K/TMC28KU 8x8-bit Parallel Multiplier

    Known Equivalents:
    - Logic Devices Inc. LMU08/8U

***************************************************************************/

#include "emu.h"
#include "tmc208k.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(TMC208K, tmc208k_device, "tmc208k", "TRW TMC208K 8x8-bit Multiplier")
DEFINE_DEVICE_TYPE(TMC28KU, tmc28ku_device, "tmc28ku", "TRW TMC28KU 8x8-bit Multiplier")


//-------------------------------------------------
//  tdc1008_device - constructor
//-------------------------------------------------

template <typename RegType, typename OutType>
tmc208_base_device<RegType, OutType>::tmc208_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_a_in(0)
	, m_a(0)
	, m_b_in(0)
	, m_b(0)
	, m_r_out(0)
	, m_trim(false)
	, m_tril(false)
	, m_clk_a(false)
	, m_clk_b(false)
	, m_clk_r(false)
	, m_rnd_in(false)
	, m_rnd(false)
	, m_msp(*this)
	, m_lsp(*this)
	, m_r(*this)
{
}

tmc208k_device::tmc208k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tmc208_base_device<int8_t, int16_t>(mconfig, TMC208K, tag, owner, clock)
{
}

tmc28ku_device::tmc28ku_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tmc208_base_device<uint8_t, uint16_t>(mconfig, TMC28KU, tag, owner, clock)
{
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::device_start()
{
	save_item(NAME(m_a_in));
	save_item(NAME(m_a));
	save_item(NAME(m_b_in));
	save_item(NAME(m_b));
	save_item(NAME(m_r_out));
	save_item(NAME(m_trim));
	save_item(NAME(m_tril));
	save_item(NAME(m_clk_a));
	save_item(NAME(m_clk_b));
	save_item(NAME(m_clk_r));
	save_item(NAME(m_rnd_in));
	save_item(NAME(m_rnd));

	m_msp.resolve_safe();
	m_lsp.resolve_safe();
	m_r.resolve_safe();
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::device_reset()
{
	m_a_in = 0;
	m_a = 0;
	m_b_in = 0;
	m_b = 0;
	m_r_out = 0;
	m_trim = false;
	m_tril = false;
	m_clk_a = false;
	m_clk_b = false;
	m_clk_r = false;
	m_rnd_in = false;
	m_rnd = false;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::a_w(uint8_t data)
{
	m_a_in = (RegType)data;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::b_w(uint8_t data)
{
	m_b_in = (RegType)data;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::trim_w(int state)
{
	m_trim = (bool)state;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::tril_w(int state)
{
	m_tril = (bool)state;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::clk_a_w(int state)
{
	bool old = m_clk_a;
	m_clk_a = (bool)state;
	if (!old && m_clk_a)
		clock_a();
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::clk_b_w(int state)
{
	bool old = m_clk_b;
	m_clk_b = (bool)state;
	if (!old && m_clk_b)
		clock_b();
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::clk_r_w(int state)
{
	bool old = m_clk_r;
	m_clk_r = (bool)state;
	if (!old && m_clk_r)
		multiply();
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::rnd_w(int state)
{
	m_rnd_in = (bool)state;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::multiply()
{
	m_r_out = (OutType)m_a * (OutType)m_b;
	if (!m_trim)
		m_msp((uint8_t)(m_r_out >> 8));
	if (!m_tril)
		m_lsp((uint8_t)m_r_out);
	m_r(m_r_out);
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::clock_a()
{
	m_a = m_a_in;
	m_rnd = m_rnd_in;
}

template <typename RegType, typename OutType>
void tmc208_base_device<RegType, OutType>::clock_b()
{
	m_b = m_b_in;
	m_rnd = m_rnd_in;
}

// TMC208KU - Unsigned 8x8 Multiply

void tmc28ku_device::clock_b()
{
	m_b = m_b_in;
}
