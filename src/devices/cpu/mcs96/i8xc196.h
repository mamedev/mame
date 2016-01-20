// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#ifndef __I8XC196_H__
#define __I8XC196_H__

#include "mcs96.h"

class i8xc196_device : public mcs96_device {
public:
	i8xc196_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	virtual void io_w8(UINT8 adr, UINT8 data) override;
	virtual void io_w16(UINT8 adr, UINT16 data) override;
	virtual UINT8 io_r8(UINT8 adr) override;
	virtual UINT16 io_r16(UINT8 adr) override;

#define O(o) void o ## _196_full(); void o ## _196_partial()

	O(bmov_direct_2);
	O(bmovi_direct_2);
	O(cmpl_direct_2);
	O(djnzw_rrel8);
	O(idlpd_none);
	O(pop_indexed_1);
	O(pop_indirect_1);
	O(popa_none);
	O(pusha_none);

#undef O
};

#endif
