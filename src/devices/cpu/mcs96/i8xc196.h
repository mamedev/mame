// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#ifndef MAME_CPU_MCS96_I8XC196_H
#define MAME_CPU_MCS96_I8XC196_H

#include "mcs96.h"

class i8xc196_device : public mcs96_device {
protected:
	i8xc196_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	void internal_regs(address_map &map) ATTR_COLD;

#define O(o) void o ## _196_full(); void o ## _196_partial()

	O(bmov_direct_2w);
	O(bmovi_direct_2w);
	O(cmpl_direct_2w);
	O(djnzw_wrrel8);
	O(idlpd_none);
	O(pop_indexed_1w);
	O(pop_indirect_1w);
	O(popa_none);
	O(pusha_none);

#undef O
};

#endif // MAME_CPU_MCS96_I8XC196_H
