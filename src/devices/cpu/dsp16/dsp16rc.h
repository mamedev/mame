// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series recompiler

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16RC_H
#define MAME_CPU_DSP16_DSP16RC_H

#pragma once

#include "dsp16.h"
#include "dsp16fe.h"

#include "cpu/drcuml.h"


class dsp16_device_base::recompiler
{
public:
	// construction/destruction
	recompiler(dsp16_device_base &host, u32 flags);
	~recompiler();

private:
	// compilation boundaries
	enum : u32
	{
		COMPILE_BACKWARDS_BYTES = 64,
		COMPILE_FORWARDS_BYTES = 256,
		COMPILE_MAX_INSTRUCTIONS = (COMPILE_BACKWARDS_BYTES / 2) + (COMPILE_FORWARDS_BYTES / 2),
		COMPILE_MAX_SEQUENCE = 64
	};

	// exit codes for recompiled blocks
	enum : int
	{
		EXEC_OUT_OF_CYCLES,
		EXEC_MISSING_CODE,
		EXEC_UNMAPPED_CODE,
		EXEC_RESET_CACHE
	};

	// host CPU device, frontend to describe instructions, and UML engine
	dsp16_device_base   &m_host;
	core_state          &m_core;
	frontend            m_frontend;
	drcuml_state        m_uml;
};

#endif // MAME_CPU_DSP16_DSP16RC_H
