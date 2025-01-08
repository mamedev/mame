// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16.h

    6502, reverse-engineered DECO variant

***************************************************************************/

#ifndef MAME_CPU_M6502_DECO16_H
#define MAME_CPU_M6502_DECO16_H

#include "m6502.h"
#include "deco16d.h"

class deco16_device : public m6502_device {
public:
	deco16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	address_space *io;
	address_space_config io_config;

	virtual space_config_vector memory_space_config() const override;
	virtual void device_start() override ATTR_COLD;

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

DECLARE_DEVICE_TYPE(DECO16, deco16_device)

#endif // MAME_CPU_M6502_DECO16_H
