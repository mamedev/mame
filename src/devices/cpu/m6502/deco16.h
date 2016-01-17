// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16.h

    6502, reverse-engineered DECO variant

***************************************************************************/

#ifndef __DECO16_H__
#define __DECO16_H__

#include "m6502.h"

class deco16_device : public m6502_device {
public:
	deco16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	address_space *io;
	address_space_config io_config;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	virtual void device_start() override;

#define O(o) void o ## _full(); void o ## _partial()

	O(brk_16_imp);
	O(ill_non);
	O(u0B_zpg);
	O(u13_zpg);
	O(u23_zpg);
	O(u3F_zpg);
	O(u4B_zpg);
	O(u87_zpg);
	O(u8F_zpg);
	O(uA3_zpg);
	O(uAB_zpg);
	O(uBB_zpg);
	O(vbl_zpg);

	O(reset_16);

#undef O
};

enum {
	DECO16_IRQ_LINE = m6502_device::IRQ_LINE,
	DECO16_NMI_LINE = m6502_device::NMI_LINE,
	DECO16_SET_OVERFLOW = m6502_device::V_LINE
};

extern const device_type DECO16;

#endif
