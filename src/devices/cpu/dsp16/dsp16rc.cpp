// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series recompiler

    There are a number of easy optimisations:
    * The RAM space is entirely internal, so the memory system can be
      bypassed if debugging is not enabled.
    * The DSP16A has 16-bit YAAU registers, so sign extension can be
      elided.
    * The same code rarely runs with IACK asserted and clear, allowing
      optimisation of PI register accesses and interrupt checks.
    * The PSW is rarely accessed directly, so the accumulator guard bits
      don't need to be kept in sync (interpreter already does this).
    * The DAU flags are used infrequently, so it's far cheaper to
      calculate them on-demand rather than preemptively.
    * The same code will rarely be run with different AUC modes, so it's
      cheaper to make assumptions and recompile if they break.
    * The SIO and PIO modes are rarely changed, so we can invalidate the
      entire cache when this happens.

    There are some more complex optimisations that give good gains:
    * Multiplication is free with F1, so code will sometimes multiply
      when it doesn't need to - this can be elided.
    * Address register accesses can be elided in many cases as the
      values can be computed at translation time.

***************************************************************************/

#include "emu.h"
#include "dsp16rc.h"
#include "dsp16core.h"

#include "cpu/drcumlsh.h"


/***********************************************************************
    construction/destruction
***********************************************************************/

dsp16_device_base::recompiler::recompiler(dsp16_device_base &host, u32 flags)
	: m_host(host)
	, m_core(*host.m_core)
	, m_frontend(host, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, COMPILE_MAX_SEQUENCE)
	, m_uml(host, host.m_drc_cache, flags, 2, 16, 0)
{
	m_uml.symbol_add(&m_core.xaau_pc, sizeof(m_core.xaau_pc), "pc");
	m_uml.symbol_add(&m_core.xaau_pt, sizeof(m_core.xaau_pt), "pt");
	m_uml.symbol_add(&m_core.xaau_pr, sizeof(m_core.xaau_pr), "pr");
	m_uml.symbol_add(&m_core.xaau_pi, sizeof(m_core.xaau_pi), "pi");
	m_uml.symbol_add(&m_core.xaau_i, sizeof(m_core.xaau_i), "i");

	m_uml.symbol_add(&m_core.yaau_r[0], sizeof(m_core.yaau_r[0]), "r0");
	m_uml.symbol_add(&m_core.yaau_r[1], sizeof(m_core.yaau_r[1]), "r1");
	m_uml.symbol_add(&m_core.yaau_r[2], sizeof(m_core.yaau_r[2]), "r2");
	m_uml.symbol_add(&m_core.yaau_r[3], sizeof(m_core.yaau_r[3]), "r3");
	m_uml.symbol_add(&m_core.yaau_rb, sizeof(m_core.yaau_rb), "rb");
	m_uml.symbol_add(&m_core.yaau_re, sizeof(m_core.yaau_re), "re");
	m_uml.symbol_add(&m_core.yaau_j, sizeof(m_core.yaau_j), "j");
	m_uml.symbol_add(&m_core.yaau_k, sizeof(m_core.yaau_k), "k");

	m_uml.symbol_add(&m_core.dau_x, sizeof(m_core.dau_x), "x");
	m_uml.symbol_add(&m_core.dau_y, sizeof(m_core.dau_y), "y");
	m_uml.symbol_add(&m_core.dau_p, sizeof(m_core.dau_p), "p");
	m_uml.symbol_add(&m_core.dau_a[0], sizeof(m_core.dau_a[0]), "a0");
	m_uml.symbol_add(&m_core.dau_a[1], sizeof(m_core.dau_a[1]), "a1");
	m_uml.symbol_add(&m_core.dau_c[0], sizeof(m_core.dau_c[0]), "c0");
	m_uml.symbol_add(&m_core.dau_c[1], sizeof(m_core.dau_c[1]), "c1");
	m_uml.symbol_add(&m_core.dau_c[2], sizeof(m_core.dau_c[2]), "c2");
	m_uml.symbol_add(&m_core.dau_auc, sizeof(m_core.dau_auc), "auc");
	m_uml.symbol_add(&m_core.dau_psw, sizeof(m_core.dau_psw), "psw");
	m_uml.symbol_add(&m_core.dau_temp, sizeof(m_core.dau_temp), "temp");

	(void)m_host;
}

dsp16_device_base::recompiler::~recompiler()
{
}
