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

    There are some more complex optimisations that give good gains:
    * Multiplication is free with F1, so code will sometimes multiply
      when it doesn't need to - this can be elided.
    * Address register accesses can be elided in many cases as the
      values can be computed at translation time.

***************************************************************************/

#include "dsp16rc.h"

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
	(void)m_host;
	(void)m_core;
}

dsp16_device_base::recompiler::~recompiler()
{
}
