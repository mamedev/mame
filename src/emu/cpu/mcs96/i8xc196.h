/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

****************************************************************************

    Copyright Olivier Galibert
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

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __I8XC196_H__
#define __I8XC196_H__

#include "mcs96.h"

class i8xc196_device : public mcs96_device {
public:
	i8xc196_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

	virtual void next(int cycles);	
	virtual void io_w8(UINT8 adr, UINT8 data);
	virtual void io_w16(UINT8 adr, UINT16 data);
	virtual UINT8 io_r8(UINT8 adr);
	virtual UINT16 io_r16(UINT8 adr);

#define O(o) void o ## _196_full(); void o ## _196_partial()

	O(bmov_direct_2);
	O(cmpl_direct_2);
	O(bmovi_direct_2);
	O(pop_indirect_1);
	O(pop_indexed_1);
	O(djnzw_rrel8);
	O(pusha_none);
	O(idlpd_none);

#undef O
};

#endif
