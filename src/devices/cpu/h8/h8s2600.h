// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2600.h

    H8S-2600 base cpu emulation

    Adds the multiply-and-accumulate register and related instructions


***************************************************************************/

#ifndef __H8S2600_H__
#define __H8S2600_H__

#include "h8s2000.h"

class h8s2600_device : public h8s2000_device {
public:
	h8s2600_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, address_map_delegate map_delegate);

protected:
	static const disasm_entry disasm_entries[];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

#define O(o) void o ## _full(); void o ## _partial()
	O(clrmac);
	O(ldmac_r32l_mach); O(ldmac_r32l_macl);
	O(mac_r32ph_r32pl);
	O(stmac_mach_r32l); O(stmac_macl_r32l);
#undef O
};

#endif
