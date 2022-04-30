// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am2901b.cpp
    AMD Am2901B / Am2901C
    Four-Bit Bipolar Microprocessor Slice

    To Do:
    - Opcode hookup
    - Verification

***************************************************************************/

#include "emu.h"
#include "am2901b.h"

#define VERBOSE (1)
#include "logmacro.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(AM2901B, am2901b_device, "am2901b", "AMD Am2901B Bitslice Processor")

//-------------------------------------------------
//  am2901b_device - constructor
//-------------------------------------------------

am2901b_device::am2901b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AM2901B, tag, owner, clock)
	, m_d(0)
	, m_q(0)
	, m_i(0)
	, m_a_addr(0)
	, m_b_addr(0)
	, m_a_latch(0)
	, m_b_latch(0)
	, m_q0_in(false)
	, m_q3_in(false)
	, m_ram0_in(false)
	, m_ram3_in(false)
	, m_ci(false)
	, m_cp(false)
	, m_y_out(0)
	, m_g_out(true)
	, m_p_out(true)
	, m_ovr_out(false)
	, m_f0_out(true)
	, m_f3_out(false)
	, m_co_out(false)
	, m_y(*this)
	, m_g(*this)
	, m_p(*this)
	, m_q0(*this)
	, m_q3(*this)
	, m_ram0(*this)
	, m_ram3(*this)
	, m_ovr(*this)
	, m_f0(*this)
	, m_f3(*this)
	, m_co(*this)
{
}

void am2901b_device::device_start()
{
	save_item(NAME(m_a));
	save_item(NAME(m_d));
	save_item(NAME(m_q));
	save_item(NAME(m_i));

	save_item(NAME(m_a_addr));
	save_item(NAME(m_b_addr));

	save_item(NAME(m_a_latch));
	save_item(NAME(m_b_latch));

	save_item(NAME(m_q0_in));
	save_item(NAME(m_q3_in));
	save_item(NAME(m_ram0_in));
	save_item(NAME(m_ram3_in));
	save_item(NAME(m_ci));
	save_item(NAME(m_cp));

	save_item(NAME(m_y_out));
	save_item(NAME(m_g_out));
	save_item(NAME(m_p_out));
	save_item(NAME(m_q0_out));
	save_item(NAME(m_q3_out));
	save_item(NAME(m_ram0_out));
	save_item(NAME(m_ram3_out));
	save_item(NAME(m_ovr_out));
	save_item(NAME(m_f0_out));
	save_item(NAME(m_f3_out));
	save_item(NAME(m_co_out));

	m_y.resolve_safe();
	m_g.resolve_safe();
	m_p.resolve_safe();
	m_q0.resolve_safe();
	m_q3.resolve_safe();
	m_ram0.resolve_safe();
	m_ram3.resolve_safe();
	m_ovr.resolve_safe();
	m_f0.resolve_safe();
	m_f3.resolve_safe();
	m_co.resolve_safe();
}

void am2901b_device::device_reset()
{
	memset(m_a, 0, sizeof(uint8_t) * 16);
	m_d = 0;
	m_q = 0;
	m_i = 0;

	m_a_addr = 0;
	m_b_addr = 0;

	m_a_latch = 0;
	m_b_latch = 0;

	m_q0_in = false;
	m_q3_in = false;
	m_ram0_in = false;
	m_ram3_in = false;
	m_ci = false;
	m_cp = false;

	m_y_out = 0;
	m_g_out = true;
	m_p_out = true;
	m_q0_out = false;
	m_q3_out = false;
	m_ram0_out = false;
	m_ram3_out = false;
	m_ovr_out = false;
	m_f0_out = true;
	m_f3_out = false;
	m_co_out = false;
}

void am2901b_device::a_w(uint8_t data)
{
	m_a_addr = data;
}

void am2901b_device::b_w(uint8_t data)
{
	m_b_addr = data;
}

void am2901b_device::d_w(uint8_t data)
{
	m_d = data;
}

void am2901b_device::i_w(uint16_t data)
{
	m_i = data;
}

WRITE_LINE_MEMBER(am2901b_device::q0_w)
{
	m_q0_in = (bool)state;
}

WRITE_LINE_MEMBER(am2901b_device::q3_w)
{
	m_q3_in = (bool)state;
}

WRITE_LINE_MEMBER(am2901b_device::ram0_w)
{
	m_ram0_in = (bool)state;
}

WRITE_LINE_MEMBER(am2901b_device::ram3_w)
{
	m_ram3_in = (bool)state;
}

WRITE_LINE_MEMBER(am2901b_device::ci_w)
{
	m_ci = (bool)state;
}

WRITE_LINE_MEMBER(am2901b_device::cp_w)
{
	bool old = m_cp;
	m_cp = (bool)state;
	if (!old && m_cp)
	{
		m_a_latch = m_a[m_a_addr];
		m_b_latch = m_a[m_b_addr];
		execute();
	}
}

void am2901b_device::execute()
{
	disassemble();
}

void am2901b_device::disassemble()
{
	static const char s_r_table[8] = { 'A', 'A', '0', '0', '0', 'D', 'D', 'D' };
	static const char s_s_table[8] = { 'Q', 'B', 'Q', 'B', 'A', 'A', 'Q', '0' };
	static const char *const s_func_table[8] =
	{
		"ADD", "SUBR", "SUBS", "OR", "AND", "NOTRS", "EXOR", "EXNOR"
	};
	static const char *const s_dst_table[8] =
	{
		"QREG", "NOP", "RAMA", "RAMF", "RAMQD", "RAMD", "RAMQU", "RAMU"
	};

	char dasm_buf[64];
	int buf_idx = snprintf(dasm_buf, std::size(dasm_buf), "%s %s", s_func_table[(m_i >> 3) & 7], s_dst_table[(m_i >> 6) & 7]);
	while (buf_idx < 12)
	{
		dasm_buf[buf_idx] = ' ';
		buf_idx++;
	}
	snprintf(dasm_buf + buf_idx, std::size(dasm_buf) - 12, "%c,%c", s_r_table[m_i & 7], s_s_table[m_i & 7]);

	LOG("%s: %s\n", machine().describe_context(), dasm_buf);
}
