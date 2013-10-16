// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

***************************************************************************/

#pragma once

#ifndef __MIPS3FE_H__
#define __MIPS3FE_H__

#include "mips3com.h"
#include "cpu/drcfe.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)                    (((n) == 0) ? 0 : (1 << (n)))

// register flags 1
#define REGFLAG_CPR1(n)                 (1 << (n))

// register flags 2
#define REGFLAG_LO                      (1 << 0)
#define REGFLAG_HI                      (1 << 1)
#define REGFLAG_FCC                     (1 << 2)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mips3_frontend : public drc_frontend
{
public:
	// construction/destruction
	mips3_frontend(mips3_state &state, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev);

private:
	// internal helpers
	bool describe_special(UINT32 op, opcode_desc &desc);
	bool describe_regimm(UINT32 op, opcode_desc &desc);
	bool describe_idt(UINT32 op, opcode_desc &desc);
	bool describe_cop0(UINT32 op, opcode_desc &desc);
	bool describe_cop1(UINT32 op, opcode_desc &desc);
	bool describe_cop1x(UINT32 op, opcode_desc &desc);
	bool describe_cop2(UINT32 op, opcode_desc &desc);

	// internal state
	mips3_state &m_context;
};


#endif /* __MIPS3FE_H__ */
