/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
